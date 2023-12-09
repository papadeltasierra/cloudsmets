# Test Plan
## Overview
- Test web pages
- Test switching to/from STA mode
- Test OTA
- Test Azure connectivity
- Test reset-to-factory
- Test ZigBee comms
- Status
- Flasher

## Web Pages
- All pages log in any order
- All pages policy field length and syntax
- All pages with submit work
- All pages work from SoftAP and from STA
- All changed settings take affect.

## Switching to/from STA
- Start-of-day, SoftAP only works
- Configure STA correctly, STA comes up too - dual mode
- Remove STA configuration, STA goes down - SoftAP only mode
- Changing AP config works.

## OTA
- Simple OTA to latest works
- OTA to developer works
- OTA to specific version works
- OTA is backed-off if cable pulled before "OTA confirm" period.

## Azure
- Configuration works
- Azure connects etc
- Azure refreshes token and does not "drop off"
- Azure reflects status.

## Reset-to-factory
- Confirm "quick flick" has no affect
- Test and confirm all config removed.

## ZigBee Comms
- Confirm can show keys
- Confirm can reset keys
- Cnofirm seeing all expected exchanges, logging etc

## Status
- Confirm that shatus shows all status correctly at all times

## Flasher
- Confirm flasher works correctly at all times