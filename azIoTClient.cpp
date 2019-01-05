/**
* copyright (c) 2018, James Flynn
* SPDX-License-Identifier: MIT
*/

#define __AZIOTCLIENT_CPP__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/un.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <termios.h>
#include <stdarg.h>

#include "azIoTClient.h"

#include "iothub_client_ll.h"

#ifndef __HWLIB__
extern "C" {
#include <hwlib/hwlib.h>
}
#endif // __HWLIB__

#include "gps.hpp"
#include "devinfo.hpp"
#include "spi.hpp"
#include "led.hpp"
#include "i2c.hpp"
#include "lis2dw12.hpp"
#include "button.hpp"
#include "adc.hpp"
#include "NTPClient.hpp"
#include "barometer.hpp"
#include "hts221.hpp"
#include "wwan.hpp"

#include "azIoTClient.h"

IOTHUB_CLIENT_LL_HANDLE  setup_azure(void);
void sendMessage(IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle, char* buffer, size_t size);
void button_release(int);
void bb_release(int);             //boot button release

Led::Color   current_color;
Led::Action  current_action;
char         imei[25];
char         iccid[25];
int          report_period = 10;  //default to 10 second reports
bool         verbose = false;     //default to quiet mode
bool         done = false;        //not yet done

bool         use_uart2 = false;   //is true when using UART2
int          lpm_enabled = 0;     //Low Power Modes defined below...
unsigned int click_modules = 0;   //no Click Modules present

//Low Power Modes
#define NO_LPM     0
#define ENTER_LPM  1
#define IN_LPM     2
#define EXIT_LPM   3


//if using UART2, the following are needed
struct termios options;
int uart2_fd;

IOTHUB_CLIENT_LL_HANDLE  IoTHub_client_ll_handle;

Wncgps    gps;
Lis2dw12  mems(GPIO_PIN_6, GPIO_PIN_7);
Adc       adc;
Led       status_led(GPIO_PIN_92, GPIO_PIN_102, GPIO_PIN_101);
Barometer barom(LPS25HB_SAD);
Hts221    humid(HTS221_SAD);
Button    user_button(GPIO_PIN_98, BUTTON_ACTIVE_HIGH, button_release);
Button    boot_button(GPIO_PIN_1, BUTTON_ACTIVE_LOW, bb_release);  //handle the boot button
Devinfo   device;

//
// arguments the program takes during startup.
//
void usage (void)
{
    printf(" The 'azIoTClient' program can be started with several options:\n");
    printf(" -u  : Enable/Use UART2.\n");
    printf(" -v  : Display Messages as sent.\n");
    printf(" -r X: Set the reporting period in 'X' (seconds)\n");
    printf(" -?  : Display usage info\n");
}

void button_press(void)
{
    current_color = status_led.color(Led::CURRENT);
    status_led.action(Led::LED_ON,Led::WHITE);
    current_action= status_led.action(Led::LOCK);
}

void button_release( int dur )
{
    status_led.action(Led::UNLOCK);
    status_led.action(current_action, current_color);
    if( dur > 3 ) {
        status_led.set_interval(125);
        done = true;
        }
}

void bb_press( void )
{
    current_color = status_led.color(Led::CURRENT);
    status_led.action(Led::LED_ON,Led::WHITE);
    current_action= status_led.action(Led::LOCK);
}

void bb_release( int dur )
{
    status_led.action(Led::UNLOCK);
    status_led.action(current_action, current_color);
    switch (lpm_enabled) {
        case NO_LPM:
            lpm_enabled = ENTER_LPM;
            break;

        case IN_LPM:
            lpm_enabled = EXIT_LPM;
            break;

        default: 
            break;
        }
}

/* Standard Report sent to Azure repeatedly */
#define MSG_LEN                    512
#define IOTDEVICE_MSG_FORMAT       \
   "{"                             \
     "\"ObjectName\":\"%s\","      \
     "\"ObjectType\":\"%s\","      \
     "\"Version\":\"%s\","         \
     "\"ReportingDevice\":\"%s\"," \
     "\"DeviceICCID\":\"%s\","     \
     "\"DeviceIMEI\":\"%s\","      \
     "\"ADC_value\":%.02f,"        \
     "\"last GPS fix\":\"%s\","    \
     "\"lat\":%.02f,"              \
     "\"long\":%.02f,"             \
     "\"Temperature\":%.02f,"      \
     "\"Board Moved\":%d,"         \
     "\"Board Position\":%d,"      \
     "\"Report Period\":%d,"       \
     "\"TOD\":\"%s UTC\""          

char* make_message(char* iccid, char* imei)
{
    gpsstatus *loc;
    char      buffer[25], temp[25];
    char*     ptr = (char*)malloc(MSG_LEN);
    time_t    rawtime;
    struct tm *ptm;

    time(&rawtime);
    ptm = gmtime(&rawtime);
    strftime(buffer,sizeof(buffer),"%a %F %X",ptm);

    loc = gps.getLocation();
    ptm = gmtime(&loc->last_good);
    strftime(temp,sizeof(temp),"%a %F %X",ptm);

    snprintf(ptr, MSG_LEN, IOTDEVICE_MSG_FORMAT, 
                           REPORTING_OBJECT_NAME,
                           REPORTING_OBJECT_TYPE,
                           REPORTING_OBJECT_VERSION,
                           REPORTING_DEVICE,
                           iccid,
                           imei,
                           (float)adc,
                           temp,
                           loc->last_pos.lat,
                           loc->last_pos.lng,
                           mems.lis2dw12_getTemp(),
                           mems.movement_ocured(),
                           mems.lis2dw12_getPosition(),
                           report_period,
                           buffer);

    if( click_modules & BAROMETER_CLICK ) {
        snprintf(temp, sizeof(temp), ",\"Barometer\":%.02f", barom.get_pressure());
        strcat(ptr,temp);
        }

    if( click_modules & HTS221_CLICK ) {
        snprintf(temp, sizeof(temp), ",\"Humidity\":%.01f",  humid.readHumidity());
        strcat(ptr,temp);
        }

    strcat( ptr, "}");

    int c = (strstr(buffer,":")-buffer) - 2;
    printf("Send IoTHubClient Message@%s - ",&buffer[c]);
    return ptr;
}

void verbose_output( const char * format, ... )
{
    char buffer[256];
    va_list args;
    va_start (args, format);
    vsprintf (buffer,format, args);
    va_end (args);
    if(verbose) 
        printf("%s",buffer);
    if(use_uart2) {
        write(uart2_fd, buffer, strlen(buffer));

        int n = read(uart2_fd, buffer, sizeof(buffer));
        if( n>0 ){
            buffer[n] = '\0';

            printf("Read (%i bytes): %s", n, buffer);
            if( strstr(buffer,"lpm") ) {
              if( strstr(buffer, "on") )
                 lpm_enabled = ENTER_LPM;
              if( strstr(buffer, "off") )
                 lpm_enabled = EXIT_LPM;
              }
            }
        }
    fflush(stdout);

}

int main(int argc, char *argv[]) 
{
    int            i, msg_sent=1;
    char          *ptr;
    Wwan           wan_led;
    void           prty_json(char* src, int srclen);
    NTPClient      ntp;
    time_t         timestamp=-1;
    struct timeval time_sent, time_now;

    gettimeofday(&time_sent, NULL);
    gettimeofday(&time_now, NULL);

    status_led.action(Led::LED_ON,Led::RED);
    user_button.button_press_cb( button_press );
    boot_button.button_press_cb( bb_press );

    while((i=getopt(argc,argv,"uvr:?")) != -1 )
        switch(i) {
           case 'u':
               use_uart2 = true; 
               uart2_fd = open("/dev/ttyHSL0", O_RDWR | O_NOCTTY | O_NDELAY);
               if (uart2_fd == -1) {
                   printf("Unable to open UART2!");
                   exit(EXIT_FAILURE);
                   }
               else {
                   fcntl(uart2_fd, F_SETFL, FNDELAY);
                   tcgetattr(uart2_fd, &options);
                   cfsetspeed(&options, B115200);
                   tcsetattr(uart2_fd, TCSANOW, &options);
                   int n = write(uart2_fd, "Using UART2!\n",13);
                   if (n < 0) {
                     printf("Write to UART2 failed!");
                     exit(EXIT_FAILURE);
                     }
                   }
               break;
           case 'v':
               verbose = true;
               printf(">> output messages as sent\n");
               break;
           case 'r':
               sscanf(optarg,"%x",&report_period);
               i=report_period % REPORT_PERIOD_RESOLUTION;
               if( i != 0 )
                   report_period += (REPORT_PERIOD_RESOLUTION-i);
               printf(">> auto update every %d seconds ",report_period);
               printf("(reports in %dx second increments)\n",REPORT_PERIOD_RESOLUTION);
               break;
           case '?':
               usage();
               exit(EXIT_SUCCESS);
           default:
               fprintf (stderr, ">> nknown option character `\\x%x'.\n", optopt);
               exit(EXIT_FAILURE);
           }

    printf("\n\n");
    printf("     ****\r\n");
    printf("    **  **     Azure IoTClient Example, version %s\r\n", APP_VERSION);
    printf("   **    **    by AVNET\r\n");
    printf("  ** ==== **\r\n");
    printf("\r\n");
    printf("This program uses the AT&T IoT Starter Kit, M18QWG (Global)/M18Q2FG-1 (North America) SoC \r\n");
    printf("and interacts with Azure IoTHub sending sensor data and receiving messeages.\r\n");
    printf("\r\n");

    status_led.action(Led::LED_BLINK,Led::RED);
    device.getICCID(iccid,sizeof(iccid));
    device.getIMEI(imei,sizeof(imei));
    verbose_output("ICCID= %s\nIMEI = %s\n\n",iccid,imei);

    click_modules |= (barom.who_am_i()==LPS25HB_WHO_AM_I)? BAROMETER_CLICK:0;
    click_modules |= (humid.who_am_i()==I_AM_HTS221)? HTS221_CLICK:0;

    if( click_modules & BAROMETER_CLICK ) printf("Click-Barometer PRESENT!\n");
    if( click_modules & HTS221_CLICK ) printf(   "Click-Temp&Hum  PRESENT!\n\n");

    status_led.set_interval(125);
    status_led.action(Led::LED_BLINK,Led::GREEN);
    gps.enable();
    i=0;
    while( !gps.status() && !user_button.chkButton_press() ) {
        printf("\rGetting Initial GPS Location Fix. (%d)",++i);
        fflush(stdout);
        sleep(1);
        }
    printf("\n");
    gpsstatus *loc = gps.getLocation();
    printf("Latitude = %f\n", loc->last_pos.lat);
    printf("Longitude= %f\n\n", loc->last_pos.lng);

    i=0;
    wan_led.enable();
    while( timestamp == -1 ) {
        timestamp=ntp.get_timestamp();
        printf("\rWait for Cellular Connection (%d)",i++);
        fflush(stdout);
        sleep(1);
        }

    stime(&timestamp);
    printf("\rntp.org used to set the time: %s     \n",ctime(&timestamp));

    status_led.set_interval(500);
    status_led.action(Led::LED_ON,Led::GREEN);
    verbose_output("Now, establish connection with Azure IoT Hub.\n\n");
    IoTHub_client_ll_handle =setup_azure();
    if( IoTHub_client_ll_handle == NULL ) {
        printf("ERROR:couldn't connect to Azure!\n");
        exit(EXIT_FAILURE);
        }

    status_led.action(Led::LED_ON,Led::GREEN);
    lpm_enabled = NO_LPM;

    while( !done ) {
        switch (lpm_enabled) {
            case ENTER_LPM:
                verbose_output("\nEnter Low Power Mode.\n");
                gps.disable();
                if( device.setLPM(true) == 0)
                    lpm_enabled = IN_LPM;
                status_led.set_interval(2000);
                status_led.action(Led::LED_BLINK,Led::RED);
                break;

            case EXIT_LPM:
                verbose_output("\nExit Low Power Mode.\n");
                gps.enable();
                if( device.setLPM(false) == 0 ) 
                    lpm_enabled = NO_LPM;
                status_led.action(Led::LED_ON,Led::GREEN);
                break;

            case IN_LPM:
                verbose_output("Low Power Mode active, Sleeping (%3d seconds)\r",i++);
                fflush(stdout);
                sleep(1);
                break;

            case NO_LPM:
                gettimeofday(&time_now, NULL);
                if( difftime(time_now.tv_sec, time_sent.tv_sec) >= report_period ) {
                    time_sent = time_now;
                    status_led.action(Led::LED_ON,Led::BLUE);
                    printf("(%04d)",msg_sent++);
                    ptr = make_message(iccid, imei);
                    sendMessage(IoTHub_client_ll_handle, ptr, strlen(ptr));
                    prty_json(ptr,strlen(ptr));
                    free(ptr);
                    status_led.action(Led::LED_ON,Led::GREEN);
                    }
        
                IoTHubClient_LL_DoWork(IoTHub_client_ll_handle);
                break;

            }
        }

    status_led.set_interval(125);
    status_led.action(Led::LED_BLINK,Led::RED);

    gps.terminate();
    user_button.terminate();
    boot_button.terminate();
    mems.terminate();

    if(verbose) printf("\nClosing connection to Azure IoT Hub...\n\n");
    IoTHubClient_LL_Destroy(IoTHub_client_ll_handle);

    status_led.terminate();
    wan_led.terminate();

    printf(" - - - - - - - ALL DONE - - - - - - - \n");
    exit(EXIT_SUCCESS);
}

