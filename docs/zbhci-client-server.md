# How to use zbhci etc
1. Want to be able to send requests from CloudSMETS to ESME
1. Want to be able to query local config such as secret key and IEEE address.

# Investigations
- Looking at the sampleLight and sampleGW applications.

## sampleLight
### sampleLightEpCfg.c
- `g_zcl_basicAttrs` is basic attributes including the device name
- `basic_attrTbl` references the attributes in `g_zcl_basicAttrs`
- `g_sampleLightClusterList` references `basic_attrTable`
- `g_sampleLightClusterList` also references `sampleList_basicCB` which is a callback method
    - appears that ZCL (handling the ZigBee communications) calls this either before or after reading the attributes (if appropriate)
    - currently does nothing but could reset all to factory defaults (or do something else?)
- seems that basic attributes are just read and passed along.

## sampleGW
- Sets up basic attributes in similar manner to `sampleLight`
- So also expects tobe queries, but never show these values locally?

There are `zcl` callbacks that trigger the `zbhci`` interface so how are these triggered?

### ZDO callbacks
This is the ZDO stuff relating to the ZigBee device on the network.
- `sampleGW_devAnnHandler` is called is a device announcement message is received and it sends an indicating to the _user_ over `zbhci`
- a `zdo_appIndCb_t` list, `appCbLst`, seems to define callbacks although it is not clear whether trhese are all the callbacks possible?
- `zb_zdoCbRegister()` registers ZDO callbacks.

### ZCL/zbhci Callbacks?
- `sampleGW_zclReadRspCmd()` sends an _attribute read_ response to the user over the `zbhci`
- `sampleGW_zclReadRspCmd()` is called from `sampleGW_zcl_ProessIncomingMsg()`
- `sampleGW_zclProcessIncomingMsg()` is a parameter to `zcl_init()`
- `zcl_init()` registers a callback for the _ZCL Foundation command/response messages_
- **NOTE** that there is processing in the callback for an attribute read **response** but not for an attribute read **request**
    - Does this mean that read requests just return the attribute?
- `sampleGW_zclCfgReportCmd()` is a handler for a **request**
    - It does nothing!
- `sampleGW_zclCfgReportRspCmd()` does send stuff over the `zbhci`

> Think: This implies that perhaps the `zbhci` is intended for use in one direction only and not really designed for the use we are putting it to!
>
> However we will already need to extend is to handle the Smart Enery profile (probably) so this is not an issue.  We can make new commands do whatever we want!
>
> We will probbaly also want to ensure that as much of the unwanted code as possible is disabled.

- `sampleGW_zclCfgReportCmd()` does cause a request to be sent upwards to the user via the `zbhci`

> Question:/ When does `zbhci_clusterCommonCmdHandle()` get involved?

- `zbhci_clusterCommonCmdHandle()` is scheduled as a task from `zbhciCmdHandler()`
- `zbhciCmdHandler()` in invoked from `uart_data_handler()` i.e. on receipt of stuff over the `zbhci`
- `zbhciCmdHandler()` receives requests/responses from `zbhci` and then triggers the appropriate processing.

> Question:/ Can we add extra commands?
>
> Not easily because the default action is is just to return an invalid command response but perhaps we can patch because this is code that we compile as part of building a project.


# What is the ZDO Layer?
Ref: [ZDO & ZDP]

ZigBee Device Object; this handles _the ZigBee device on the network_

# What is the ZDP Layer?
Ref: [ZDO & ZDP]

ZigBee Device Profile; this handles attributes, clusters etc.

[ZDO & ZDP]: https://www.sciencedirect.com/science/article/abs/pii/B9780750685979000057