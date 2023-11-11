# Phased Development
## Requirements
- Complete application can be upgraded using hot-upgrade procedures via WiFi and the [ESP32-C3]
- [TLSR8258] is also updated by the [ESP32-C3] downloading a new image and using this to update the [TLSR8258] (might need to "update as we download)
- All initial configuration is via a ad-hoc WiFi connection to the [ESP32-C3]
- Configuration will normally be via regular WiFi with a server exposed
- If regular WiFi fails, the [ESP32-C3] will retry but also fallback to ad-hoc in case, for example, the regular WiFi password needs to be changed
- Data will be relayed from [SMETS2] network to [Azure] (Microsoft Cloud) (and later [GCP] (Google Cloud Platform) or [AWS] (Amazon web Services)) in a defined JSON format
- Upgrades are all OTA (over the air)
    - There has to be some sort of recovery process, and at least "OTA first, before any new code exercised"
- Debugging is available, ideally via WiFi and not requiring serial connection etc
    - Might be to a local server, perhaps, or to [Azure] etc
    - Need to write a simple local server (Python) if we are going to do that
        - Will want to seiralize the data (JSON) and then produce sensible debuggable output (CSV files?)

## Security
- ZigBee security will be via the combination of MAC address and password (?) and the closed nature of the [SMETS2] (Smart Energy) system
- The HTTP webserver will have a username and password
- The HTTP webserver will, by default, be locked down to "same subnet"
  - Will this be enough?  Would a router pinholed connection appear to be from the router and not a remote endpoint?
  - Subnet lockdown can be disabled
- If HTTPS and a certificate can be used to secure the HTTP webserver, it will be but this might not be possible.

## Design and Development
> How can we break the development up into sensible phases?  Imagine that we had to "demo" the development.

### ESP32-C3 Configuration
1. Design configuration storage
    - JSON based storage, limited size
    - Defaulting required and reset to factory defaults
1. Implement reset to factory settings (triggerd from web page)
1. Implement reset to factory settings triggered by reset button

### ESP32-C3 WiFi
1. Implement ad-hoc WiFi connectivity and HTTP webserver function to configure initial WiFi settings.
1. Implement full WiFI function with fallback to ad-hoc if required
    - What can we show about the WiFi connection?
        - Connection status
        - Bytes exchanged?  Really cloud stats!
        - Last send message?  Really cloud stats!
1. Implement HTTP function to show current software version level ([ESP32-C3] only)

### GitHub Actions CI
1. Create a Dockerfile and Docker image that can compile the ESP32-C3 code locally and on GitHub as per [GitHub docker container actions]
    1. Where do the build compilers, libraries etc come from?
    1. Need to ensure that all tools are "locked down" and do not change under out feet (specific commit or tag/version)
1. Create a Dockerfile and Docker image that can _UT (Unit Test)_ the ESP32-C3 code locally and on GitHub using the ESP32 [Unit testing on Linux] environment
1. Create a Dockerfile and docker image that can compile for the [TLSR8258] using the [TLSR825x SDK] locally and on GitHub
1. Create a Dockerfile and Docker image that can _UT_ the [TLSR8258] if possible (and if required)
1. Set-up GitHub based builds

### Local Data Lake
1. Implement a simple local server to receive data "sort of like Azure"
    - Support receipt of tracing and debugging information
    - Support receipt of SMETS2 type data
    - Support logging to output files
    - Support configurable display to screen too.

### Debugging
1. Design tracing/debug and allow for multiple instances (configuration) in case one site is taking data from many locations
1. Implement debugging (via serial) for both [ESP32-C3] and [TLSR8258], ideally both via serial port and some sort of logging to WiFi
1. Create a tool that can hash the file ID to minimise debug logs.
    - Maybe hash the following into 4 hex digits?
        - Filename (just the final part of perhaps the relative path too)
        - An extra field in case of hash clashes (defaults to 0x0000)
    - Update only those files that have no `__FILEID__` field defined
    - Write a (new) mapping file to match the commit
    - Have second tool to validate all files have a `__FILEID__` during builds

### Image Integrity
1. Implement tag-based build system that creates and publishes images with the build ID/tag made part of the build image
1. Implement signing of the images

### OTA
1. Implement OTA upgrade of the [ESP32-C3]
1. Implement OTA based upgrade of the [TLSR8258] for the same reason
    1. Note that and upgrade of the [TLSR8258] might not require an upgrade of the [ESP32-C3] and vice-versa

### Unit Testing
1. ESP32-C3: Design some sort of testing of as much of the code as possible
    -  Design isolated to allow for testing wherever possible
2. TLSR825x: Ditto if at all possible (or sensible)

### ZigBee Configuration
1. Implement [ESP32-C3] configuration of the [TLSR8258]
    - Show GUID (IEEE address)
    - Configure IHD (In-house display) key

### ZigBee Pairing
1. Implement the "pair to SMETS2 system" control in the HTTP webserver
    - What other info can we show?
        - Can we show the remote key?
        - Can we show pairing info?
        - Can we show connection status?
        - Can we show bytes exchanged?
        - Can we show last data received (maybe the date, or at least the time?)

### ZigBee SMETS2 Support
1. Implement the "sending data" functionality based on received [SMETS2] data
    - Refer to the ??? profile to see what we might be receiving
    - Make the sending format known and common to (later) [Azure], [GC] and [AWS] data targets
    - Implement status via HTTP webserver
        - Connection status
        - Bytes/messages exchanged
        - Last message?
1. Implement the [SMETS2] ??? profile to receive information from the SMET2 network

### Azure Configuration
1. Implement the HTTP webserver "configure [Azure]" functionality
1. Implement the connection to [Azure] configuration and connectivity

### Webserver Security
1. Implement HTTP webserver security as far as possible
    - Username and password
    - HTTPS and certificates

### Azure-as-a-product
1. Design and implement the Azure functionality as an Azure product
    - Need a database to hold the data, both SMETS and debugging/tracing
    - Need a web interface to view the data
    - Need some good queries/filtered to show the data
    - Can we allow extensions via Kusto and save the queries somehow?
        - This is a "nice to have" for much, MUCH later

### [GCP] (Google Cloud Platform) Support
1. Extend to [GCP] (Google Cloud Platform)

### [AWS] (Amazon Web Services Support
1. Extend to [AWS] (Amazon Web Services)

[ESP32-C3]: https://www.espressif.com/sites/default/files/documentation/ESP32-C3_datasheet_en.pdf
[TLSR8258]: http://wiki.telink-semi.cn/doc/ds/PB_TLSR8258-E_Product%20Brief%20for%20Telink%20BLE%20IEEE802.15.4%20Multi-Standard%20Wireless%20SoC%20TLSR8258.pdf
[TLSR825x SDK]: http://wiki.telink-semi.cn/tools_and_sdk/Zigbee/Zigbee_SDK.zip

[SMETS2]: https://zigbeealliance.org/wp-content/uploads/2019/11/docs-07-5356-19-0zse-zigbee-smart-energy-profile-specification.pdf

[GitHub docker container actions]: https://docs.github.com/en/actions/creating-actions/creating-a-docker-container-action
[Unit Testing on Linux]: https://docs.espressif.com/projects/esp-idf/en/release-v4.4/esp32/api-guides/linux-host-testing.html


[Azure]: https://azure.microsoft.com/en-gb/free
[GCP]: https://cloud.google.com/gcp?hl=en
[AWS]: https://aws.amazon.com/
