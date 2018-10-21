/**
* copyright (c) 2018, James Flynn
* SPDX-License-Identifier: MIT
*/

/**
*   @file   mal.cpp
*   @brief  the modem abstraction layer (mal) is used to control the radio functioinallity of the WNC device. Similar
*           to the i2c interface, there is only one that can be used so it is created as a singleton class.
*
*   @author James Flynn
*
*   @date   1-Oct-2018
*/

#include "jsmn.h"
#include "mal.hpp"

Mal* Mal::mal_ptr = NULL;
bool Mal::mal_started = false;

#define JSON_SOCKET_ADDR	"/tmp/cgi-2-sys"

static pthread_mutex_t mal_mutex = PTHREAD_MUTEX_INITIALIZER;

// 
// This function opens a socket to the MAL manager and then sends the JSON command to it.  It will
// then wait for a response, close the socketl, and return.
//
// Inputs:
//     json_cmd     : json string with command to be sent to the MAL manager
//     json_resp    : json string that will contain the response if needed
//     len_json_resp: space availble for the response
//     wait_resp    : TRUE if we should wait for a response
//
// Returns:
//      will return <0 if an error occurs, othersize 0.
        
int Mal::send_mal_command(char *json_cmd, char *json_resp, int len_json_resp, uint8_t wait_resp) 
{
    int client_socket;
    socklen_t addr_length;
    struct sockaddr_un addr;

    while( pthread_mutex_trylock(&mal_mutex) ) 
        pthread_yield();

    strcpy(addr.sun_path, JSON_SOCKET_ADDR);    // max 108 bytes
    addr.sun_family = AF_UNIX;
    addr_length = SUN_LEN(&addr);

    if ((client_socket=socket(AF_UNIX, SOCK_STREAM, 0)) < 0)  {
        pthread_mutex_unlock(&mal_mutex);
        return -1;
        }

    if (connect(client_socket, (struct sockaddr*) &addr, addr_length) < 0) {
        close(client_socket);
        pthread_mutex_unlock(&mal_mutex);
        return -2;
        }
    if (write(client_socket, json_cmd, strlen(json_cmd)) < 0) {
        close(client_socket);
        pthread_mutex_unlock(&mal_mutex);
        return -3;
        }
        
    if (wait_resp) {
        char tresp[1024];                       
        int bytes_read = read(client_socket, tresp, sizeof(tresp)-1);
        if (bytes_read <= 0 || bytes_read > len_json_resp) {
            close(client_socket);
            pthread_mutex_unlock(&mal_mutex);
            return -4;
            }
        memcpy(json_resp, tresp, (len_json_resp>bytes_read)? bytes_read:len_json_resp);
        }
    close(client_socket);
    pthread_mutex_unlock(&mal_mutex);
    return 0;
}

#define VALUE   0
#define KEY     1
#define OBJECT  2
#define ARRAY   3

int Mal::parse_maljson(char *jstr, json_keyval rslts[], int s) 
{
    int ro=0, r, k, i, key=OBJECT;
    jsmn_parser p;
    jsmntok_t t[256]; /* We expect no more than 256 tokens */

    memset( rslts, 0x00, s);

    if (!strlen(jstr))
      return -3;

    jsmn_init(&p);
    r = jsmn_parse(&p, jstr, strlen(jstr), t, 256);
    if (r < 0) {
        return -1;
	}

    /* The top-level element must be an object */
    if (r < 1 || t[0].type != JSMN_OBJECT) {
        printf("Object expected\n");
       	return -2;
        }

    /* Loop over remaining elements of the object */
    i = 1;
    do {
    if (t[i].type == JSMN_PRIMITIVE){ //0-is a PRIMITIVE, e.g., bolean, null, symbol
        k = t[i].end-t[i].start;
        strncpy(rslts[ro++].value, (!k)?"<null>":jstr+t[i].start, (!k)?6:k);
        key = KEY;
        }
    if (t[i].type == JSMN_OBJECT){     //1-is a json object (string)
        key = KEY;
        }
    if (t[i].type == JSMN_ARRAY){      //2-is an ARRAY, need the next token to get the array name
        key = KEY;
        strcpy (rslts[ro-1].value, rslts[ro].key);
        sprintf (rslts[ro-1].key, "ARRAY(%d)",t[i].size);
        memset(rslts[ro].key,0x00,strlen(rslts[ro].key));
        }
    if (t[i].type == JSMN_STRING){     //3-is a STRING
        if (key == VALUE) {
            k = t[i].end-t[i].start;
            strncpy(rslts[ro++].value, (!k)?"<null>":jstr+t[i].start, (!k)?6:k);
            key = KEY;
            }
        else {
            key = VALUE;
            strncpy(rslts[ro].key, jstr+t[i].start,t[i].end-t[i].start);
            if (i == 1) {
                strcpy(rslts[ro].key, "OBJECT");
                strncpy(rslts[ro++].value, jstr+t[i].start, t[i].end-t[i].start);
                }
            }
        }
        i++;
    ro = (ro>s)? s-1:ro;
    }
    while( i < r );
    return ro;
}

//
// This function starts data service in the WNC M18Qx using the MAL manager.
//
// Inputs: NONE
//
// Returns:
//      will return <0 if an error occurs, otherwise 0.

int Mal::start_mal(bool quiet) {

    int timer=1, done = 0;
    char jcmd1[]="{\"action\":\"set_network_connection_mode\",\"args\":{\"mode\":0,\"ondemand_timeout\":2,\"manual_mode\":1}}";
    char jcmd2[]="{\"action\":\"set_wwan_allow_data_roaming\",\"args\":{\"enable\":1}}";

    if( !quiet ) printf( "\n");
    while( !done ) {
        if( send_mal_command(jcmd1, NULL, 0, false) < 0 ) {
            if( !quiet ) {
                printf( "Starting MAL interface. (%d)\r",timer++);
                fflush(stdout);
                }
            sleep(1);
            continue;
            }
        done = (send_mal_command(jcmd2, NULL, 0, false) == 0);
        }
    if( !quiet ) printf( "                                \r");
    return done;
} 

bool Mal::mal_running(void) {
    return mal_started;
}

