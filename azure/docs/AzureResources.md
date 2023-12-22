# Azure Resources
CloudSMETS uses a number of Azure resources, some provided by CloudSMETS and shared across all users and some provided by you, the owner to the CLoudSMETS device.  This document explains what they do and how they are used.

## Security
### Shared Resources
Your CloudSMETS device will ask the shared CloudSMETS resources for two things:
- The version of the latest CloudSMETS version
- Downloads of the latest version of the [ESP32c3] and [tlsr8258] firmwares.

Both of these queries take place over TLS, secured by the normal certificate processes.

Both the firmware images are also crytogrphically signed to ensure that your CLoudSMETS will not try to run software from an unofficial source.

## CloudSMETS Resources
CloudSMETS provides two sets of resources which are shared by all users:
- An Azure function that provides an HTTPS endpoint that returns the version of the latest CloudSMETS release as a simple string.
- A web server hosting the CloudSMETS images for both the [ESP32c3] and the [tlsr8258].

## Your Resources
Your resources belong to you and are required for you to receive the data from your CloudSMETs device.  Once the data is in Azure, you can do whatever you like with it but the following resources are required at a minimum.
- An Azure account; you can get one for free, with about £150.00 free credit per month which should be more than enough for your CloudSMETS usage
- An Azure IoT Hub; you are only permitted one per subscription so you should reuse any existing IoT Hub or create a new one if none exists
- An Azure hosted NoSQL database server; your data will be written here
- An Azure _MQTT receiver_ that receives data from CloudSMETS and places it into the NOSQL server database
- Sample Azure functions that provide a simple fron-tend website that allows you to view your data.

### Azure Subscription
- Your _account_ with Microsoft Azure.

### Azure Resource Group
- The top-level resource in your `Subscription`

### IoTHub
- Created inside your `Resource Group`

### (Optional) Event Hubs Namespace
- Created inside your `Resource Group`

### (Optional) Event Hub
- Created inside your `Event Hub Namespace`

### (Optional) Custom Endpoint & Rule
- Created inside your `Iot Hub`
- References the Event Hub

### NOSQL Database server
- Created inside your `Resource Group`

### MQTTReceiver Azure Function
- References either the custom or default Event Hub
> How do we ship either of these?



### Securing Your Resources!
Securing your resources is your responsibility and CloudSMETS requires just one input into your Azure resources, the receipt of the MQTT data to the Azure _MQTT receiver_.  This access is granted by you configuring the `Acccess Keys` into the MQTT configuration page of your CloudSMETS.

> Neither I, nor anyone else, can see your `Access Keys` and CloudSMETS doesn't send your data anywhere other than too you Azure _MQTT receiver_.  And once the data has reached your NOSQL server, only you can see it unless you choose tolet others do so.