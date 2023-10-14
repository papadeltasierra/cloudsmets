# ZigBee Overview
## Glossary
Documentation can be found at the [ZigBee Alliance] website.

|Acronymn|Expanded|Meaning|
|-|-|-|
|[BDB]|Base Device Behaviour|
|[ZDO]|ZigBee Device Objects|
|[ZCL]|ZigBee Cluster Library|
|Node|Single IEEE address|1+ logical devices|
|[NLME]|Network Layer Management Entity|
|[MLME]|Medium Access Control (MAC) sub-layer Management Entity<br/>Ref: [ZigBee RF4CE Specification]||

Other [acronymns and abbreviations] can be found at URL.

## [BDB Attributes]
There are a number of local attributes defined by the BDB specification.  some of these might be worth exposing in `CloudSMETS`.

|Attribute|Notes|Expose?|
|-|-|-|
|bdbCommissioningGroupID|unicast or groupcast|No|
|bdbCommissioningMode|e.g. Finding & Binding|No|
|bdbCommissioningStatus|Commissioning (joining) status<br/>Commissioning is the process of trying to join a ZigBee network|Yes|
|bdbJoiningNodeEui64|aka Expanded PID (EPID)<br/>Coord. only|No|
|bdbJoiningNodeNewTCLinkKey||No|
|bdbJoinUsesInstallCodeKey|Coord. only|No|
|bdbNodeCommissioningCapability|How can we commision nodes|No|
|bdbNodeIsOnANetwork|Joined to a network<br/>**Need to distinguish from commisioned**<br/>Note that this might indicate "this node was once joined" and therefore should try and rejoin the same network.  It is this that we expect to allow `CloudSMETS` to rejoin the SMETS network.|Yes|
|bdbNodeJoinLinkKeyType|Key type used to decrypt network key|No|
|bdbPrimaryChannelSet|Channels to scan|No|
|bdbScanDuration|Time per channel during scanning|No|
|bdbSecondaryChannelSet|2ry channels to scan|No|
|bdbTCLinkKeyExchangeAttempts|tries so far to exchange keys|No|
|bdbTCLinkKeyExchangeAttemptsMax|Max key exchange attempts|No|
|bdbTCLinkKeyExchangeMethod|Key exchange method|No|
|bdbTrustCenterNodeJoinTimeout|Coord. only|No|
|bdbTrustCenterRequireKey|Coord. only|No|

## Link Keys
Each node SHALL contain the following link keys:
1. The default global Trust Center link key (`ZigbeeAlliance09`)
1. The distributed security global link key (`0xd0d1d2d3d4d5d6d7d8d9dadbdcdddedf`)
1. An install code derived preconfigured link key (exchange by methods outside of the Zigbee specification)

> Question:/ Does SMETS use an _install code_?  If so, then we are up a creek!

## [ZDO]
It appears that the devices act on ZDO commands e.g. _Active_EP_req_.

## How Might We Join?

### Top-Down
- We are not already joined
- Initialization or commissioning is being performed
- We are supporting Finding & Binding and Network Steering
- Network Steering is performed to find a network to join
- A NLME-NETWORK-DISCOVERY.request is issued which causes...
- An MLME-SCAN.req to be issued...
- An 802.15.4 beacon request is generated
- 802.15.4 beacons are received from potentual routers or coordinators
- We may check the PAN-ID for a match and reject those if we expect to find a
  specific PAN-ID.

### Initialization
See [Node Initialization], [BDB] page 39.

Note that a node will not attempt to join a network on power-up **unless** is already believes it was part of a network i.e. _bdbNodeIsOnANetwork_ is **TRUE**.

> This probably explains the sampleSwitch issue; it believes it is already on a network but cannot find the network it thinks it is part of.

### [Commissioning]
See [Commissioning], [BDB] page 41.

> This doesn't attempt to join if _bdbNodeIsOnANetwork is **FALSE**.

Note that _Finding & Binding_ seems to be for a Coordinator or Router (_target endpoint_) to allow others (_initiator endpoint_) to join the network.  So presumably a Coordinator is _never on a network_ because it _is_ the network?

### [Network Steering]
This appears to be the point at which the _initiator endpoint_ finds the coordinator and/or router.

> Network discovery seems to be what we want to see happening.  But what does _NLME-NETWORK-DISCOVERY.request_ actually do?

> simpleSwitch does not appear to be attempting MAC association so it doesn't like the available network(s).

### First Time
[Finding & Binding] for an initiator endpoint seems to imply that IEEE 802.15.4 and ZigBee _Identify_ cluster etc are different.

> Question: Does joining take place at a level **below** the BDB?  MAC level?

## What's Wrong with SampleSwitch?
### PreInstallCode?
- Read from flash address `CFG_PRE_INSTALL_CODE`
  - Length is expected to be 16 (128 bits)
- `CFG_PRE_INSTALL_CODE` has value `0x78000`
- Found `DEFAULT_PANID` which should have worked but implied that perhaps it didn't?

#### Addresses...
See file `proj/drivers/drv_nv.h`.
|Address|Name|Function|
|-|-|-|
|0x76000|FLASH_ADDR_OF_MAC_ADDR_512K|MAC address|
|0x76040|2 bytes, Telink USB ID|
|0x77000|FLASH_ADDR_OF_F_CFG_INFO_512K|F_CFG_Info<br/>factory configure information|
|0x78000|CFG_PRE_INSTALL_CODE|User configuration|
|0x79000|CFG_FACTORY_RST_CNT|Set to anything other than 0xff to force a factory reset.|
|0x34000|NV_BASE_ADDRESS (bootloader)
|0x7A000|NV_BASE_ADDRESS2 (bootloader)
|0x6A000|NV_BASE_ADDRESS
|0x7A000|NV_BASE_ADDRESS2


[ZigBee Alliance]: https://zigbeealliance.org
[BDB]: https://zigbeealliance.org/wp-content/uploads/2019/12/docs-13-0402-13-00zi-Base-Device-Behavior-Specification-2-1.pdf
[Node Initialization]: https://zigbeealliance.org/wp-content/uploads/2019/12/docs-13-0402-13-00zi-Base-Device-Behavior-Specification-2-1.pdf#page=39
[Commissioning]: https://zigbeealliance.org/wp-content/uploads/2019/12/docs-13-0402-13-00zi-Base-Device-Behavior-Specification-2-1.pdf#page=41
[ZDO]: https://zigbeealliance.org/wp-content/uploads/2021/10/docs-05-3474-22-0csg-zigbee-specification.pdf#page=226
[ZCL]: https://zigbeealliance.org/wp-content/uploads/2019/12/07-5123-06-zigbee-cluster-library-specification.pdf
[Acronymns and Abbreviations]: https://zigbeealliance.org/wp-content/uploads/2019/12/docs-13-0402-13-00zi-Base-Device-Behavior-Specification-2-1.pdf#page=21
[BDB Attributes]: https://zigbeealliance.org/wp-content/uploads/2019/12/docs-13-0402-13-00zi-Base-Device-Behavior-Specification-2-1.pdf#page=24
[ZigBee RF4CE Specification]: https://zigbeealliance.org/wp-content/uploads/2019/11/095262r01ZB_zigbee_rf4ce_sc-ZigBee_RF4CE_Specification_public.pdf