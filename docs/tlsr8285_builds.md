# Building TLSR8258 code
> Be aware that the Telink SDK contains two projects, one for RISC and one for TC32 processors.
>
> These projects have the **same** name as far as the Eclipse IDE are concerned so you cannot import both at the same time without first changing the name of one or both in their `.project` file.
>
> For the TLSR8258, you want the `tlsr_tc32` project.

## Importing SDK Project into the IDE
- Start the IDE (which is an old vesion of Eclipse; I do **NOT** recommend you update it!)
- Configure and allow it to create a new directory somewhere for workspaces; do not put this inside the SDK directory as you probably won't want to archive stuff created in it
- Import the workspace from the SDK using the following menu items:
  - 'File', 'Import...'
  - From the _folders_ select 'General', 'Existing Projects into Workspace'
  - Press 'Next >'
  - Ensure that 'Select root directory' is selected and enter the root directory of the Telink SDK
  - From the list of 'Projects' select the `tlsr_tc32` roject
  - Press 'Finish' to import the project.
- Once the import has completed, open the Workspace view by clicking the 'curling arrow icon'
- Now select 'Project', 'Build All' and see if the prokect will build
  - Below the SDK project, you should now find a `sampleGW_8269\sampleGw_8269.elf` image and associated files.

## Improve Build Speed
- Enable parallel building via menu 'Project', 'Properties'
- Select 'C/C++ Build'
- Select 'Behaviour' tab
- Check 'Use parallel build'
- Select 'Use optimal jobs number'
- Hit 'OK' to save
- Builds should be much faster now.

## Configuring for TLSR8258
There are a number of configurations defined in the project and it appears that perhaps the default is not the one we want.  The available configurations are:

|Chip Types|
|-|
|8258|
|8269|
|8278|

Configurations|Description|
|-|-|
|bootLoader|boot loader for...
|sampleContactSensor|EndDevice..
|sampleGW|Coordinator...
|samplelight|Router...
|sampleSwitch|EndDevice...

### sampleGW_8258
- Select the 'sampleGW_8258', 'Coordinator-8258' as the active configuration
- 'Project', 'Build All' again
- Confirm that build output now appears below `sampleGW_8258`.

### bootLoader_8258
- Failed to build because `BOOT_LOADER_MODE` is not set to `1`

> `BOOT_LOADER_MODE = 1` enables the bootLoader based startup whch limits image sizes to around 197KB.  See `tl_zigbee_sdk/apps/common/comm_cfg.h` for information where it recommends not using this mode for 512K flash chips.  **The TLSR8258 is a 512K flash chip!**

## So What does sampleGW_8258 Build?

> Before making changes, suggest we copy and existing project and create our own custom version.  Suggest names `cloudsmart_gw` and `cloudsmets_en` based on the `sampleGW` and one of the _End Device_

### sampleGW_8258
- See the [Telink Zigbee SDK Developer Manual] for some information
- Start from 'Window', 'Show View', 'Navigator' menu to display the code
- Open the 'tl_zigbee_sdk', 'apps', sampleGW' folders
- See the 'board...' files; which board are we using?
  - Might need a custom version of `board...h` that matches the development board
- Do we want to control the coordinator?  We might want to use the _Extended Function HCI Interface_ described in chapter 7 of the SDK docs
  - **Question:/** There are two USB interfaces described?
  - USB CDC is a [USB communications device class]
    - Can be made to look like a serial port to the host system
  - USB HIB is a [USB human interface device class]
    - Not sure how the TLSR8258 might use this but probably not what we want!
  - USB PRINT might be the TLSR8258 acting as a USB attached printer and accepting commands in that fashion.  Again, not what we want.
    - Apparently used with the "Telink BDT tool" (burning/flashing tool)
  - UART is presumably using the standard UART interface, whatever that might be

> Believe that `ZBHCI_USB_CDC` is the one we want.
>
> **We will need to enable this if appropriate, probably in `tl_sigbee_sdk/apps/app_cfg.h`

#### app_cfg.h
Contains **lots** of very useful hints as to what we might want to enable.

|Feature|What it does|
|-|-|
|UART_PRINTF_MODE|Debugging via one of the `GPIO` pins.<br/>**Need to make sure that the correct pin is defined for our board, and probably that suitable UARt configuration is performed**.
|BOARD_8258...|Defines the board type<br/>**Suggest we determine whether our board matches an existing type or we define a new one and add appropriate code wherever appropriate.**
|PM_ENABLE|Power management<br/>**Leave this disabled as CloudSMETS is mains powered.**
|PA_ENABLE|Seems to imply that an "external RF PA used" ie. not using the boards own radio system.<br/>**Leave this disabled.**
|CLOCK_SYS_CLOCK_HZ|Clock runs at 48MHz<br/>**Leave this unchanged.***
|VOLTAGE_DETECT_ENABLE|Not sure if the development board supports this but we are powering it off the mains anyway so no point enabling.
|MODULE_WATCHDOG_ENABLE|Not sure but presumably reboots the TLSR8258 is something bad happens and it appears dead.<br/>**Do not enable this but we might later if we that the board is not reliable.**
|ZCL cluster support flags|We probably do not want any of this but we should check.
|ZCL_OTA_SUPPORT|Might be interesting but does this imply OTa over Zigbee, which we definitely would not want.<br/>**Seems to mena OTA over Zigbee so not need to enable this.**

**ZCL Options**

See `zcl_config.h` where they are defined.

Enabling an option adds attributes and other parsing function.  Each supported option is defined through a _profile_ defined in an additional Zigbee specification e.g. the [Smart energy] profile

> It feels like perhaps _BASIC_ and _METERING_ are the two profiles that CloudSMETS needs to support.  Might also need _IDENTIFY_ or similar perhaps and maybe _TIME_ since the smart meter system seems to provide time information.

|Option|Purpose|
|-|-|
|****** General *******
|ZCL_BASIC|Basic
|ZCL_POWER_CFG_SUPPORT|Power Cluster
|ZCL_DEV_TEMPERATURE_CFG|Device temperature config
|ZCL_IDENTIFY|Identify cluster
|ZCL_GROUP_SUPPORT|Group cluster
|ZCL_SCENE_SUPPORT|Scene cluster
|ZCL_ON_OFF_SUPPORT|On/Off cluster
|ZCL_ON_OFF_SWITCH_CFG_SUPPORT|On/Off config cluster
|ZCL_LEVEL_CTRL_SUPPORT|Level control cluster
|ZCL_ALARMS_SUPPORT|Alarm cluster
|ZCL_TIME_SUPPORT|Time cluster
|ZCL_RSSI_LOCATION_SUPPORT|RSSI location cluster
|ZCL_DIAGNOSTICS_SUPPORT|Diagnostics
|ZCL_POLL_CTRL_SUPPORT|Poll control
|ZCL_GP_SUPPORT|Green power
|ZCL_BINARY_INPUT_SUPPORT|Binary input
|ZCL_BINARY_OUTPUT_SUPPORT|Binary output
|ZCL_MULTISTATE_INPUT_SUPPORT|Multistate input
|ZCL_MULTISTATE_OUTPUT_SUPPORT|Multistate output
|***** Measurement and Sensing ******
|ZCL_ILLUMINANCE_MEASUREMENT_SUPPORT|Illuminance Measurement
|ZCL_ILLUMINANCE_LEVEL_SENSING_SUPPORT|Illuminance Level Sensing
|ZCL_TEMPERATURE_MEASUREMENT_SUPPORT|Temperature Measurement
|ZCL_OCCUPANCY_SENSING_SUPPORT|Occupancy Sensing
|ZCL_ELECTRICAL_MEASUREMENT_SUPPORT|Electrical Measurement
|****** Lighting ******|
|ZCL_LIGHT_COLOR_CONTROL_SUPPORT|Color Control
|ZCL_BALLAST_CFG|Ballast Configuration
|****** HVAC ******
|ZCL_THERMOSTAT_SUPPORT|THERMOSTAT
|****** Closures ******
|ZCL_DOOR_LOCK_SUPPORT|Door Lock
|ZCL_WINDOW_COVERING_SUPPORT|Window Covering
|****** Safe and Security ******
|ZCL_IAS_ZONE_SUPPORT|IAS Zone
|ZCL_IAS_ACE_SUPPORT|IAS ACE
|ZCL_IAS_WD_SUPPORT|IAS WD
|****** Smart energy ******
|ZCL_METERING_SUPPORT|Metering support
|****** OTA Upgrading ******
|ZCL_OTA_SUPPORT|OTA Configuration
|****** Commissioning ******
|ZCL_COMMISSIONING|Commissioning
|ZCL_ZLL_COMMISSIONING_SUPPORT|Touchlink Commissioning
|****** WWAH *****
|ZCL_WWAH_SUPPORT| Work with all hub

#### Board Differences
There seems to be three different _standard_ TLSR8258 boards; which do we match?

|Board/<br/>Feature|Dongle<br/>(32pins)|Dongle<br/>(48 pins)|EVK|EVK v1p2|Development<br/>Board
|-|-|-|-|-|-|
|Button1|PD7|PD6|PD1|PB2|
|Button2|PA1|PD5|PD2|PB3|
|Button3||||PB4|
|Button4||||PB5|
|Green LED|PA0|PA2|PD4|PD3|PA0
|Red LED|PD4|PA3|PD0|PD5|PD4
|Power LED|red|red|red|red|red
|Prmit LED|green|green|green|green|green
|UART TX|PB1|PD7|PB1|PB1|PB1
|UART RX|PB7|PA0|PB0|PB0|PB7
|Debug TX (1)|PC4|PC6|PC7|PD0|PC4

1. The _Debug TX_ seems to be a software emulated UART.  We probably don't want to use this because we have a second UART available that we can use instead so we want to use that instead.

> Although the Lilygo board has a USB connector, the connections to the TLSR8258 are actually a plain UART and the T-U2T dongle does the UART/USB conversion.
>
> This also means that the ESP32-C3 has only one extra UART available over the one used for the _fake USB_ connection.
>
> The table above says that the development board is like the 32-pin dongle so we should use that as our target.  The board type defaults to `BOARD=BOARD_8258_DONGLE` for the TLSR8258 so no changes required there.
>
> The LilyGO board doesn't have buttons but the SampleGW doesn't use these anyway so we can continue to use the dongle.

#### Build Targets
Some variables are set via the 'Properties', 'C/C++ Build', 'Settings', 'Tool Settings' tab, 'TC32 Compiler', 'Symbols' dialog.  for example the `MCU_CORE_8258=1` value is set.

#### LED usage
Do we want to use the LEDs in the manner defined above, namely:

- Red LED on says that power is present
- Green LED says that the coordinator is ready to accept associations?

> Having a power LED ie helpful as the TLSR8258 power is controlled by the ESP32-C3 so if that is not _running_ properly then the TLSR8258 will be dead.

[Telink Zigbee SDK Developer Manual]: http://wiki.telink-semi.cn/doc/an/AN_19052900-E_Telink%20Zigbee%20SDK%20Developer%20Manual.pdf
[USB communications device class]: https://en.wikipedia.org/wiki/
[USB human interface device class]: https://en.wikipedia.org/wiki/USB_human_interface_device_class


[Smart energy]: https://zigbeealliance.org/wp-content/uploads/2019/11/docs-07-5356-19-0zse-zigbee-smart-energy-profile-specification.pdf