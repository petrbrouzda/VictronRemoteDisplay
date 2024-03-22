# VictronRemoteDisplay
Simple remote (wifi connected) status display for Victron photovoltaic system (with Cerbo GX central system) without cloud access - for environments without internet.

Victron photovoltaic systems with Cerbo GX has display module (GX Touch 50, GX Touch 70) but they are connected to Cerbo by cable. And they are touch-enabled - you can change view, you can change system settings. 

I need wifi-connected display with **only** passive function (no configuration changes), and it run in no-internet environment (no connection to Victron VRM cloud).

So I've build this:
![](/imgs/2024-03-12%2019.46.18.jpg)

It runs basic [ESP32-C3 board](https://s.click.aliexpress.com/e/_DEw2w8v) with [cheap 1.8" SPI display](https://s.click.aliexpress.com/e/_DeaLqWd). Without soldering - by using dupont F-F cables:
![](/imgs/2024-03-06%2015.52.43.jpg)

Connection (display pin - ESP32 pin):
- CS - GPIO 7
- RST - GPIO 6
- DC - GPIO 5 
- SDA - GPIO 3
- SCL - GPIO 2
- BKL - GPIO 4
- VCC - 3V3
- GND - GND

Box is 3D printed, STL files are in [box-display18](box-display18/) folder.


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

1) If your Cerbo is not connected to WiFi: switch on WiFi access point in Cerbo GX.

2) In Cerbo's console, go to **Settings - Services** and switch on **MQTT on Lan (SSL)**. Then another option will appear: **MQTT on Lan (plaintext)**. Switch it on also.

3) Edit **config.h** section with your photovoltaic system limits:

```
#define SOLAR_PANELS_POWER_WP (6.0*450.0)
#define INVERTER_POWER_W 2400.0
#define BATTERY_CAPACITY_KWH 3.5
#define INVERTER_OWN_CONSUMPTION 20.0
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

6) Used libraries:
* PubSubClient 2.8 
* Tasker 2.0.3
* ArduinoJson 6.21.4 
* Adafruit ST7735 and ST7789 Library 1.10.0
* Adafruit GFX Library 1.11.8
* Adafruit BusIO  1.14.4 


7) Build application, send it into ESP and enjoy.

## What is displayed?

![](/imgs/6c22871d-8867-4ad8-91c0-da45876fa43c.jpg)

**First bar** is **battery status.** 
* Battery icon shows if battery is charged or discharged. 
* Percentage is shown as "usable capacity". Battery will cut off at 20 %, so you've got technical reading 20 % - 100 %, which will be displayed as 0 % - 100 % here.
* First data item (-2264 W) is power consumed (negative) or charging (positive). 
* Then estimated energy available (2.6 kWh) and estimated time battery will be able to suply current consumption.

**Second bar** is status of **solar panels**. 753 W in bar is current power supplied by panels. 
* First data row contains: 110 V = panels voltage; 0.5 kWh = today's yield; 1374 W = today maximum power supplied
* Second data row: yield yesterday / two days back / three days back

**Third bar** is **consumption**. 2997 W consumed at the moment.

Last line are LEDs on inverter, so:
* Low Battery (correct - there is only one battery, which is unsupported configuration for 3 KW consumption)
* Overload (correct, inverter is rated for constant output of 2400 W)
* inv = "Inverter on"
* Last field is number of messages processed per minute (as debug information - if connection is somewhat broken, there will be 0/m).


## Application limitations

Application has been tested on only one photovoltaic system - mine. 

It is not generic / universal tool - for example it reads data only for one battery module. If you have more batteries, you will have to change application.

I'm using it on off-grid system, so it doesn't look for grid consumption/production. 


## Resources

https://community.victronenergy.com/questions/155407/mqtt-local-via-mqtt-broker.html
