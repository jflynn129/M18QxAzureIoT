/**
* copyright (c) 2018, James Flynn
* SPDX-License-Identifier: MIT
*/

#ifndef __GPS_HPP__
#define __GPS_HPP__

#include <pthread.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include <chrono>

#include "mal.hpp"

#include <nettle/nettle-stdint.h>

typedef struct latlong_t {
    float lat, lng;
    } latlong;
     
typedef struct gpsstatus_t {
    latlong last_pos;
    time_t  last_try;
    time_t  last_good;
    } gpsstatus;
    
class Wncgps {
    private:
        gpsstatus       gps_stat;
        pthread_t       gps_thread;
        bool            gps_good;
        bool            enable_acq;
        bool            gps_on;
        latlong         loc;
        pthread_mutex_t gps_mutex;
        Mal*            malptr;

        static void *gps_task(void *thread);
        int getGPSlocation(json_keyval *kv, int kvsize);

    public:
        Wncgps() : 
            gps_good(false),
            enable_acq(false),
            gps_on(true),
            gps_mutex(PTHREAD_MUTEX_INITIALIZER)
            {
            loc.lat = loc.lng = 0.0;
            gps_stat.last_pos = loc;
            gps_stat.last_try = 0;
            gps_stat.last_good= 0;

            malptr = Mal::get_mal();
            while( !malptr->mal_running() )
                sleep(1);
            pthread_create(&gps_thread, NULL, gps_task, (void*)this);
            }

        ~Wncgps () { }

        void terminate(void) { 
            int rval;
            gps_on=false;
            pthread_join(gps_thread, (void**)&rval);
            }

        gpsstatus* getLocation(void) {
            while( pthread_mutex_trylock(&gps_mutex) )
               pthread_yield();
            gpsstatus* ptr=&gps_stat;
            pthread_mutex_unlock(&gps_mutex);
            return ptr;
            }

        bool enable(void) { bool t = enable_acq; enable_acq=true; return t; };

        bool disable(void){ bool t = enable_acq; enable_acq=false; return t; };

        bool status(void) { return gps_good; }

        int reset(void) {
            char rstr[300];
            char jcmd[] = "{ \"action\": \"set_loc_relocate\" }";
            return malptr->send_mal_command(jcmd, rstr, sizeof(rstr), false);
            }

};


#endif // __GPS_HPP__
