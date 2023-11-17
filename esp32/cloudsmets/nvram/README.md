# CloudSMETS NVS Generation
Ref: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_partition_gen.html

ESP32's non-Volatile Storage (NVS) can be preloaded based on a CSV file and this is used by CloudSMETS as follows.

- Version 0 of CloudSMETS creates an initial NVS partition complete with the namespaces and keys expected
- Later versions of CloudSMETS will do start-of-day checking for namespaces/keys added after version 0 and will add them as required
- At any point after the start-of-day operations, CloudSMETS can assume that at least an empty default exists for all namespace/key values.

## Default NVS Space
The default available space is 16KB.  We should aim to allow at least twice the amount of space that is actually required.

> Question:/ How do we update certificates?  Can we do this remotely?  Suggest that OTA does this and we actually test this somehow.

## Configuration File
The NVS initialization file is `esp32/nvram/nvram.csv`.

## Expected Configuration
|Namespace|Key|Size|Example|
|-|-|-|-|
|certificates||||
||DGRG2|1kB|-----BEGIN CERTIFICATE-----...|
||MswRSARoot2017|2kB|-----BEGIN CERTIFICATE-----...|
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
||azIotHub|51|my-iothub|
||azDevice|33|CloudSMETS|
||azCnct1|196|HostName=myhub.iothub...|
||azCnct2|196|HostName=myhub.iothub...|

Total maximum data length is about 4kB bytes (including certificates)

## Certificates
Refs:
- https://learn.microsoft.com/en-us/azure/iot-hub/migrate-tls-certificate?tabs=portal
- https://learn.microsoft.com/en-us/azure/security/fundamentals/azure-ca-details?tabs=root-and-subordinate-cas-list


Two certificates are required by CloudSMETS
- A certificate for the CloudSMETS webserver
    - Not supported at this time.
- Two certificates to allow CloudSMETS to access Azure.
    - DigiCert Global Root G2 certificate (~ 1kB)
    - Microsoft RSA Root Certificate Authority 2017 (~ 2kB)

