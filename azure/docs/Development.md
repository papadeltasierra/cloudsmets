## MQTT Receiver
### Purpose
Receives energy usage information sent from your CloudSMETS device, over the internet to the Azure IotHub using the [MQTT] protocol.

### Design
- Written using Visual Studio in C#
- An Azure IotHub function

### Configuration
#### Application settings
|Name|Value|
|-|-|
|AzureWebjobsStorage|Either of the two `Connection string`s for your Storage Account|
|EventHubconnection|Either of the two `Connection string`s for your IoT Hub|

[MQTT]: https://mqtt.org/

#### Event Routing
Ref: https://learn.microsoft.com/en-us/azure/iot-hub/iot-hub-devguide-messages-d2c

If we don't route messages explcitly from the IoT Hub `CloudSMETS` device (the logical connection to your T-ZigBee board) to the `MQTTReceiver` Azure Function, then the `MQTTReceiver` could receive events from any device sending events to the IoT Hub's _Build-in endpoint_.

> As an example, I have a weather monitoring system that also uses my Iot Hub and initially I ended up with the `MQTTReceiver` receiving messages from my weather monitoring system and if my `CloudSMETS` device had been active, I would also have had weather monitoring system events being received by the `MQTTReceiver`.  Neither system would have been happy!


