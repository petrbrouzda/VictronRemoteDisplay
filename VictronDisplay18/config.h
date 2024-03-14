#ifndef __MY_CONFIG_H_
#define __MY_CONFIG_H_

/****************** FV system parameters ***************/

#define SOLAR_PANELS_POWER_WP (6.0*450.0)
#define INVERTER_POWER_W 2400.0
#define BATTERY_CAPACITY_KWH 3.5
#define INVERTER_OWN_CONSUMPTION 20.0


/****************** hardware configuration ***************/

// LED indicator
#define LED 12

// WiFi restart interval if there is no connection
#define WIFI_NOCONN_RESTART_SEC 600

// working set of pins on ESP32-C3 https://wiki.luatos.com/chips/esp32c3/board.html
#define TFT_CS        7
#define TFT_RST        6
#define TFT_DC        5 
#define TFT_MOSI 3
#define TFT_SCLK 2

#define SPI_MISO_UNUSED 10

// LOW = backlight off
#define TFT_BACKLIGHT 4


/****************** connection parameters ***************/


// WiFi
#define WIFI_SSID (const char *)"venus-XXXXXXXX-xxx"
#define WIFI_PASSWORD (const char *)"xxxxxxxx"

// MQTT Broker
#define MQTT_BROKER (const char *)"172.24.24.1"
#define MQTT_USERNAME (const char *)""
#define MQTT_PASSWORD (const char *)""
#define MQTT_PORT 1883


#endif