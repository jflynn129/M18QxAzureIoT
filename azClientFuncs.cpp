/**
* copyright (c) 2018, James Flynn
* SPDX-License-Identifier: MIT
*/


//#define USE_MQTT //define USE_MQTT to use MQTT, otherwise it will use HTTP

#include <stdlib.h>

#ifdef USE_MQTT
#include "iothubtransportmqtt.h"
#else
#include "iothubtransporthttp.h"
#endif
#include "iothub_client_core_common.h"
#include "iothub_client_ll.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/agenttime.h"
#include "jsondecoder.h"

#include "led.hpp"
#include "lis2dw12.hpp"
#include "adc.hpp"
#include "barometer.hpp"
#include "hts221.hpp"
#include "gps.hpp"

#include "azure_certs.h"

#include "azIoTClient.h"

//The following connection string must be updated for the individual users Azure IoT Device
static const char* connectionString = "HostName=XXXX;DeviceId=xxxx;SharedAccessKey=xxxx";


extern void sendMessage(IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle, char* buffer, size_t size);
extern void prty_json(char* src, int srclen);

char* send_sensrpt(void);
char* send_devrpt(void);
char* send_locrpt(void);
char* send_temprpt(void);
char* send_posrpt(void);
char* send_envrpt(void);
IOTHUBMESSAGE_DISPOSITION_RESULT receiveMessageCallback( IOTHUB_MESSAGE_HANDLE message, void *userContextCallback);

void sendMessage(IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle, char* buffer, size_t size)
{
    IOTHUB_MESSAGE_HANDLE messageHandle = IoTHubMessage_CreateFromByteArray((const unsigned char*)buffer, size);
    if (messageHandle == NULL) {
        printf("unable to create a new IoTHubMessage\r\n");
        return;
        }
    if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messageHandle, NULL, NULL) != IOTHUB_CLIENT_OK)
        printf("FAILED to send!\n");
    else
        printf("OK\n");

    IoTHubMessage_Destroy(messageHandle);
}

IOTHUB_CLIENT_LL_HANDLE setup_azure(void)
{

    /* Setup IoTHub client configuration */
#ifdef IOTHUBTRANSPORTHTTP_H
    IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, HTTP_Protocol);
#else
    IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, MQTT_Protocol);
#endif
    if (iotHubClientHandle == NULL) {
        printf("Failed on IoTHubClient_Create\r\n");
        return NULL;
        }

    // add the certificate information
    if (IoTHubClient_LL_SetOption(iotHubClientHandle, "TrustedCerts", certificates) != IOTHUB_CLIENT_OK) {
        IoTHubClient_LL_Destroy(iotHubClientHandle);
        printf("failure to set option \"TrustedCerts\"\r\n");
        return NULL;
        }

#ifdef IOTHUBTRANSPORTHTTP_H
    // polls will happen effectively at ~10 seconds.  The default value of minimumPollingTime is 25 minutes. 
    // For more information, see:
    //     https://azure.microsoft.com/documentation/articles/iot-hub-devguide/#messaging

    unsigned int minimumPollingTime = 9;
    if (IoTHubClient_LL_SetOption(iotHubClientHandle, "MinimumPollingTime", &minimumPollingTime) != IOTHUB_CLIENT_OK) {
        IoTHubClient_LL_Destroy(iotHubClientHandle);
        printf("failure to set option \"MinimumPollingTime\"\r\n");
        return NULL;
        }
#endif

    // set C2D and device method callback
    IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, receiveMessageCallback, NULL);
    return iotHubClientHandle;
}

//------------------------------------------------------------------
#define SENS_REPORT "{"                                           \
  "\"ObjectName\":\"sensor-report\","                             \
  "\"SOM\":[\"ADC\",\"LIS2DW12-TEMP\",\"LIS2DW12-POS\",\"GPS\"]," \
  "\"CLICK\":["

char* send_sensrpt(void)
{
    int   len = sizeof(SENS_REPORT)+30;
    char* ptr = (char*)malloc(len);

    snprintf(ptr,len,SENS_REPORT);

    if( click_modules & (BAROMETER_CLICK|HTS221_CLICK) ) {
        if (click_modules & BAROMETER_CLICK )
            strcat(ptr,"\"BAROMETER\"");

        if (click_modules & HTS221_CLICK )
            strcat(ptr,",\"TEMP&HUMID\"");
        }
    else
        strcat(ptr,"\"NONE\"");

    strcat(ptr,"]}");
    return ptr;
}

//------------------------------------------------------------------
#define DEV_REPORT "{"                             \
  "\"ObjectName\":\"Device-Info\","                \
  "\"ReportingDevice\":\"M18QWG/M18Q2FG-1\","      \
  "\"DeviceICCID\":\"%s\","                        \
  "\"DeviceIMEI\":\"%s\""                          \
  "}"

char* send_devrpt(void)
{
    int     len = sizeof(DEV_REPORT)+50;
    char*   ptr = (char*)malloc(len);

    snprintf(ptr,len,DEV_REPORT, iccid, imei);
    return ptr;
}

//------------------------------------------------------------------
#define LOC_REPORT "{"                  \
  "\"ObjectName\":\"location-report\"," \
  "\"last GPS fix\":\"%s\","            \
  "\"lat\":%.02f,"                      \
  "\"long\":%.02f"                      \
  "}"

char* send_locrpt(void)
{
    gpsstatus *loc;
    char      temp[25];
    struct tm *ptm;
    int       len = sizeof(LOC_REPORT)+35;
    char*     ptr = (char*)malloc(len);

    loc = gps.getLocation();
    ptm = gmtime(&loc->last_good);
    strftime(temp,25,"%a %F %X",ptm);

    snprintf(ptr,len, LOC_REPORT, temp, loc->last_pos.lat, loc->last_pos.lng);
    return ptr;
}

//------------------------------------------------------------------
#define TEMP_REPORT "{"             \
  "\"ObjectName\":\"temp-report\"," \
  "\"Temperature\":%.02f"           \
  "}"

char* send_temprpt(void)
{
    int   len = sizeof(TEMP_REPORT)+10;
    char* ptr = (char*)malloc(len);

    snprintf(ptr,len,TEMP_REPORT, mems.lis2dw12_getTemp());
    return ptr;
}

//------------------------------------------------------------------
#define POS_REPORT "{"                 \
  "\"ObjectName\":\"board-position\"," \
  "\"Board Moved\":%d,"                \
  "\"Board Position\":%d"              \
  "}"

char* send_posrpt(void)
{
    int   len = sizeof(POS_REPORT)+10;
    char* ptr = (char*)malloc(len);

    snprintf(ptr,len,POS_REPORT, mems.movement_ocured(), mems.lis2dw12_getPosition());
    return ptr;        
}

//------------------------------------------------------------------
#define ENV_REPORT "{"                    \
  "\"ObjectName\":\"enviroment-report\"," \
  "\"Barometer\":%.02f,"                  \
  "\"Humidity\":%.01f"                    \
  "}"

char* send_envrpt(void)
{
    int   len = sizeof(ENV_REPORT)+15;
    char* ptr = (char*)malloc(len);

    snprintf(ptr,len,ENV_REPORT,
            (click_modules & BAROMETER_CLICK )? barom.get_pressure():0,
            (click_modules & HTS221_CLICK )? humid.readHumidity():0   );
    return ptr;
}

IOTHUBMESSAGE_DISPOSITION_RESULT receiveMessageCallback(
    IOTHUB_MESSAGE_HANDLE message, 
    void *userContextCallback)
{
    const unsigned char *buffer = NULL;
    char* pmsg = NULL;
    size_t size = 0;

    if (IOTHUB_MESSAGE_OK != IoTHubMessage_GetByteArray(message, &buffer, &size))
        return IOTHUBMESSAGE_ABANDONED;

    // message needs to be converted to zero terminated string
    char * temp = (char *)malloc(size + 1);
    if (temp == NULL)
        return IOTHUBMESSAGE_ABANDONED;

    strncpy(temp, (char*)buffer, size);
    temp[size] = '\0';

    if( !strcmp(temp, "REPORT-SENSORS") )
        pmsg = send_sensrpt();
    else if( !strcmp(temp, "GET-DEV-INFO") )
        pmsg = send_devrpt();
    else if( !strcmp(temp, "GET-LOCATION") )
        pmsg = send_locrpt();
    else if( !strcmp(temp, "GET-TEMP") )
        pmsg = send_temprpt();
    else if( !strcmp(temp, "GET-POS") )
        pmsg = send_posrpt();
    else if( !strcmp(temp, "GET-ENV") )
        pmsg = send_envrpt();
    else if( !strcmp(temp, "LED-ON-MAGENTA") ){
        status_led.action(Led::LED_ON,Led::MAGENTA);
        if( verbose ) printf("Turning LED on to Magenta.\n");
        }
    else if( !strcmp(temp, "LED-BLINK-MAGENTA") ){
        status_led.action(Led::LED_BLINK,Led::MAGENTA);
        if( verbose ) printf("Setting LED to blink Magenta\n");
        }
    else if( !strcmp(temp, "LED-OFF") ){
        status_led.action(Led::LED_OFF,Led::BLACK);
        if( verbose ) printf("Turning LED off.\n");
        }
    else if( strstr(temp, "SET-PERIOD") ) {
        sscanf(temp,"SET-PERIOD %d",&report_period);
        int i=report_period % REPORT_PERIOD_RESOLUTION;
        if( i != 0 )
            report_period += (REPORT_PERIOD_RESOLUTION-i);
        if( verbose ) printf("Report Period remotely set to %d.\n",report_period);
        }
    else
        printf("Received message: '%s'\r\n", temp);

    if( pmsg != NULL ) {
        printf("(----)Azure IoT Hub requested response sent - ");
        sendMessage(IoTHub_client_ll_handle, pmsg, strlen(pmsg));
        if( verbose )
            prty_json(pmsg,strlen(pmsg));
        free(pmsg);
        }
    free(temp);
    return IOTHUBMESSAGE_ACCEPTED;
}




