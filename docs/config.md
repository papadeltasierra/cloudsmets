# Sample Configuration
The following is an exmaple of the configuration layout but note the following:

- The real configuration is stored with no whitespace at all to make it as 
  compact as possible
- The real configuration uses single character keys, again to make it compact
- keys only need to be unique at the level that they occur (so "`c`" can be used as a key in a number of distinct places).

### Key Mappings
|Short|||Full key|Purpose|
|-|-|-|-|-|
|v|||cloudsmets|Configuration structure version
|w|||wifi|WiFi connection parameters
||s||ssid|
||p||password|
|c|||cloud|Cloud server configuration
||c||common|Common configuration
|||p|period|Tiem between MQTT Publish requests in seconds
||a||azure|Azure specific configuration
|||e|enabled
|||h|IoT hub|
|||n|device name|
|||s|connection string|
||g||google|Google Cloud specific configuration
|||e|enabled
||w||aws|Amazon Web Services specific configuration
|||e|enabled
|z|||zigbee|ZibBee configuration
||i||hid|...about your IHD
|||g|GUID|Globally unique ID
|||k|key|Encryption key
|o|||ota|Other-the-air configuration
||e||enabled
||s||server|Server from which to pull OTA updates
||k||signing keys|Public OTA image signing keys
||f||force|Force update to this version if not empty

## Representative JSON
```json
{
    "cloudsmets": "1.2.3",
    "wifi": {
        "ssid": "your-router-name",
        "password": "tell-nobody"
    },
    "cloud": {
        "common": {
            "period": 300
        },
        "azure": {
            "enabled": true,
            "IoT hub": "my-iothub",
            "device name": "cloudsmets",
            "connection string": "gfjgdhfjshjgfdngjfngkfdn"
        },
        "google": {
            "enabled": false,
        },
        "aws": {
            "enabled": false,
        }
    },
    "zigbee": {
        "ihd": {
            "GUID": "00-11-22-33-44-55-66-77",
            "key": "01234567890ABCDEF"
        }
    },
    "ota": {
        "enabled": true,
        "server": "https://github/cloudsmets/images",
        "signing keys": [
            "01234567890ABCDEF",
            "01234567890ABCDEF"
        ],
        "force": "1.2.3"
    }
}
```