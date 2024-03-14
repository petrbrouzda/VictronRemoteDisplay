# VictronRemoteDisplay
Simple remote (wifi connected) status display for Victron photovoltaic systems (Cerbo GX) without cloud access - for environments without internet access.

Victron photovoltaic systems with Cerbo GX have display component (GX Touch 50, GX Touch 70) but they are connected to Cerbo by cable. And they are touch-enabled - you can change view, you can change system settings. 

I need wifi-connected display with only passive function (no configuration changes), and it run in no-internet environment (no connection to Victron VRM cloud).

So I've build this:
![](/imgs/2024-03-12%2019.46.18.jpg)

It runs basic [ESP32-C3 board](https://s.click.aliexpress.com/e/_DEw2w8v) with [cheap 1.8" SPI display](https://s.click.aliexpress.com/e/_DeaLqWd). Without soldering - by using dupont F-F cables:
![](/imgs/2024-03-06%2015.52.43.jpg)

Connection (display - ESP32-C3):
- CS - 7
- RST - 6
- DC - 5 
- SDA - 3
- SCL - 2
- BLT - 4
- VCC - 3V3
- GND - GND

Box is 3D printed, STL files are in [box-display18 folder](box-display18/).


## How it works

cerbo má MQTT
jak ho zapnout
je třeba keepalive

## How to run it

zapnout MQTT
do config.h doplnit wifi a IP brokeru
do config.h nastavit parametry systemu
pocita se s 20% rezervou baterek

## Limitations

není obecné, jen jedna baterka, jen moje díly

