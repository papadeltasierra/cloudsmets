# WiFi Task Design
## States
- Power on
- Starting WiFi Direct
- WiFi Direct active
- Starting WiFi network
- WiFi Network active

## Inputs
- Valid WiFi configuration
- No valid configuration
- WiFi configuration removed
- Wifi network failed
- Reset to Wifi direct (from webpage or key press)
- Disable Basic WiFi
- Enable Basic WiFi

## State Table
```mermaid
---
title: WiFi Task state table
---
    flowchart TD;

    %% States
    powerOn[Power-on];
    wifiDirectStarting[Starting WiFi Direct];
    wifiDirectActive[WiFi Direct Active];
    wifiStarting[Starting WiFi];
    wifiActive[Wifi Active];
    isWifiDirectActive{Is WiFi Direct<br/>Active?}
    isConfigAvailable{Is Config<br/>Available?}
    isWifiEnabled{Is WiFi<br/>Enabled?}

    %% Actions
    startWifiDirect([Start WiFi Direct])
    startWifi([Start WiFi])
    stopWifi([Stop Wifi])
    waitFor30s([Wait for 5s])


    %% Inputs/State changes
    powerOn-->isWifiEnabled

    isWifiEnabled-->|Yes|isConfigAvailable
    isWifiEnabled-->|No|isWifiDirectActive

    isConfigAvailable-->|No|isWifiDirectActive
    isConfigAvailable-->|Yes|startWifi
    startWifi-->wifiStarting

    wifiStarting-->|Wifi Active|wifiActive
    wifiStarting-->|Wifi Failed|startWifiDirect

    wifiDirectStarting-->|WiFi Direct active|wifiDirectActive
    wifiDirectStarting-->|Wifi Direct failed|waitFor30s
    waitFor30s-->isWifiEnabled

    wifiActive-->|Wifi failed|wifiDirectStarting
    wifiActive-->|Configuration removed|stopWifi
    wifiActive-->|Disable Basic WiFi|stopWifi
    stopWifi-->wifiDirectStarting

    wifiDirectActive-->|WiFi configuration|isWifiEnabled
    wifiDirectActive-->|Enable Basic Wifi|isWifiEnabled
    wifiDirectActive-->|WiFi Direct failed|isWifiEnabled
    wifiDirectActive-->|Timer check|isWifiEnabled

    isWifiDirectActive-->|Yes|wifiDirectActive
    isWifiDirectActive-->|No|startWifiDirect
    startWifiDirect-->wifiDirectStarting




```