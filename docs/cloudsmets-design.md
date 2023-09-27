# Design Overview

- [tlsr8258] acts as a "relay" whilst all control resides with the [ESP32-C3]
- The ESP32-C3 controls the operation of the tlsr8258 using the [Telink] [zbhci] protocol over the internal [T-ZigBee] UART/serial link between the two chips
- A peer-to-peer WiFi network is initially created with SSID named `CloudSMETS-<MACaddress>`
    - The MAC address is used in case someone tries to set up multiple CloudSMETS devices at once
- A webserver is avaiable on the WiFi network on port 80 (1)
- ZigBee connection is not established until the secret ?? is configured
- The ZigBee secret (??) is passed from [ESP32-C3] to the [tlsr8258] over the [zbhci] (?? API call ??)
- A WiFi connection is established once the WiFi information has been configured
- A web server will continue to be available on port 80 (1) but at the IP address assigned to CloudSMETS after the WiFI connection has been established (2)
- A connection to [Azure] will be attempted if WiFi is connected and the Azure connection configuration has been configured
- CloudSMETS configuration information is stored in the [ESP32-C3] non-volatile storage; this includes the ZigBee secret (??)
- Time for CloudSMETS will always be learnt from the ZigBee network and will always reflect [UTC]
- As soon as a ZigBee connection is established, the [ESME]/[gas proxy] will be queried for information as described below
- ZigBee attribute information is always obtained as follows:
    - The [ESP32-C3] issues a request, driven by a timer, over the serial/UART connection using the [Telink] [zbhci] protocol
    - The [tlsr8258] passes the request out to the Zigbee network
    - The ZigBee network responds and the [tlsr8258] relays the information to the [ESP32-C3] over the [zbhci]
    - A _time_ response is used to set the local time of the [ESP32-C3] and that ends processing
    - A _Smart Energy_ response is used to generate an [MQTT] request which is then forwarded over WiFi to [Azure]; if there is no existing [Azure] connection, one is attempted and if none can be estabished, the [MQTT] request is just dropped
- The [tlsr8258] uses its two LEDs (red and green) as follows:

|Red|Green|Meaning|
|-|-|-|
|Off|Off|Configuration required|
|On|Flashing|Configured and attempting to contact the ZigBee network|
|Flashing|Off|Configured but unable to contact the ZigBee network|
|On|On|Configured and successfully connected to the ZigBee network|

- The [ESP32-C3] uses it's blue LED as follows:

|Blue|Meaning|
|-|-|
|Off|Starting, no web server available.|
|Rapid flashing<br/>on-1/2s-off-1/2s...|Peer-to-peer WiFi network created and web server ready but insufficient configuration.|
|Slow flashing<br/>on-2s-off-2s...|Configured but WiFi or [Azure] connection not established.|
|On|Configured and WiFi and [Azure] connections established.|

## Reset/Switches
- The [T-ZigBee] `Reset` button will _reboot_ the CloudSMETS but not affect any configuration; this is the same as unplugging and then plugging back in
- The [T-ZigBee] `Key`, which is connected to the [ESP32-C3], can be used as follows:

|Key press|Action|
|-|-|
|Press for 1s|Reset Wifi and start peer-to-peer network.<br/>This does not remove the Azure or ZigBee configuration and is intended for if you change your WiFi network and then need to change the CloudSMETS WiFi configuration.|
|Press for 5s|Total reset to shipping state; all configuration wiped.|


## Notes
1. If possible, a secure (`https://..`) webserver will be coded later but for now the web servers are all unsecure (`http://...`).
2. For now [DHCP] will be assumed and CloudSMETS will have an IP assigned by the router; this can be hard-coded via a router's ?? table.

[ESP32-C3]: http://somewhere
[tlsr8258]: http://somewhere
[Telink]: http://somewhere
[zbhci]: http://somewhere
[Azure]: http://somewhere
[MQTT]: https://somewhere
[ESME]: https:/somewhere
[Gas proxy]: https://somewhere
[T-ZigBee]: https://somewhere
[UTC]: https://somewhere