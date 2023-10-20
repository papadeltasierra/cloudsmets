# CloudSMETS tlsr8258 Design Notes

Everything below refers to the [Telink] tlsr8258 portion of the CloudSMETS device unless explcitly state otherwise.

# ISSUES/QUESTIONS
> These should al be resolved and removed before this project is completed.
1. What does the __PROJECT_CLOUDSMETS__ build variable do?
    - Seems that this protects the source code which implies that some of the source code is common between various [Telink] examples.
    - Propoose to remove all unused code and retire the variable.
1. There are lots of options being build in; we don't need them but we will need some others, probably.
1. What is "retention" in
1. Need to update Copyright messages.
1. what are the ZigBee clusters requierd for SMETs?
1. What is the HA profile required?
    - What is HA?!
1. What are "scenes"?
1. Can we just delee zb_afTestCb.c?  Looks like it would never be used
1. Can we enable debugging via the extra TX line and if so, what should we be logging?
1. What is read/write/report?  Do we only need read?
1. What profiles etc does a
1. Wht are _Time_ and _Scenes_ handled quite differently by the [ZigBee SDK Library]?

## Requirements
- CloudSMETS is a [ZigBee] _end device_ i.e. it comments out to a _coordinator_ and does not relay ZigBee signals like a _router_.
- CloudSMETS speaks ZigBee over the radio network to the [SMETS Technical Spec] (Smart Metering Equipment Technical Specifications, Version 2) electricity meter, whch acts as the ZigBee _coordinator_.
- CloudSMETS communicates with the [ESP32-C3] over a serial port (UART) interface using the _zbhci_ interface defined by [Telink].
- CloudSMETS accepts meter reading information from the electricity meter (both electricity and gas; the electricity meter acts as a realy for this information) and passes it to the [ESP32-C3].
- CloudSMETS firmware can be (if achievable) upgraded over the serial lik, ultimately driven by the [ESP32-C3] downloading the tlsr8258 firnmware and driving the upgrade.

# ZigBee Clusters
See the [ZigBee Cluster Library] documentation for reference.

We have metering (at least some) and should be able to produce an attribute table for pricing based on the [ZigBee Cluster Library] documentation.

See [SMETS technical Spec], section 6.3.2 onwards.  These apply for both electricy and gas unless stated otherwise.
|Section|Purpose|Cluster|Attr Set|Attr/Cmd|Telink provides?|
|-|-|-|-|-|-|
|6.3.2.1|HAN signal strength|N/A|N/A|N/A|?
|6.3.2.2|Local time with<br/>BST adjustment|Time|N/A|Time<br/>Maybe localTime|Yes
|6.3.3.1|Active Tariff Price|Price|N/A|PublishPrice<br/>(cmd, D.4.2.4.1)|No
|6.3.3.2|Aggregate Debt|Prepayment|Debt Attribute|DebtAmmount
|6.3.3.3|Aggregate Debt Recovery Rate|Prepayment|Debt Attribute|DebtRecoveryAmount/<br/>DebtRecoveryFrequency
|6.3.3.4|Cumulative Consumption<br/>(Amount)|Prepayment|Historical Consumption|CurrentDayConsumptionDelivered<br/>CurrentWeekConsumptionDelivered<br/>CurrentMonthConsumptionDelivered|Yes
|6.3.3.4|Cumulative Consumption<br/>(Cost)|Prepayment|Historical Cost Consumption|CurrentDayCostConsumptionDelivered<br/>CurrentWeekCostConsumptionDelivered<br/>CurrentMonthCostConsumptionDelivered|Yes
|6.3.3.5|Emergency Credit Balance|Prepayment|Prepayment Information|EmergencyCreditRemaining<br/>CreditStatus|No
|6.3.3.4|Historic Consumption|Metering|Historical Consumption|CurrentDay/Week/MonthCnosumption<br/>PreviousDay/Week/MonthNConsumption|Yes
|6.3.3.5|Low Credit Alert|Prepayment|Prepayment Information|CreditRemaining<br/>EmergencyCredit<br/>CreditLimit<br/>CreditStatus|No
|6.3.3.9|Meter Balance|Prepayment|Prepayment Information|AccumulatedDebt<br/>CreditRemaining|No
|6.3.3.10|Payment Mode|Prepayment|Prepayment Information|PaymentControlConfiguration|No
|6.3.4.10|Power Threshold Status<br/>(electricity only)|Metering|Meter Status|AmbientConsumptionIndicator


## Starting Point
The starting point for the CloudSMETS application is the samples provided by [Telink] as part of their [ZigBee SDK].  The `sampleSwitch` and `sampleContactSensor` devices both act as _end device_s and the `sampleSwitch` is chosen as the starting point as it is anticipated that this is the device that accepts information from the ZigBee network and act upon it.

## Development Notes
- The source code for the `sampleSwitch` is copied to the `cloudsmets` directory in the CloudSMETS repo, with `sampleSwitch*` files being renamed to `cloudsmets*`.
- Files for boards other than the tlsr8258 dongle are removed.
- Build configuration is copied from the `sampleLight`` project in the [ZigBee SDK] build environment.
- Perform complete clean-and-build to see if the project will actually build.
    - It does not; reference to missing `user-init` method.
      - Build configuration had been changed to `workspace` scope but not at project scope which meant that the build was still building with `__PROJECT_TL_SAMPLELIGHT__` set instead of `__PROJECT_CLOUDSMETS__`.
    - Still no; import of `sampleSwitch.h` fails.
        - Pattern change of `sampleSwitch` to `cloudsmets`
    - Still no; lots of missing references implying that the renamed files are not being built.
        - Clean-and-build did not solve this; assume this is an eclipse "find files" error.
    - Build issues traced to `sampleList` in various places plus some slightly off build options.
    - It now builds!
- Enabled UART and threfore zbhci
- Removed "switch" type profile flags and added metering
- Enabled ALL metering attributes (for now)
- "PM" is "power management"; don't bother with this.







[Telink]: https://www.telink-semi.com/
[ZigBee]: https://zigbeealliance.org/wp-content/uploads/2019/11/docs-05-3474-21-0csg-zigbee-specification.pdf
[SMETS Technical Spec]: https://assets.publishing.service.gov.uk/government/uploads/system/uploads/attachment_data/file/68898/smart_meters_equipment_technical_spec_version_2.pdf
[ESP32-C3]: https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf
[eclipse]: https://www.eclipse.org/ide/
[ZigBee Cluster Library]: https://zigbeealliance.org/wp-content/uploads/2021/10/07-5123-08-Zigbee-Cluster-Library.pdf