#commands that can be sent:
# REPORT-SENSORS 
# SET-PERIOD x 
# GET-DEV-INFO 
# GET-LOCATION 
# GET-TEMP 
# GET-POS 
# GET-ENV
# LED-ON-MAGENTA 
# LED-BLINK-MAGENTA  
# LED-OFF 

az iot device c2d-message send --login "HostName=AvnetBG96-Example.azure-devices.net;SharedAccessKeyName=iothubowner;SharedAccessKey=hH+1sO3xh8J0P9NuTtNXQOZ901MN9amgFbVb8t+nwoY=" --device-id STL496ZG-BG96 --data "$*"

