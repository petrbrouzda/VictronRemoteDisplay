#ifndef __MY_CONFIG_H_
#define __MY_CONFIG_H_

/****************** FV system parameters ***************/

#define SOLAR_PANELS_POWER_WP (6.0*450.0)
#define INVERTER_POWER_W 2400.0
#define BATTERY_CAPACITY_KWH 7.0

// if battery gets hotter than that, red text will be displayed
#define BATTERY_TEMP_LIMIT 28


/****************** hardware configuration ***************/


// WiFi restart interval if there is no connection
#define WIFI_NOCONN_RESTART_SEC 30

// parametry pripojeni displeje
#define TFT_CS   7
#define TFT_RST  10
#define TFT_DC   6
#define TFT_MOSI 3
#define TFT_SCLK 2

#define SPI_MISO_UNUSED -1

// nastavení RST do režimu ne-reset
#define RST_VALUE HIGH 

// LOW = vypnuto, HIGH = zapnuto
#define TFT_BACKLIGHT 5

#define LED 12
#define LED_UNUSED 13

#define TFT_BACKLIGHT_ON HIGH

// comment out if you have no solar-resistor in circuit
#define USE_BRIGHTNESS_SENSOR 

#ifdef USE_BRIGHTNESS_SENSOR 
    #define BRIGHTNESS_SENSOR_PIN 0

    #define SENSOR_U_MIN 0.6
    #define SENSOR_U_MAX 2.0

    #define BL_MAX 255.0
    #define BL_MIN 10.0
#endif


/****************** connection parameters ***************/



// WiFi
#define WIFI_SSID (const char *)"venus-XXXXX-xxx"
#define WIFI_PASSWORD (const char *)"xxxxxxxxx"

// MQTT Broker
#define MQTT_BROKER (const char *)"172.24.24.1"
#define MQTT_USERNAME (const char *)""
#define MQTT_PASSWORD (const char *)""
#define MQTT_PORT 1883




#endif