/**
* copyright (c) 2018, James Flynn
* SPDX-License-Identifier: MIT
*/

#ifndef __GPS_HPP__
#define __GPS_HPP__

#include <time.h>
#include <sys/time.h>
#include <chrono>

#include "mal.hpp"

#include <nettle/nettle-stdint.h>

typedef struct latlong_t {
    float lat, lng;
    int   error;
    } latlong;
     
class Wncgps {
    private:
        pthread_t       gps_thread;
        int             gps_to;
        latlong         loc;
        bool            quiet;
        Mal*            malptr;

        static void *gps_task(void *thread) {
            Wncgps *self = static_cast<Wncgps *>(thread);
            struct timeval gps_start, gps_end;  
            json_keyval om[12];
            double elapse=0;
            int k, done, i, m=0;
            m=done=0;

            if( !self->quiet) printf("\nGetting GPS Fix...");

            gettimeofday(&gps_start, NULL);
            while( !done ) {
                k=self->getGPSlocation(om,sizeof(om));
                done = atoi(om[3].value)?1:0;
                for( i=1; i<k; i++ ) {
                    if( !strcmp(om[i].key,"latitude") ) 
                        sscanf( om[i].value, "%f", &self->loc.lat);
                    else if( !strcmp(om[i].key,"longitude") ) 
                        sscanf( om[i].value, "%f", &self->loc.lng);
                    else if( !strcmp(om[i].key,"errno") ) 
                        sscanf( om[i].value, "%d", &self->loc.error);
                    }
                gettimeofday(&gps_end, NULL);
                elapse = (((gps_end.tv_sec - gps_start.tv_sec)*1000) + (gps_end.tv_usec/1000 - gps_start.tv_usec/1000));
                if( ((self->gps_to*1000)-round(elapse))/1000 < 0) {
                    self->loc.error = done = -1;
                    if( !self->quiet) printf("\rGPS Acquisiton timed out after %d seconds\n",(int)round(elapse)/1000);
                    }
                else {
                    if( !self->quiet) {
                        printf("\rGetting GPS Fix... %c",(m==0)?'O':(m==1)?'o':(m==2)?'.':' ');
                        fflush(stdout);
                        m++;
                        m %= 4;
                        std::this_thread::sleep_for(std::chrono::milliseconds(500));
                        }
                    }
                }
            if( !self->quiet) printf("\r");

            pthread_exit(&done);
            }

        int getGPSlocation(json_keyval *kv, int kvsize) {
            char rstr[300];
            char jcmd[] = "{ \"action\" : \"get_loc_position_info\" }";
            char enable_jcmd[] = "{ \"action\": \"set_loc_config\", \"args\": { \"loc\": true } }";
            char mode_jcmd[]   = "{ \"action\": \"set_loc_mode\", \"args\": { \"mode\": 4 } }";

            memset(rstr,0x00,sizeof(rstr));
            malptr->send_mal_command(mode_jcmd, NULL, 0, false);
            malptr->send_mal_command(enable_jcmd, NULL, 0, false);  //enable GPS
            if( !malptr->send_mal_command(jcmd, rstr, sizeof(rstr), true) ) 
                return malptr->parse_maljson (rstr, kv, kvsize);

            return 0;
            }

    public:
        Wncgps() : 
            gps_to(120),  //default time-out is 120 seconds
            quiet(true) 
            {
            char rstr[300];
            char enable_jcmd[] = "{ \"action\": \"set_loc_config\", \"args\": { \"loc\": true } }";
            char mode_jcmd[]   = "{ \"action\": \"set_loc_mode\", \"args\": { \"mode\": 3 } }";
            loc.lat = loc.lng = 0.0;
            loc.error = 0;
            malptr = Mal::get_mal();
            while( !malptr->mal_running() )
                sleep(1);
            malptr->send_mal_command(mode_jcmd, rstr, sizeof(rstr), false);
            malptr->send_mal_command(enable_jcmd, rstr, sizeof(rstr), true);  //enable GPS
            }

        ~Wncgps () {
            char rstr[300];
            char disable_jcmd[] = "{ \"action\": \"set_loc_config\", \"args\": { \"loc\": false } }";
            malptr->send_mal_command(disable_jcmd, rstr, sizeof(rstr), false); //disable GPS
            }

        int setTO(int t) {
            int i = gps_to;
            gps_to = t;
            return i;
            }

        latlong getLocation(bool q) {
            quiet = q;
            pthread_create( &gps_thread, NULL, gps_task, (void*)this);
            pthread_join(gps_thread, NULL);
            if( loc.error == -1 )
                reset();
            return loc;
            }

        int reset(void) {
            char rstr[300];
            char jcmd[] = "{ \"action\": \"set_loc_relocate\" }";
            return malptr->send_mal_command(jcmd, rstr, sizeof(rstr), false);
            }
};


#endif // __GPS_HPP__
