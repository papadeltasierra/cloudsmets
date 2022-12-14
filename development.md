# Development Notes

## Phase 0
- Read the ZigBee2MQTT code and see if it could be reused and/or gives hints as
  to design
  - Do not want any "MQTT to ZigBee" code
  - Do need the "CLI" between ZigBee and MQTT
  - Can we update ZigBee from the ESP32?
  - Need to configure the cloud settings
  - Need to configure the cloud MQTT topic(s)
- See if can _tickle_ ths IHD to reveal any secrets

## Phase 1
- Image builds signed and public/private keys
- Non-volatile storage for WiFi details etc
- Simple HTTP server to configure WiFi etc
  - WiFi hub if no details else local
- OTA updates for ESP32 working (from Github pages?)
  - Multiple keys to allow rollover?

## Phase 2
- Define CLI to/from ZigBee from/to ESP32
- Updating ZigBee processor via OTA

## Phase 3
- Connecting to (Azure) cloud
  - Done in cloud agnostic manner to allow later addition of AWS/Google
    support.
- Example MQTT actions
  - Timestamp
  - Electrcity, list of tarrifs
  - Gas, list of tarrifs
- ZigBee creating fake data using simple time driven script?  

## Phase 4
- Learning IHD security key
- Returning to ESP32 for storage
- Showing on website (read-only)

## Phase 5
- Sniffing ZigBee
- Filtering as much as possible in hardware
- Decoding and passing the ESP32
- EPS32 relays