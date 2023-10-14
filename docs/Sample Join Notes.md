# Sample Join Notes

## First Join
- Source file is `firstjoin.pcapng`
- Frame numbers are shown as (123)

 1. Beacon request from the endpoint (initiator), board 2
 1. Beacon command from the coordinator (??) sending the IEEE 802.15.4 PAN (0x104f) and the ZigBee extended PAN (EU164) `a4:c1:38:e9:a0:26:91:6a`), board 1
    1. Beacon command contains **_Association Permit_** flag set
 1. IEEE 802.15.4 association request from board 2, sending it's extended source (EU164) `a4:c1:38:c3:6d:d6:cb:e4` and own PAN of 0xffff (not set yet?)
 1. ACK to the association request from the coordinator (implicit) carrying the same sequence number as the association request.
     1. Futher ACKs are just shown as"- ACKed" after the request below.
 1. IEEE 802.15.4 data from the initiator, appear to carry just the extended source (EU164) address - ACKed
 1. Association response returned from coordinator to initiator; carries new short address for the initiator (`0x7cf5`) - ACKed.
 1. Another data request from the initiator - why????? - ACKed
 1. Transport key is sent from the coordinator to the initiator
     1. `95 83 fc d6 bf fd 50 51 e2 55 cd ca 47 c5 94 d3`
         1. After we add this to Wireshark, we can see all the ZigBee requests
         1. _47 bytes_ is a _Link Status_
         1. _51 bytes_ is a Many-to-one Route Request
     1. Seems to be sent many times
     1. Has NOT been ACKed yet
1. Another data request from the intiator - ACKed
1. (112) Finally a transport key ACK from the initiator.
1. ...later...
1. (146) Initiator requests a Transport Key - ACKed
    1. _Trust Center Link Key_
1. (150) Coordinator sends a _Transport Key_, `e8 ae 2c 59 70 69 76 46 3c 78 72 e6 29 1f 3b df` - ACKed
1. (154) Initiator sends a _Verify Key_, `45 39 fe 58 be 5d da b1 56 9a 04 64 f0 1c 2e 99`
1. (162) Coordinator sends a _Confirm Key, SUCCESS_

> There are other frames such as APS flowing and these seem to be querying supported cluster IDs.
>
> There is also a _ZCL Identify_ which seends to indicate something about home automation.

## ReJoin
- Source file is `rejoin.pcapng`
- Frame numbers are shown as (123)

SampleLight joined to the SampleGW but was then unplugged and plugged in again; what happened next?

1. (4) Rejoin request is made and there is no key exchange.

> So how do we force a new key exchange?  Need to somehow clear stored data but where is this?
