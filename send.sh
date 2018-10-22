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

az iot device c2d-message send --login "HostName=XXX;SharedAccessKeyName=XXX;SharedAccessKey=XXX" --device-id XXX --data "$*"

