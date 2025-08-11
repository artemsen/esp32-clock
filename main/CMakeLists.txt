# register project as IDF component
idf_component_register(
    SRCS         "main.c" "resources.c" "display.cpp"
    INCLUDE_DIRS "."
    REQUIRES     "lgfx" "esp_wifi"
)

# WiFi name and password
if(DEFINED ENV{WIFI_SSID})
    set(WIFI_SSID $ENV{WIFI_SSID})
else()
    set(WIFI_SSID WiFiNameNotSpecified)
endif()
idf_build_set_property(COMPILE_DEFINITIONS "-DWIFI_SSID=\"${WIFI_SSID}\"" APPEND)

if(DEFINED ENV{WIFI_PASSWORD})
    set(WIFI_PASSWORD $ENV{WIFI_PASSWORD})
else()
    set(WIFI_PASSWORD WiFiPasswordNotSpecified)
endif()
idf_build_set_property(COMPILE_DEFINITIONS "-DWIFI_PASSWORD=\"${WIFI_PASSWORD}\"" APPEND)
