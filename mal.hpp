/**
* copyright (c) 2018, James Flynn
* SPDX-License-Identifier: MIT
*/

/**
*   @file   mal.hpp
*   @brief  class definition for the modem abstraction layer (mal).
*
*   @author James Flynn
*
*   @date   1-Oct-2018
*/


#ifndef __MAL_HPP__
#define __MAL_HPP__

#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <thread>

#ifndef __HWLIB__
extern "C" {
#include <hwlib/hwlib.h>
}
#endif

typedef struct _json_keyval {
    char key[50];
    char value[50];
    } json_keyval;

class Mal {
    private:
        static Mal* mal_ptr;
        static bool mal_started;
        int start_mal(bool);
        Mal() {;}

    public:
        static Mal* get_mal( void ) {
            if( !mal_ptr ) {
                mal_ptr = (Mal*)new Mal;
                mal_ptr->start_mal(false);
                mal_ptr->mal_started = true;
                }
             return mal_ptr;
             }

        bool  mal_running(void);
        int   send_mal_command(char *json_cmd, char *json_resp, int len_json_resp, uint8_t wait_resp);
        int   parse_maljson(char *jstr, json_keyval rslts[], int s);
};

#endif // __MAL_HPP__
