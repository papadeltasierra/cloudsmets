# cloudsmets
Cloud enabling getting SMETS Smart Meter to cloud servers.

## Overview
A system which listens in on communications between your Smart meter hub and your
IHD (in home device, that little "how much electricity and gas am I using" display)
and uses this to sent the same information to a **your** cloud based server.

The cloud based service could be a Microsoft [Azure], Amazon [AWS] (Amazon Web Services)
or [Google Cloud] based server.

## Technology
- The system uses a development board that contains an ESP32-C3 (WiFi) processor
  and an TLS??? (ZigBee) processor
- Listens to the ZigBee communications between the Smart meter hub and IHD
- Uses ZigBee security information learnt from your IHD to decode the encrypted 
  signals
- Uses WiFi to create a secure TLS connection to your cloud server
- Uploads your data using the [MQTT] protocol
- Allows configuration via a website initially acting as a WiFi server to which
  you can connect, for example, your mobile phone and later as a website 
  accessible via your regular WiFi router
- Using OTA (over the air) updates to push updated releases such as adding new
  TLS certificates if [Azure] changes its certificates
- The ZigBee board is updated via the ESP8265 board using similar OTA techniques
- OTA images are signed for security
- Configuration is stored on the ESP32's non-volatile storage.

## Security
- Your data is received over a secure, encrypted, ZigBee link (the same one that your smart
  meters normally use)
- Your data is sent to the cloud servers using a secure TLS link
- The OTA images are signed and downloaded over a secure TLS link
- Network time is determined using UK based NTP server; this is the same process
  that your PC uses to keep time updated
- Ultimately you either trust me to produce secure images, or you can disable
  updates, at least until you MUST update to take a new cloud server certificate
  for example.

> If you are really paranoid, using Wireshark or similar to see what connections
  the _cloudsmets_ makes and if you see more than these, let me know because you
  shuold not!

## Configuration
Configuration breaks down into two parts, the WiFi/cloud portion and the ZigBee
portion.

### WiFi/Cloud
The _cloudsmets_ device has a simple webserver that is used to configure the
device.  The webserver is exposed either:

- https://cloudsmets if your wiFi router can resolve the name
- https://an-ip-address where you will need to determine the IP address by looking
  at your routers webpages to see what IP address it assigned to the _cloudsmets_ device
- http://192.168.0.1 via a local WiFi server called "cloudsmets" that you can
  connect you phone (say) so if a WiFi router cannot be reached for whatever
  reason.

The website is very basic and self-explanatory.

### ZigBee
There are two bits of ZigBee configuration required to sniff (spy on) the IHDs
exchanges with the hub:

1.  The _GUID_ (globally unique ID) of the Hub
2.  The _security key_ used by the IHD when communicating to the hub.

The _GUID_ is configured via the devices website as for the WiFi/Cloud configuration
but the _security key_ has to be learnt from the IHD.

#### Where is my GUID?
- Is it on a label on your hub?
- Does your hub show it on start-up?
- Is it in the hubs settings?
- OK, I give up; try a web search for "GUID" and the make and model of your hub.

#### Learning the Security Key
Your hub does not transmit its _security key_ in normal usage; the only time it does
is when pairing to a ZigBee hub which means we need to make it try and do this.
The process we use is as follows:

1. Place the _cloudsmets_ device into _ZigBee hub_ mode via the website
2. Turn off your hub
3. Take the _cloudsmets_ device and the hub well away from your smart meters;
   the hub must be so far away that it cannot connect to your smart meters' hub
4. Turn on the hub; it should fail to reach the smart meter hub, find the _cloudsmets_
   hub, try to connect to it, revealing its _security key_ and then fail because
   the _cloudsmets_ hub is deliberately coded to reject it.

Note that for a shot while, the _cloudsmets_ hub can be powered from an external
USB power cell or an atchaed (and charged) LiPo connected directly to the board.  This
alows oyu to carry it away from the smart meters.

[Azure]: https://azure.microsoft.com/en-us/get-started/azure-portal
[AWS]: https://aws.amazon.com/
[Google Cloud]: https://cloud.google.com/
[MQTT]: https://mqtt.org
[ESP32-C3]: https://esp32.org
[TLS2323]: https://tls?????
[LiPo]: https://en.wikipedia.org/wiki/Lithium_polymer_battery