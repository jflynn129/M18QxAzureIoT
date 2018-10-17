/**
* copyright (c) 2018, James Flynn
* SPDX-License-Identifier: MIT
*/

#ifndef __DEVINFO_HPP__
#define __DEVINFO_HPP__

#include "mal.hpp"

class Devinfo {
    private:
        Mal*        malptr;
        json_keyval om[12];

    public:
        Devinfo() {
            malptr = Mal::get_mal();
            while( !malptr->mal_running() )
                sleep(1);
            }

        ~Devinfo() {;}

        int getIMEI(char*str,int len) {
            json_keyval om[12];
            char        rstr[100];
            char        jcmd[] = "{ \"action\" : \"get_system_imei\" }";
            int         done = 0;
        
            while( !done ) {
                malptr->send_mal_command(jcmd, rstr, sizeof(rstr), true);
                int i = malptr->parse_maljson (rstr, om, sizeof(om));
                if( i < 0 )  //parse failed
                    return 0;
                if( atoi(om[1].value) == 0 ){  // no errors
                    done = 1;
                    strncpy(str, om[3].value, len);
                    }
                }
            return done;
            }

        int getICCID(char *str, int len) {
            json_keyval om[12];
            char        rstr[100];
            char        jcmd[] = "{ \"action\" : \"get_system_iccid\" }";
            int         done = 0;
        
            while( !done ) {
                malptr->send_mal_command(jcmd, rstr, sizeof(rstr), true);
                int i = malptr->parse_maljson (rstr, om, sizeof(om));
                if( i < 0 )  //parse failed
                    return 0;
                if( atoi(om[1].value) == 0 ){  // no error?
                    done = 1;
                    strncpy( str, om[3].value, len);
                    }
                }
            return done;
            }
};


#endif // __DEVINFO_HPP__
