# Designing a case for the Lilygo T-Zigbee
## Introduction
CloudSMETS requires a case for the Lilygo T-Zigbee board and these are notes on how this was designed using [Autodesk]'s [Fusion 360] CAD software.  The notes draw heavily on the very helpful [Step By Step Tutorial] video created by [Robert Feranec].

There is a section on the logic of the design process and some notes on elements of [Fusion 360] usage that were not immediately obvious to me.

> Questions: Answer these before completing this document
> - Is the _ESP-C3_ `Red` LED actually for the ESP-C3 or is it a generic power LED?

## Design Overview
### Target Design
The final case has these characteristics:
- It is a snug fit for the LilyGo T-Zigbee board
- It consists of a base and a cover
- There are `cut-outs` for:
    - The USB charger port
    - The (ESP-C3) `Key` button
    - The `Reset` button
- There are holes above the three LEDs so that they can be seen:
    - The `Red` (referred to as `A`) and `Green` LED (referred to as `B`) for the tlsr8258
    - The `Red` LED (referred to as `C`) for the ESP-C3 (??)
- There is a `CloudSMETS` name cut-out on the cover
- There are no additional ventilation holes are these should not be required.

#### LED pipes?
A future enhancement might add LEDs pipes, so that the LEDs are more easily visible, by doing the following:
- Adding some strenghtniong around the holes in the cover so that a circular hole can be drilled that will hold...
- Short lengths of perspex rod from the cover to the LEDs to transmit the LED colour to the top of the cover.

### Design Parameters
The design will use the following common parameters and measurements will be relative to these, combined with the information on the PCB determined below.

|Parameter|Value|
|-|-|
|PCB clearance<br/>(PCB edges to case walls)|0.5mm|
|Case wall thickness|2mm|

## Design Process
### Measure the PCB
It is important that we know the dimensions of the LilyGo T-ZigBee PCB before we start.  All dimensions assume that the USB connector is bottm-left as we look at the top of the board.

|Dimension|Value|
|-|-|
|Length of PCB|74.7mm|
|Width of PCB|29.0mm|
|Depth of PCB|2.0mm|
|Mounting hole (MH) diameter|3.0mm|
|MH length spacing|70.0mm|
|MH width spacing|25.0mm|
|Left MH centre from left edge|
|Right MH centre from left edge|
|Top MH from top edge|
|Bottom MH from top edge|
|USB width (1, 2)|
|USB height (1, 2)|
|USB bottom relative to PCB top (1, 2)|
|Key width (2)|
|Key height (2)|
|Key bottom relative to PCB top (2)|
|Reset width (2)|
|Reset height (2)|
|Reset bottom relative to PCB top (2)|
|LED "view" diameter (all)|2.0mm|
|LED A length from PCB left edge|
|LED A width from PCB top edge|
|LED B length from PCB left edge|
|LED B width from PCB top edge|
|LED C length from PCB left edge|
|LED C width from PCB top edge|

#### Notes
1. Remember that the cut-out for the USB connect has to allow for the additional width of the USB plug itself; I am assuming a [Raspberry Pi 4 Power Supply].
1. These are the dimensions required for the case cut-out and not the sise of the item on the board.

### Define the PCB
[Fusion 360] is used to design case using the following logical steps.
1. A (body), herby referred to as the "PCB", is created which minics the LilyGo T-ZigBee board.  The PCB is then used to provide reference points around which the base and cover can be designed.  For example the USB socket is designed as part of this box, then the cover is extruded around the USB socket.

### Create the Base
x

### Crease the Cover
x

## Fusion 360 Notes
The following are things learnt about how to use [Fusion 360] as I was creating the box.  They relate to [Fusion 360] version `(v.2.0.16985 ) – August 24, 2023` so things might have changed if you are using a newer version.

## Extruding
### Fusion 360 Terminology
|Term|Explanation|
|-|-|
|Drawing||
|Sketch||
|Body||

### Join To...
If you want your extrusions to join to an exising body (?) then you should have that body, **and just that body**, enable when you select `Join To` from the extrusion drop down.  You can then hide the box whilst you set-up the remainder of the extrusion info and [Fusion 360] remembers which body to join your new extrusion to.


[Autodesk]: https://www.autodesk.co.uk/
[Fusion 360]: https://www.autodesk.co.uk/products/fusion-360/overview?term=1-YEAR&tab=subscription
[Step By Step Tutorial]: https://www.youtube.com/watch?v=lII0ldT5TSk
[Robert Feranec]: https://www.youtube.com/@RobertFeranec
[Raspberry Pi 4 Power Supply]: https://thepihut.com/products/raspberry-pi-psu-uk?variant=20064070303806