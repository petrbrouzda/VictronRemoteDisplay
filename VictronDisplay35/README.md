# VictronRemoteDisplay
Simple remote (wifi connected) status display for Victron photovoltaic system (with Cerbo GX central system) without cloud access - for environments without internet.

Victron photovoltaic systems with Cerbo GX has display module (GX Touch 50, GX Touch 70) but they are connected to Cerbo by cable. And they are touch-enabled - you can change view, you can change system settings. 

I need wifi-connected display with **only** passive function (no configuration changes), and it run in no-internet environment (no connection to Victron VRM cloud).

So I've build this:

![](/imgs2/2025-05-05%2018.51.06.jpg) 

It runs basic [ESP32-C3 board](https://s.click.aliexpress.com/e/_DEw2w8v) with [cheap 3.5" SPI display](https://s.click.aliexpress.com/e/_opxDnTV). 

Connection (display pin - ESP32 pin) is defined in config.h:
- CS - GPIO 7
- RST - GPIO 10
- DC - GPIO 6 
- SDA - GPIO 3
- SCL - GPIO 2
- BKL - GPIO 5
- VCC - 3V3
- GND - GND

![](/imgs2/2025-04-14%2009.21.35.jpg) 

There is also photoresistor for automatic backlight intensity setting. 
It is connected to GPIO 0 and used in voltage divider - so when there is much light around, voltage on GPIO 0 drops to zero; in darkness, voltage is >2.5 V.

![](/imgs2/2025-05-06%2007.23.00.jpg)

If you haven't photoresistor, you can disable the functionality by commenting out this line in config.h:

``#define USE_BRIGHTNESS_SENSOR 
``

Box is 3D printed, STL files are in [box-display35](/box-display35/) folder.



## How it works

Cerbo GX offers WiFi access point. And Cerbo GX has MQTT broker built in - you just have to switch it on in Settings. 

ESP32 can connect to Cerbo's WiFi, then connect to MQTT broker and get all information about your photovoltaic system. Seems easy.

But when you connect to Cerbo's MQTT and subscribe to all topics ("#"), you've got only one topic with ID of Cerbo's system:

![](/imgs/empty.png)

You have to read the system ID and publish **keeepalive message**: empty message to topic `R/<system ID>/keepalive` - for example `R/c0619ab33b2f/keepalive`

After you send the message, MQTT will magically starts filling with data:
![](/imgs/full.png)

But your happiness will last only for one minute, then Cerbo stops publishing more data until you send another keepalive message. So you have to send keepalive message periodically, every 55 seconds.

## How to run it

This is tested on ESP32 core for arduino version **2.0.17** (version in "board manager" in Arduino IDE). It **will not** run on current 3.0.x versions as some APIs changed in them. Switch board version to 2.0.17 before building it!

1) If your Cerbo is not connected to WiFi: switch on WiFi access point in Cerbo GX.

2) In Cerbo's console, go to **Settings - Services** and switch on **MQTT on Lan (SSL)**. Then another option will appear: **MQTT on Lan (plaintext)**. Switch it on also.

3) Edit **config.h** section with your photovoltaic system limits:

```
#define SOLAR_PANELS_POWER_WP (6.0*450.0)
#define INVERTER_POWER_W 2400.0
#define BATTERY_CAPACITY_KWH 7.0
```

4) Edit **config.h** section with connection parameters. MQTT broker address is address of Cerbo GX.

```
// WiFi
#define WIFI_SSID (const char *)"venus-XXXXXXXX-xxx"
#define WIFI_PASSWORD (const char *)"xxxxxxxx"

// MQTT Broker
#define MQTT_BROKER (const char *)"172.24.24.1"
```

5) Board configuration:
- if you have board with serial-to-USB converter: `USB-CDC on boot: disabled`
- if you have board with no serial-to-USB converter, with direct USB to ESP32-C3: `USB-CDC on boot: enabled`
- `Flash mode: DIO`
- `CPU speed: 80 MHz`
- `Flash speed: 40 MHz`

6) Used libraries (should be installed in library manager):
* PubSubClient 2.8 
* Tasker 2.0.3
* ArduinoJson 6.21.5 
* Adafruit ST7735 and ST7789 Library 1.11.0
* Adafruit GFX Library 1.12.0
* Adafruit BusIO  1.17.0
* ESP32AnalogRead 0.3.0 


7) Build application, send it into ESP and enjoy.

## What is displayed?

![](/imgs2/2025-05-05%2018.51.03.jpg)

**First bar** is **battery status.** 
* Battery icon shows if battery is charged or discharged. 
* Percentage is shown as "usable capacity". Battery will cut off at 20 %, so you've got technical reading 20 % - 100 %, which will be displayed as 0 % - 100 % here. 
* First data item (50 W) is power consumed (negative) or charging (positive). 
* Then estimated energy available (5.6 kWh); and when battery is discharged, estimated time battery will be able to suply current consumption.

Variants:

![](/imgs2/display1.png)
* battery is discharging at 1841 W, estimated energy available is 5.6 kWh and it will remain for 3 hours
* battery is charging by 30 W, 5.6 kWh is available

**Second bar** is status of **solar panels**. 103 W in bar is current power supplied by panels. 
* First data row contains: 120 V = panels voltage; 0.6 kWh = today's yield; max 357 W = today maximum power supplied
* Second data row: yield yesterday / two days back / three days back; summary yield during last 31 days

Then two charts are displayed:
* panels power (W) in every minute for the last 3.5 hour
* daily yield (kWh) for last 31 days

**Third bar** is **consumption**. 41 W consumed at the moment.

It is followed by chart of consumed power in every minute for the last 3.5 hours - so you can see that the maximal consumption was 1742 W.

At the bottom three rows of status data are displayed:
* Temperature of batteries and inverter.
* WiFi strength; number of messages processed in last minute (as debug information - if connection is somewhat broken, there will be 0/m); display uptime
* Status LEDs of inverter.


## Application limitations

Application has been tested on three photovoltaic systems only.

It is not generic / universal tool - for example it reads data only for one battery module. If you have more batteries and they are reported as separate modules, you will have to change application. We're using two Pylontech batteries connected by data link - and they are reported as one module in Victron.

I'm using it on off-grid system, so it doesn't look for grid consumption/production.  


## Resources

https://community.victronenergy.com/questions/155407/mqtt-local-via-mqtt-broker.html
