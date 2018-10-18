# M18QxAzureIoT
Azure IoT Client example program uses the Avnet M18Qx System On a Module (SOM)--*I use the acronym M18Qx when referring to the board in this README*.  The example program demonstrates connecting to Microsoft Azure IoT Hub, sending telemetry data and receiving commands/messages.  If used with the Avnet LTE IoT Breakout Carrier, Click Modules can be added and sensor readings from those modules will be sent also (currently it is only supports the **Temp&Humidity Click (https://www.mikroe.com/temp-hum-click)** and the **Barometer Click (https://www.mikroe.com/barometer-click)**).

The example is capable of running in a standalone configuration (only power required) but to load and interact with the board an ADB (Android Debug Bridge) session must be used.  If using an ADB session, the following is the startup screen that is displayed. 



         ****                       
        **  **     Azure IoTClient Example, version 1.0
       **    **    by AVNET
      ** ==== **
    
    This program uses the AT&T IoT Starter Kit, M18QWG (Global)/M18Q2FG-1 (North America) SoC 
    and interacts with Azure IoTHub sending sensor data and receiving messeages.
    
    Click-Barometer PRESENT!
    Click-Temp&Hum  PRESENT!
    
    ntp.org used to set the time: Wed Oct 17 19:30:37 2018

When started, the example program (*referred to as azIoTClient*) performs the following operations:

 - The Modem Abstraction Layer to the M18Qx is started 
 - Device Information (i.e., ICCID and IMEI) is collected 
 - Any Supported Click Modules that are connected are discovered 
 - A cellular connection is established and time obtained from *ntp.org*

After these tasks, azIoTClient begins iteratively sending sensor telemetry to the Azure IoT Hub. This behavior continues until you depress the USR button for >3 seconds at which time azIoTClient terminates.  The messages that are sent are similar to:

    {
      "ObjectName":"Avnet M18x LTE SOM Azure IoT Client",
      "ObjectType":"SensorData",
      "Version":"1.0",
      "ReportingDevice":"M18QWG/M18Q2FG-1",
      "DeviceICCID":"89011703278100836678",
      "DeviceIMEI":"353087080010952",
      "ADC_value":0.08,
      "lat":36.04,
      "long":-78.63,
      "Temperature":93.20,
      "Board Moved":1,
      "Board Position":10,
      "Report Period":10,
      "TOD":"Wed 2018-10-17 20:06:00 UTC",
      "Barometer":1011.98,
      "Humidity":61.5
    }


You can also  send messages from Azure IoT Hub to azIoTClient to elicit various messages or set  operational parameters. Currently, the messages you can send  are:

|Message/Command|Description  |
|--|--|
| REPORT-SENSORS | azIoTClient sends a list of available sensor |
|SET-PERIOD x |sets the reporting period for standard telemetry messages|
|GET-DEV-INFO |lists the information about this M18Qx device|
|GET-LOCATION |sends the current latitude/longitude location|
|GET-TEMP |sends the current temperature at the boards location|
|GET-POS |sends the positional information about the board|
|GET-ENV |sends  enviromental information about the boards location|
|LED-ON-MAGENTA |turns the boards LED to Magenta, always on|
|LED-BLINK-MAGENTA  |turns the boards LED to Magenta, blinking|
|LED-OFF |turns off the boards LED|

The remainder of the README.md discusses building and running azIoTClient and assumes you are using  a PC that has Ubuntu Linux installed and running (*other operating systems, e.g., Windows, are not covered here*).

## Prepare the development environment
Prepare the development environment by installing ADB and the compiler/tools.  Except for ADB, all necessary tools are available from  **http://github.com/Avnet/AvnetWNCSDK**

### Install ADB
1. Open up a terminal windows.
2. Install ADB by issuing the command '**sudo apt-get install adb**'
3. Use GIT to clone the AvnetWNCSDK by issuing the command **'git clone http://github.com/Avnet/AvnetWNCSDK'**
4. Copy the file file **adbpub.key** from the AvnetWNCSDK directory to the ADB directory (cp ./AvnetWNCSDK/adbpub.key ~/.android/).  This installs the security key so ADB can access the SK2.
5. Connect your M18Qx to the development PC
6. Verify the ADB connection by listing the attached devices, **sudo adb devices** When you do this the SK2 should be reported as:
```
    $sudo adb devices
    List of devices attached
    WNC_ADB	device
```
### Install the compiler and configure
1. Goto the AvnetWNCSDK directory

2. Install the tools by executing:  **sudo ./oecorex-86_64-cortexa7-neon-vfpv4-toolchain-nodistro.0.sh**

3. Add the environment variables (you need to do this any time you open a new terminal window for development) by executing: **" .    /usr/local/oecore-x86_64/environment-setup-cortexa7-neon-vfpv4-oe-linux-gnueabi "** *(there is a spaces after the '.')*

## Install the azIoTClient  source code
1. Move to the directory that is the root of your development work.

2. Clone the M18qxAzureIoT code by executing: **'git clone https://github.com/Avnet/M18QxAzureIoT'**

3. Go to the  newly cloned directory (**M18QxAzureIoT**)

4. Set the environment: **" .    /usr/local/oecore-x86_64/environment-setup-cortexa7-neon-vfpv4-oe-linux-gnueabi "**

5. run autogen: **./autogen.sh**

6. run configure: **"./configure ${CONFIGURE_FLAGS}"**

7. Modify **azClientFuncts.cpp** (line 26) and replace *connectionString* with the connection string for your Device.

The tools and source code are now installed and you can compile the code by typing: **"make"**

## Push the executable to the SK2
Using  ADB, push the executable image to the M18Qx and place it in the correct location.  The location you must use is **"/CUSTAPP/"**.  Execute the following:
```
   $ adb push azIoTClient /CUSTAPP/
    [100%] /CUSTAPP/azIoTClient
```

After the code is on the M18Qx,  you can execute it by entering an ADB shell and running it, e.g. '*adb shell "/CUSTAPP/azIoTClient -v"*'

## Program Flags and other Notes
When starting azIoTClient, there are several flags you can utilize:


|Flag|Description  |
|--|--|
|-g *X*  |Set a timeout (in seconds) to wait for a valid GPS fix to be obtained. If a valid fix is not obtained within this timeout, it simply returns a lat/long of 0/0  |
|-r *X* | Set the reporting time as *x* seconds. azIoTClient will send a standard telemetry message to Azure ~every *x* seconds -- ~ because this is the minimum time to wait
|-v | Display message contents as they are sent along with other informational data.
|-? | Display the flags and their explaination |

While running, The LED's on the M18Qx indicate various things:
* The WWAN Led:
	* Is off when no signal is detected by the modem
	* Flashes slowly when a signal is detected by we are not connected
	* Flashes quickly when connected but no IP address has been obtained
	* Is on continuously when connected and an IP address is assigned/obtained

* The LED is:
	- Color RED when initially powering up while the hardware is being initialized, it will then begin blinking when obtaining the M18Qx device information such as ICCID, IMEI, Time, and Click Module information.
	- Color is GREEN when in the while waiting to send information to Azure
	- Color is BLUE when sending/receiving message with Azure
	- Color is WHITE when  pressing the USR key
	- Color is MAGENTA if Azure has instructed the M18Qx to illuminate the LED

**Push button Operation:**
* if Pressed for >3 seconds, the M18Qx will exit azIoTClient

**Device Positional information**.  The M18Qx has an ST LIS2DW12 sensor on it and it uses this to report positional information about the orientation of the SOM (*The Face is the top of the SOM that  contains the WNC module*).  It can be:
* Face UP (10)
* Face DOWN (11)
* Face RIGHT (12)
* Face LEFT (13)
* Face FORWARD (14)
* Face AWAY (15)

**The SENSORS report contains**:
```
{
  "ObjectName":"%s",      
  "SOM"  : [ "ADC", "LIS2DW12-TEMP", "LIS2DW12-POS", "GPS" ],
  "CLICK": [ "BAROMETER", "TEMP&HUMID" ]
}
```
**The DEVICE-INFO report contains**:
```
{
  "ObjectName":"Device-Info",      
  "ReportingDevice":"%s", 
  "DeviceICCID":"%s",     
  "DeviceIMEI":"%s",      
}
```

**The LOCATION  report contains**:
```
{
  "ObjectName":"location-report",      
  "lat":%.02f,              
  "long":%.02f,             
}
```
**The TEMP  report contains**:
```
{
  "ObjectName":"temp-report",      
  "Temperature":%.02f,      
}
```
**The POSITION report contains**:

```
{
  "ObjectName":"board-position",      
  "Board Moved":%d,        
  "Board Position":%d,      
}
```

**The ENVIROMENT report contains**:
```
{
  "ObjectName":"barometer-report",      
  "Barometer":%.02f,
  "Humidity:%.01f
}
```




