// SPDX-License-Identifier: MIT
// Super-duper-clock.

#include "display.h"

#include <bme280.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_sntp.h>
#include <esp_timer.h>
#include <esp_wifi.h>
#include <i2c_bus.h>
#include <nvs_flash.h>

// Log tag
static const char* log_tag = "clock";
// BME280 sensor hadle
static bme280_handle_t bme280;
// Current info to display
static struct info info;

// Timer callback: called once per second
static void on_clock_tick(void* arg)
{
    float sensor;
    time_t now_utc;
    struct tm now_local;

    // current time
    time(&now_utc);
    localtime_r(&now_utc, &now_local);
    info.hours = now_local.tm_hour;
    info.minutes = now_local.tm_min;
    info.seconds = now_local.tm_sec;

    // BME280 sensor data
    info.temperature =
        bme280_read_temperature(bme280, &sensor) == ESP_OK ? sensor : 0;
    info.humidity =
        bme280_read_humidity(bme280, &sensor) == ESP_OK ? sensor : 0;
    info.pressure =
        bme280_read_pressure(bme280, &sensor) == ESP_OK ? sensor : 0;

    ESP_LOGD(log_tag, "%02d:%02d:%02d t=%.1f h=%.1f p=%.1f", info.hours,
             info.minutes, info.seconds, info.temperature, info.humidity,
             info.pressure);

    display_redraw(&info);
}

// Initialize BME280 sensor
static void bme280_init(void)
{
    const i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GPIO_NUM_18,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = GPIO_NUM_19,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    const i2c_bus_handle_t bus = i2c_bus_create(I2C_NUM_0, &conf);

    bme280 = bme280_create(bus, BME280_I2C_ADDRESS_DEFAULT);
    bme280_default_init(bme280);
}

// NTP callback: sync complete
static void on_time_sync(struct timeval* tv)
{
    ESP_LOGI(log_tag, "NTP sync completed");
    info.ntp = true;
}

// Initialize Network Time Protocol client
static void ntp_init(void)
{
    esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(on_time_sync);
    esp_sntp_init();
}

// Event handler for network info notifications
static void on_net_event(void* arg, esp_event_base_t event_base,
                         int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT) {
        if (event_id == WIFI_EVENT_STA_START ||
            event_id == WIFI_EVENT_STA_DISCONNECTED) {
            ESP_LOGW(log_tag, "Reconnect WiFi");
            info.wifi = false;
            info.ntp = false;
            esp_wifi_connect();
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(log_tag, "WiFi connected");
        info.wifi = true;
        ntp_init();
    }
}

// Initialize WiFi
static void wifi_init(void)
{
    esp_event_handler_instance_t event;
    const wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();

    wifi_config_t wifi_cfg = {
        .sta.threshold.authmode = WIFI_AUTH_WPA2_PSK,
    };
    strcpy((char*)wifi_cfg.sta.ssid, WIFI_SSID);
    strcpy((char*)wifi_cfg.sta.password, WIFI_PASSWORD);

    // initialize wifi
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    ESP_ERROR_CHECK(esp_wifi_init(&init_cfg));

    // set callbacks for network stat events
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &on_net_event, NULL, &event));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &on_net_event, NULL, &event));

    // setup wifi
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    // start wifi client
    ESP_LOGI(log_tag, "Start WiFi connection with %s", WIFI_SSID);
    ESP_ERROR_CHECK(esp_wifi_start());
}

// Entry point
void app_main(void)
{
    const esp_timer_create_args_t ptimer_args = {
        .callback = &on_clock_tick,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "time",
        .skip_unhandled_events = true,
    };
    esp_timer_handle_t ptimer_handle;

    ESP_LOGI(log_tag, "Initialization started");

    // set timezone
    setenv("TZ", "GMT-3", 1);
    tzset();

    bme280_init();
    display_init();
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init();

    // register periodic timer
    ESP_ERROR_CHECK(esp_timer_create(&ptimer_args, &ptimer_handle));
    ESP_ERROR_CHECK(esp_timer_start_periodic(ptimer_handle, 1000000)); // 1 sec

    ESP_LOGI(log_tag, "Initialization completed");
}
