# WebPages Design
## Page Layout
```mermaid
---
title: Relative Webpage Layout
---
stateDiagram-v2
Index/Home --> Cloud
Index/Home --> Debug
Index/Home --> Info
Index/Home --> WiFi
Index/Home --> ZigBee
Cloud --> Azure
Cloud --> GCP
Cloud --> AWS
```
- All pages have a link back to the parent
- All pages have a link back to `Home`
- All configuration pages have a `Save` and `Cancel` button
    - `Save` will save changes then reenter the page, now displaying the new information
    - `Cancel` will reenter the page, displaying again the current (unchanged) configuration
- Information only pages only have the `Home` and `Parent` links

## CSS (Cascading Style Sheets)
- Pretty basic styling
- Must work for mobiles
- Should also work for PCs
    - Can we detect or should we have two sets of selectable styles?

~~~
Where should the OTA configuration be?
~~~


## Page Design
### / (Index)
||||
|-|-|-|
||WiFi configuration||
||Cloud(s) configuration||
||ZigBee configuration||
||Information||
||Debugging||
||Factory Reset||
||||
### /cloud
||||
|-|-|-|
||Azure||
||Google Cloud Platform||
||Amazon web Services||
||Local Debugging||
|-| |-|
||Home||
||||
### /cloud/azure
```mermaid
flowchart LR
    id1([WiFi configuration])
    id2(["Cloud(s) configuration"])
    id3([ZigBee configuration])
    id4([Information])
    id5([Debugging])
    id6([Factory Reset])
```
### /cloud/gcp
```mermaid
flowchart LR
    id1([WiFi configuration])
    id2(["Cloud(s) configuration"])
    id3([ZigBee configuration])
    id4([Information])
    id5([Debugging])
    id6([Factory Reset])
```
### /cloud/aws
```mermaid
flowchart LR
    id1([WiFi configuration])
    id2(["Cloud(s) configuration"])
    id3([ZigBee configuration])
    id4([Information])
    id5([Debugging])
    id6([Factory Reset])
```
### /debug
```mermaid
flowchart LR
    id1([WiFi configuration])
    id2(["Cloud(s) configuration"])
    id3([ZigBee configuration])
    id4([Information])
    id5([Debugging])
    id6([Factory Reset])
```
### /info
```mermaid
flowchart LR
    id1([WiFi configuration])
    id2(["Cloud(s) configuration"])
    id3([ZigBee configuration])
    id4([Information])
    id5([Debugging])
    id6([Factory Reset])
```
### /reset
```mermaid
flowchart LR
    id1([WiFi configuration])
    id2(["Cloud(s) configuration"])
    id3([ZigBee configuration])
    id4([Information])
    id5([Debugging])
    id6([Factory Reset])
```
### /wifi
||||
|-|-|-|
||SSID:||
||Password:||
|-||-|
|Save||Cancel|
|-||-|
|Home||Back|
||||

### /zigbee
```mermaid
flowchart LR
    id1([WiFi configuration])
    id2(["Cloud(s) configuration"])
    id3([ZigBee configuration])
    id4([Information])
    id5([Debugging])
    id6([Factory Reset])
```
