# Core dump Debugging

Ref: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/core_dump.html

## Development
### Core dump to UART then decode
Enable _Core Dump to UART_
- `idf.py menuconfig`
- Component config
- Core dump
- Data destination (UART)

From UART (e.g. `Putty`) output, cut the Base64 encoded data out; do not keep the `CORE DUMP START/END` lines.

```
================= CORE DUMP START =================
<body of Base64-encoded core dump, save it to file on disk>
================= CORE DUMP END ===================
```

Decode using `idf.py` as follows (you must provide some sort of port!)

`idf.py coredump-info --core putty.log -p /dev/x >crash.log`