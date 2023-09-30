# SMETS Emulation Test System

- ESME, a ZigBee coordinator emulating the eletricity supply meter
- Gate-proxy, a ZigBee endpoint emulating the gas proxy
- CloudSMETS, the CAD device.

## ESME
- Will expose Smart Energy information
    - Simple procing attributes
    - Simple usage attributes

> Presumably there is something that says "electricity" as opposed to "gas"?

## Gas proxy
- Same as ESME except using gas information

## CloudsSMETS
Must be expected to:
- Find the coordinator
- Find the ESME and Gas proxy as devices exposing the Smart Meter cluster
    - What about the time cluster?
        - Find this separately?
- Read the time from the ESME
- Read the Smart Meter attributes from the ESME and Gas proxy.

## Python Tool
- Maybe then write something to run this, or perhaps now time to write first ESP32-C3 application?
