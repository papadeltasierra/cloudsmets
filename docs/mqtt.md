# MQTT Usage


## MQTT Topics

### Microsoft Azure

### Google Cloud

### Amazon Web Services

## MQTT Payload


### Notes
- The data is sent without whitespace to reduce the size of the messages

|Field|Syntax|
|-|-|
|cloudsmets||dentifies this as data from cloudsmets|
|GUID|The IHD GUID to allow a family, say, to use a single server that display energy data from multiple homes (strictly multiple IHDs).
|time|Time in seconds since the UTC epoch (00:00:00 on 1st January 1970)
|electricity|Meter reading in kWh; one entry per tarrif
|gas|Meter reading in cubic-meters; one enrty per tarrif

## Representative JSON

```json
{
    "cloudsmets": {
        "GUID": "00-11-22-33-44-55-66-77",
        "time": 12345678,
        "electricity": [
            1234.56,
            7890.34
        ],
        "gas": [
            1234.56
        ]
    }
}    
```