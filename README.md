# VictronRemoteDisplay
Simple remote (wifi connected) status display for Victron photovoltaic system with Cerbo GX central system without cloud access - for environments without internet connection.

Victron photovoltaic systems with Cerbo GX has display module (GX Touch 50, GX Touch 70) but they are connected to Cerbo by cable. And they are touch-enabled - you can change view, you can change system settings. 

I need wifi-connected display with **only** passive function (no configuration changes), and it run in no-internet environment (no connection to Victron VRM cloud).

So I've build this 3.5" display - as described in [VictronDisplay35](VictronDisplay35/README.md)  directory:
![](/imgs2/2025-05-05%2018.51.06.jpg) 

Previous 1.8" display version is documented in [VictronDisplay18](VictronDisplay18/README.md)  directory:
![](/imgs/2024-03-12%2019.46.18.jpg)



## How it works

Cerbo GX offers WiFi access point. And Cerbo GX has MQTT broker built in - you just have to switch it on in Settings. 

ESP32 can connect to Cerbo's WiFi, then connect to MQTT broker and get all information about your photovoltaic system. Seems easy.

But when you connect to Cerbo's MQTT and subscribe to all topics ("#"), you've got only one topic with ID of Cerbo's system:

![](/imgs/empty.png)

You have to read the system ID and publish **keeepalive message**: empty message to topic `R/<system ID>/keepalive` - for example `R/c0619ab33b2f/keepalive`

After you send the message, MQTT will magically starts filling with data:
![](/imgs/full.png)

But your happiness will last only for one minute, then Cerbo stops publishing more data until you send another keepalive message. So you have to send keepalive message periodically, every 55 seconds.


## Limitations

Application has been tested on three photovoltaic systems only.

It is not generic / universal tool - for example it reads data only for one battery module. If you have more batteries and they are reported as separate modules, you will have to change application. We're using two Pylontech batteries connected by data link - and they are reported as one module in Victron.

I'm using it on off-grid system, so it doesn't look for grid consumption/production. 


## Resources

https://community.victronenergy.com/questions/155407/mqtt-local-via-mqtt-broker.html
