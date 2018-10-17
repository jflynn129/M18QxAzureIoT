/**
* copyright (c) 2018, James Flynn
* SPDX-License-Identifier: MIT
*/

/**
*   @file   button.hpp
*   @brief  A NTP Client class for obtaining the time from NTP.ORG
*
*   @author James Flynn
*
*   @date   24-Aug-2018
*/

#ifndef __NTPCLIENT_HPP__
#define __NTPCLIENT_HPP__

#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#define NTP_DEFULT_NIST_SERVER_ADDRESS "2.pool.ntp.org"
#define NTP_DEFULT_NIST_SERVER_PORT    "123"

class NTPClient {
    public:
        NTPClient() : nist_server_address((char *)NTP_DEFULT_NIST_SERVER_ADDRESS), 
                      nist_server_port((char *)NTP_DEFULT_NIST_SERVER_PORT) {}

        void set_server(char* server, char* port) {
            nist_server_address = server;
            nist_server_port = port;
            }

        time_t get_timestamp(void) {
            struct addrinfo hints, *result, *rp;
            ssize_t         i=-1;
            int             sfd;
            const time_t TIME1970 = (time_t)2208988800UL;
            int ntp_send_values[12] = {0};
            int ntp_recv_values[12] = {0};

            memset(ntp_recv_values, 0x00, sizeof(ntp_recv_values));
            memset(ntp_send_values, 0x00, sizeof(ntp_send_values));
            ntp_send_values[0] = '\x1b';

            // Obtain address(es) matching host/port.. there may be more than 1
            memset(&hints, 0, sizeof(struct addrinfo));
            hints.ai_flags    = 0;
            hints.ai_family   = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
            hints.ai_socktype = SOCK_DGRAM;   /* Datagram socket */
            hints.ai_protocol = 0;            /* Any protocol */

            if( getaddrinfo(NTP_DEFULT_NIST_SERVER_ADDRESS, NTP_DEFULT_NIST_SERVER_PORT, &hints, &result) ) 
                return -1;

            // getaddrinfo() returns a list of address structures. Try each address until we 
            // successfully connect

            for (rp = result; rp != NULL; rp = rp->ai_next) {
                sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
                if (sfd == -1)
                    continue;
                if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
                    break;                  /* Success */
                close(sfd);
                }

            freeaddrinfo(result);
            if (rp == NULL)  // No valid address found 
                return -1;

            if (write(sfd, ntp_send_values, sizeof(ntp_send_values)) == sizeof(ntp_send_values) ) 
                i = read(sfd, ntp_recv_values, sizeof(ntp_recv_values));
            close(sfd);

            if (i > 10) 
                return ntohl(ntp_recv_values[10]) - TIME1970;
            else
                return -1; // No or partial data returned
            }

    private:
        char*             nist_server_address;
        char*             nist_server_port;

        uint32_t ntohl(uint32_t x) {
            uint32_t ret = (x & 0xff) << 24;
            ret |= (x & 0xff00) << 8;
            ret |= (x & 0xff0000UL) >> 8;
            ret |= (x & 0xff000000UL) >> 24;
            return ret;
            }
};

#endif // __NTPCLIENT_HPP__

