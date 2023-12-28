# As well as the source code, register the HTML and CSS files that we use to
# provide the web server.
idf_component_register(
    SRCS "cs_app.c" "cs_flash.c" "cs_cfg.c" "cs_mqtt.c" "cs_ota.c" "cs_time.c" "cs_web.c" "cs_wifi.c" "cs_zigbee.c"
    INCLUDE_DIRS "."
    EMBED_TXTFILES index.html
    EMBED_TXTFILES menu.html
    EMBED_TXTFILES wifi.html
    EMBED_TXTFILES web.html
    EMBED_TXTFILES ota.html
    EMBED_TXTFILES mqtt.html
    EMBED_TXTFILES tools.js
    EMBED_TXTFILES style.css)

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
