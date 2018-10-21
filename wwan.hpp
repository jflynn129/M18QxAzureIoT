/**
* copyright (c) 2018, James Flynn
* SPDX-License-Identifier: MIT
*/

/**
*   @file   wwan.hpp
*   @brief  A class for manaing the WWAN LED on the WNC M18Qx board. This isn't a normal binary i/o
*           LED and must be controlled through the linux driver. The class creates a thread that 
*           runs every 500msec and checks to see if we have signal, are on-line, and have an ip 
*           address and flashes at a different rate depending on which is true.
*
*   @author James Flynn
*
*   @date   1-Oct-2018
*/

#ifndef __WWAN_HPP__
#define __WWAN_HPP__

#include <chrono>
#include <thread>

#include "mal.hpp"

class Wwan {
    private:
        Mal*        malptr;
        pthread_t   wwan_thread;
        int         wwan_active;
        bool        wwan_on;
        bool        wwan_enable;

        char* get_ipAddr(json_keyval *kv, int kvsize) {
            int  i;
            char rstr[500];
            char jcmd[] = "{ \"action\" : \"get_wwan_ipv4_network_ip\" }";

            malptr->send_mal_command(jcmd, rstr, sizeof(rstr), true);
            i = malptr->parse_maljson (rstr, kv, kvsize);
            if( i < 0 )  //parse failed
                return NULL;
            else if( atoi(kv[1].value) ) // we got an error value back
                return kv[2].value;      // so return error message
            else
                return kv[3].value;    
            }

        char *getOperatingMode(json_keyval *kv, int kvsize) {
            int  i;
            char rstr[100];
            char jcmd[] = "{ \"action\" : \"get_operating_mode\" }";

            malptr->send_mal_command(jcmd, rstr, sizeof(rstr), true);
            i = malptr->parse_maljson (rstr, kv, kvsize);
            if( i < 0 )  //parse failed
                return NULL;
            else if( atoi(kv[1].value) ) // we got an error value back
                return kv[2].value;      // so return error message
            else
                return kv[3].value;    
            }

        int get_wwan_status( json_keyval *kv, int kvsize) {
            char rstr[500];
            char jcmd[] = "{ \"action\" : \"get_wwan_serving_system_status\" }";
        
            malptr->send_mal_command(jcmd, rstr, sizeof(rstr), true);
            return malptr->parse_maljson (rstr, kv, kvsize);
            }

        void wwan_io(int onoff) {
            int fd;
            fd = open("/sys/class/leds/wwan/brightness", O_WRONLY);
            write(fd, onoff?"1":"0", 1);
            close(fd);
            }

    protected:
        static void *wwan_task(void *thread) {
            json_keyval om[20];
            Wwan  *self = static_cast<Wwan *>(thread);
            char *ptr;
            int   blink_cnt=0;
            bool  wwan_on=false;

            while( self->wwan_active ) {
                if( !self->wwan_enable ) {
                    sleep(2);
                    continue;
                    }
                ptr=self->get_ipAddr(om, sizeof(om));
                if( strcmp(ptr,"0.0.0.0") )               //on-line with IP - no blink
                    self->wwan_io(1);
                else{
                    ptr=self->getOperatingMode(om, sizeof(om)); //on-line no IP - fast blink
                    if( !atoi(ptr) ) 
                        self->wwan_io( wwan_on = !wwan_on );
                    else{
                        self->get_wwan_status(om, sizeof(om));
                        if( atoi(om[6].value) ) {         //not on-line and no IP, but have signal - slow blink
                            self->wwan_io( (blink_cnt>1)?0:1 );
                            ++blink_cnt %= 4;
                            }
                        else 
                            self->wwan_io(0);                   //nothing - off
                        }
                    }
                sleep(2);
                }
            pthread_exit(0);
            }

    public:
        Wwan(void) : wwan_active(true), wwan_enable(false) {
            malptr = Mal::get_mal();
            while( !malptr->mal_running() )
                sleep(1);
            pthread_create( &wwan_thread, NULL, wwan_task, (void*)this);
            }

        ~Wwan() { }

        bool enable(void) {
            bool ok = wwan_enable;
            wwan_enable = true;
            return ok;
            }

        bool disable(void) {
            bool ok = wwan_enable;
            wwan_enable = false;
            return ok;
            }

        void terminate(void) {
            int rval = 0;
            wwan_active=false;
            pthread_join(wwan_thread, (void**)&rval);
            wwan_io(0);
            }

};

#endif //__WWAN_HPP__

