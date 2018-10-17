
/**
* copyright (c) 2018, James Flynn
* SPDX-License-Identifier: MIT
*/

/**
*   @file   azIoTClient.h
*   @brief  this header is included by azIoTClient.cpp and azClientFuncs.cpp to cover #defines and shared globals.
*
*   @author James Flynn
*
*   @date   17-Oct-2018
*/

#ifndef __AZIOTCLIENT_H__
#define __AZIOTCLIENT_H__


#define APP_VERSION             "1.0"
#define REPORT_PERIOD_RESOLUTION 10  //minimum reporting period in seconds 

#define IOT_AGENT_OK CODEFIRST_OK  //Microsoft code bug...

#define BAROMETER_CLICK          0x01
#define HTS221_CLICK             0x02
#define RELAY_CLICK              0x04
#define LIGHTRANGER_CLICK        0x08
#define FLAME_CLICK              0x10
#define HEARTRATE_CLICK          0x20

#define REPORTING_OBJECT_NAME    (char*)"Avnet M18x LTE SOM Azure IoT Client"
#define REPORTING_OBJECT_TYPE    (char*)"SensorData"
#define REPORTING_OBJECT_VERSION (char*)APP_VERSION
#define REPORTING_DEVICE         (char*)"M18QWG/M18Q2FG-1"

#ifndef __AZIOTCLIENT_CPP__

extern IOTHUB_CLIENT_LL_HANDLE  IoTHub_client_ll_handle;

extern int          gps_to;
extern int          report_period;
extern bool         verbose;
extern char         imei[25];
extern char         iccid[25];

extern Lis2dw12     mems;
extern Adc          adc;
extern Led          status_led;
extern Barometer    barom;
extern Hts221       humid;
extern Wncgps       gps;
extern unsigned int click_modules;

extern Led::Color  current_color;
extern Led::Action current_action;


#endif //__AZIOTCLIENT_CPP__
#endif // __AZIOTCLIENT_H__
