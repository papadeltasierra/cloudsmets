# ESP32 Components
```mermaid

flowchart TD;
  %% Components
  wifi[WiFi connection]
  web[Web server]
  logger[WiFi logger]
  config[Cofiguration store]
  ntp[NTP client]
  tlsr8258[tlsr8258 serial port]
  smets[SMETS processing]
  azure[Azure MQTT]

  %% Connections
  wifi -->|Config<br/>request/set|config
  wifi -->|Active|web

  web -->|WiFi config<br/>request/set|wifi
  web2>From web] -->|NTP config<br/>request/set|ntp
  web3>From web] -->|SMETS config<br/>request/set|smets
  web4>From web] -->|Azure config<br/>request/set|azure
  web5>From web] -->|WiFi logger config<br/>request/set|logger

  ntp -->|Config<br/>request/set|config
  SMETS -->|Config<br/>request/set|config
  Azure -->|Config<br/>request/set|config

  web -->|Web requests|web6>From web]

  wifi --> ntp
  wifi --> azure
  wifi --> logger

  smets --> azure
  smets --> tlsr8258

  tlsr8258 --> smets
```

## Component Creation Order
1. Configuration store
1. tlsr8258 serial port
1. SMETS processing; 2-way with tlsr8258
1. WiFi logger
1. Azure MQTT
1. Web server; 2 way with WiFi connection
1. NTP client
1. WiFi connection; 2 way with web
