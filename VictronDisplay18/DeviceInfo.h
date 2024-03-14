#ifndef DEVICE_INFO__H
#define DEVICE_INFO__H

#define WIFI_POWER_OFF -999

void formatDeviceInfo( char * out );
char * getWifiStatusText( int status, char * desc );

#endif
