#include "persistent_storage.h"
#include <nvs_flash.h>
#include <nvs.h>
#include <Arduino.h>

#define NVS_NAMESPACE "crocker_storage"

void storage_init() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    Serial.println("NVS storage initialized");
}

void save_brightness(uint8_t brightness) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    
    if (err == ESP_OK) {
        nvs_set_u8(handle, "brightness", brightness);
        nvs_commit(handle);
        nvs_close(handle);
        Serial.printf("Saved brightness: %d\n", brightness);
    } else {
        Serial.println("Error opening NVS handle");
    }
}

uint8_t load_brightness() {
    nvs_handle_t handle;
    uint8_t brightness = 128;  // Default value
    
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err == ESP_OK) {
        nvs_get_u8(handle, "brightness", &brightness);
        nvs_close(handle);
        Serial.printf("Loaded brightness: %d\n", brightness);
    } else {
        Serial.println("No saved brightness found, using default");
    }
    
    return brightness;
}

void save_alarm_enabled(bool enabled) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    
    if (err == ESP_OK) {
        uint8_t value = enabled ? 1 : 0;
        nvs_set_u8(handle, "alarm_enabled", value);
        nvs_commit(handle);
        nvs_close(handle);
        Serial.printf("Saved alarm_enabled: %d\n", enabled);
    } else {
        Serial.println("Error opening NVS handle");
    }
}

bool load_alarm_enabled() {
    nvs_handle_t handle;
    uint8_t value = 1;  // Default: alarm enabled
    
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err == ESP_OK) {
        nvs_get_u8(handle, "alarm_enabled", &value);
        nvs_close(handle);
        Serial.printf("Loaded alarm_enabled: %d\n", value);
    } else {
        Serial.println("No saved alarm_enabled found, using default");
    }
    
    return (value == 1);
}
