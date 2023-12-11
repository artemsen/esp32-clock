// SPDX-License-Identifier: MIT
// Super-duper-clock.

#include "display.h"

#include <esp_log.h>
#include <esp_sntp.h>
#include <esp_timer.h>
#include <esp_wifi.h>
#include <nvs_flash.h>

// Log tag
static const char* TAG = "clock";

// Timer callback: called once per second
static void on_clock_tick(void* arg)
{
    display_update();
}

// NTP callback: sync complete
static void on_time_sync(struct timeval* tv)
{
    ESP_LOGI(TAG, "ntp sync complete");
    display_setup(16, 0xff, 0, 0);
}

// Initialize Network Time Protocol client
static void ntp_init(void)
{
    esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(on_time_sync);
    esp_sntp_init();
}

// Event handler for network state notifications
static void on_net_event(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT) {
        if (event_id == WIFI_EVENT_STA_START || event_id == WIFI_EVENT_STA_DISCONNECTED) {
            ESP_LOGW(TAG, "wifi reconnect");
            display_setup(8, 0x80, 0x80, 0x80);
            esp_wifi_connect();
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "wifi connected");
        display_setup(8, 0xff, 0x80, 0);
        ntp_init();
    }
}

// Initialize WiFi
static void wifi_init(void)
{
    esp_event_handler_instance_t event;
    const wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();

    wifi_config_t wifi_cfg = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        }
    };
    strcpy((char*)wifi_cfg.sta.ssid, WIFI_SSID);
    strcpy((char*)wifi_cfg.sta.password, WIFI_PASSWORD);

    // initialize wifi
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    ESP_ERROR_CHECK(esp_wifi_init(&init_cfg));

    // set callbacks for network stat events
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
        ESP_EVENT_ANY_ID, &on_net_event, NULL, &event));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
        IP_EVENT_STA_GOT_IP, &on_net_event, NULL, &event));

    // setup wifi
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    // start wifi client
    ESP_LOGI(TAG, "start wifi connection with %s", WIFI_SSID);
    ESP_ERROR_CHECK(esp_wifi_start());
}

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

    ESP_LOGI(TAG, "init start");

    // set timezone
    setenv("TZ", "GMT-3", 1);
    tzset();

    // init LCD
    display_init();
    display_setup(8, 0, 0, 0x80);

    // initialize nvram and wifi
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init();

    // register periodic timer
    ESP_ERROR_CHECK(esp_timer_create(&ptimer_args, &ptimer_handle));
    ESP_ERROR_CHECK(esp_timer_start_periodic(ptimer_handle, 1000000)); // 1 sec

    ESP_LOGI(TAG, "init complete");
}
