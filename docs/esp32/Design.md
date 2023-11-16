# ESP32 Design

## Questions
- Can we list available WiFi networks and also have hidden network option?
- Can we make the passwork optionally visible (function also required elsewhere?)
- Can the WiFi server be secured?

## Configuration
### WiFi
- SSID
- Password
- Server/device name

### Webserver
- Admin name and password?

## Webserver Pages
- Home
- Status Page
  - WiFi status
    - Direct or ??
    - IP address
  - Azure
    - Connected
  - ZigBee
    - Connected
    - EU164
    - PANID
    - Channel?
    - Link keys?
- WiFi
  - SSID, password, device name
  - Show IP address (maybe on device info page?)
- Azure
  - Connection string
  - Device name
  - Enabled?

## Blue LED Status
|Action|State|
|-|-|
|Off|WiFi Direct, nothing connected|
|Slow flash|WiFi connected|
|Fast flash|WiFi and Azure connected|
|Stutter|WiFi and ZigBee connected|
|On|WiFi, Azure, ZigBee all connected|

## Key Press
|Press|Action|
|-|-|
|Quick|Switch to WiFi Direct|
|Long (>5s)|Factory reset|

## Debugging
- Uses ESP macros e.g. ESP_LOGV()
- If enabled, exposes a web interface to avoid having to connect using serial port

## Overview
- Starts with WiFi Direct (peer-to-peer) network
    - WiFi server is exposed on port 80
- Configuration allows join to customer WiFi network
    - SSID and password required
    - Webserver now available via IP address/name of Cloudsmets
- Configuration of Azure connection via webserver
- ZigBee
    - Connect to ZigBee should be automatic
    - ZigBee should provide time signal
    - Need serial/zbhci/library
    - Find endpoints to query for gas/electricity
    - Request reports for gas/electricity usage and pricing
    - Timers to make explicit requests if not received information
    - Pass information for transmission to Azure (cloud)
- Ability to reset to factory with BIG warnings
    - Ideally via website
    - By long hold of key
- Short hold of key to revert to WiFi Direct in case lose connection
- Use blue LED to signal things (but what?)