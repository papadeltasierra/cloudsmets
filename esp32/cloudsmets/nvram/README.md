# CloudSMETS NVS Generation
Ref: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_partition_gen.html

ESP32's non-Volatile Storage (NVS) can be preloaded based on a CSV file and this is used by CloudSMETS as follows.

- Version 0 of CloudSMETS creates an initial NVS partition complete with the namespaces and keys expected
- Later versions of CloudSMETS will do start-of-day checking for namespaces/keys added after version 0 and will add them as required
- At any point after the start-of-day operations, CloudSMETS can assume that at least an empty default exists for all namespace/key values.

## Configuration File
The NVS initialization file is `esp32/nvram/nvram./csv`.

## Expected Configuration
|Namespace|Key|Size|Example|
|-|-|-|-|
|softap||||
||softApSsid|33|CloudSMETS|
||softApPwd|64|CloudSMETS|
|wifi||||
||wifiSsid||CloudSMETS|
||wifiPwd||CloudSMETS|
|web||||
||webUser|32|admin|
||webPwd|32|CloudSMETS|
|debug||||
||dbgFunc|1|0 (disabled)|
||dbgBaud|4|1000000|
||dbgPort|2|9000|
||dbgEsp32c3|1|0 (none)|
||dbgTlsr8258|1|0 (none)|
|ota||||
||otaFunc|1|1|
||otaUrl|128|https://cloudsmets.blob.core.windows.net/cloudsmets|
||otaRel|32|1.2.3-dev4 (usually blank)|
|azure||||
||azFunc||0 (disabled)|
||azIotHub|51|string|my-iothub|
||azDevice|33|string|CloudSMETS|
||azCnct1|196|string|HostName=myhub.iothub...|
||azCnct2|196|string|HostName=myhub.iothub...|

Total maximum data length is about 800 bytes

## Certificates
Two certificates are required by CloudSMETS
- A certificate for the CloudSMETS webserver
- A certificate to allow CloudSMETS to access Azure.

??? What sizes are the rewquired certificates?

Ref: https://learn.microsoft.com/en-us/azure/security/fundamentals/azure-ca-details?tabs=root-and-subordinate-cas-list

> Additional certificates would be required for AWS and GCP when supported.
