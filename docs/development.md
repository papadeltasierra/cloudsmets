# Development Notes

## References
- [ZigBee2MQTT]
- [ZigBee2MQTT for development board]

## Understand the TLSR8258 SDK
> Done
- Read the ZigBee2MQTT code and see if it could be reused and/or gives hints as
  to design
- Do not want any "MQTT to ZigBee" code
- Do need the "CLI" between ZigBee and MQTT
- Can we update ZigBee from the ESP32?
- Need to configure the cloud settings
- Need to configure the cloud MQTT topic(s)
- See if can _tickle_ this IDE to reveal any secrets

> It looks like we can get all we want from the Telink SDK as it exposed sufficient code to allow us to customize it.

## Documentation and Design
> Done
- How will we debug this?
- What ESP32-C3 components do we need?
- What TLSR8252 components do we need?
- What can we achieve?
- What web pages do we need?
- What else can we do?

## Create Git repo for TLSR8258 SDK
> Done
- Store and tag the original code
- Create an initial branch
- Update `.gitignore` to allow existing libraries and other binary files in the repo from the SDK.

## Install/Test Telink IDE
- Install it
- Do some test builds
- Update `.gitignore` to ignore all new output files
- Create a Zigbee coordinator (this is what we need first)

## Python zhci Tool?
- If we want to drive the TLSR8258 directly, can we do this?
- Does the IDE come with a pythion tool already?
- Can we create one?
- Some simple testing over the API.

## ESP32-C3 Development

### Basics
- Create configuration component
  - Non-volatile storage for WiFi details etc
- Create web component
  - WiFi hub if no details else local
  - Maybe force local hub on key press?
- Create OTA component
- Compile and get working
  - Image builds must be signed and public/private keys
  - Builds must check tagged image against the tag somehow
  - What properties can we give the image?
- Test OTA
  - OTA updates for ESP32 working (from Github pages?)
  - Multiple keys to allow rollover?

## Zigbee
### zhci Control
- What zhci commands do we need to support?  Steal the code from the Zigbee2MQTT project
- Add extended zhci command to enable debugging
  - Need to have code to configure the UART ready for debugging
- Create a logging library if one does not exist
- Test logging output appears on the UART

### End-Point/Coordinator/Passive
- Wrap the existing End Point and Coordinator code
- Add new passive support
  - Never send anything out!
- Add new extendedd zhci to enable this

### Learning IHD Security Key
- Learning IHD security key
- Returning to ESP32 for storage
- Showing on website (read-only)

### Learning the Hub Key (??)
- Spoofing the IHD
- Connecting to the Hub
- Capturing the connectivity key and storing

## Cloud Connectivity
### Microsoft Azure
- Connecting to (Azure) cloud
  - Done in cloud agnostic manner to allow later addition of AWS/Google
    support.
- Example MQTT actions
  - Timestamp
  - Electrcity, list of tarrifs
  - Gas, list of tarrifs

### Amazon Web Services (AWS)
- All as above

### Google Cloud
- All as above

[ZigBee2MQTT]: https://www.zigbee2mqtt.io/
[ZigBee2MQTT for development board]: https://github.com/Xinyuan-LilyGO/T-ZigBee