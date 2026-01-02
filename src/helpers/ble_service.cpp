#include "ble_service.h"
#include "ble_file_transfer.h"
#include "JSON_reader.h"
#include "JSON_writer.h"
#include "schedule_manager.h"
#include <Arduino.h>
#include "SD_MMC.h"
#include <nvs_flash.h>
#include <nvs.h>
#include <time.h>
#include "squarelineUI/ui.h"

// Global pointers
static BLEServer* pServer = nullptr;
static BLECharacteristic* pConfigChar = nullptr;
static BLECharacteristic* pFileChar = nullptr;
static BLECharacteristic* pStatusChar = nullptr;
static BLECharacteristic* pTimeSyncChar = nullptr;
static bool deviceConnected = false;

// Time sync management
#define TIME_SYNC_NVS_NAMESPACE "time_sync"
#define NVS_UPDATE_INTERVAL_MS 60000  // Update NVS every 1 minute
static uint64_t lastSyncedUnixTime = 0;  // Last verified time from phone
static uint32_t millisAtSync = 0;        // What millis() was at that moment
static bool timeIsValid = false;         // Has time been synced at least once
static bool firstSyncSinceConnection = true;  // Track if we've had first sync
static uint32_t lastNVSUpdateMillis = 0;  // Last time we updated NVS with current time
static bool updateScreen2AfterTimeSync = false;  // Flag to update Screen 2 after time changes

// Buffer for JSON config - deferred to main loop to avoid stack overflow
// Dynamically allocated from heap/PSRAM to avoid DRAM limits
#define JSON_BUFFER_SIZE 4096  // Large buffer in heap/PSRAM, not static DRAM
static char* jsonConfigBuffer = nullptr;
static size_t jsonConfigLength = 0;
static volatile bool jsonConfigReady = false;

// Queue for BLE data chunks - more robust than single buffer
#define BLE_DATA_BUFFER_SIZE 512
#define BLE_QUEUE_SIZE 8

struct BLEDataPacket {
    uint8_t data[BLE_DATA_BUFFER_SIZE];
    size_t length;
};

static BLEDataPacket bleQueue[BLE_QUEUE_SIZE];
static volatile uint8_t bleQueueHead = 0;
static volatile uint8_t bleQueueTail = 0;

// Server callbacks to track connection state
class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        firstSyncSinceConnection = true;  // Reset first sync flag on new connection
        Serial.println("BLE Client Connected");
        updateBLEStatus(STATUS_IDLE, "Connected");
    }

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        Serial.println("BLE Client Disconnected");
        
        // Restart advertising so clients can reconnect
        BLEDevice::startAdvertising();
        Serial.println("BLE advertising restarted");
    }
};

// Config characteristic callbacks - receives JSON configuration
class ConfigCharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();
        
        if (rxValue.length() == 0) {
            Serial.println("[BLE CONFIG] ERROR: Empty write received");
            return;
        }
        
        Serial.println("\n======================================");
        Serial.println("[BLE CONFIG] Received JSON data:");
        Serial.println("======================================");
        Serial.println((const char*)rxValue.c_str());
        Serial.println("======================================\n");
        
        // Allocate buffer on first use (from heap/PSRAM, not static DRAM)
        if (jsonConfigBuffer == nullptr) {
            jsonConfigBuffer = (char*)malloc(JSON_BUFFER_SIZE);
            if (jsonConfigBuffer == nullptr) {
                updateBLEStatus(STATUS_ERROR, "Memory allocation failed");
                Serial.println("[BLE CONFIG] ERROR: Failed to allocate JSON buffer!");
                return;
            }
        }
        
        // Buffer JSON for processing in main loop (NOT in callback - avoids stack overflow)
        if (rxValue.length() < JSON_BUFFER_SIZE) {
            memcpy(jsonConfigBuffer, rxValue.c_str(), rxValue.length());
            jsonConfigLength = rxValue.length();
            jsonConfigReady = true;
            updateBLEStatus(STATUS_PROCESSING_CONFIG, "Config queued");
        } else {
            updateBLEStatus(STATUS_ERROR, "JSON too large");
            Serial.println("[BLE CONFIG] ERROR: JSON too large!");
        }
    }

    void onRead(BLECharacteristic *pCharacteristic) {
        Serial.println("[BLE CONFIG] Config characteristic read");
    }
};

// File transfer characteristic callbacks - receives image files
class FileTransferCharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();
        
        if (rxValue.length() == 0) {
            return;
        }

        Serial.printf("[BLE FILE] Received %d bytes (file transfer not yet implemented)\n", (int)rxValue.length());
        // TODO: Implement file transfer with proper protocol
    }
};

// Status characteristic callbacks - reports device status
class StatusCharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onRead(BLECharacteristic *pCharacteristic) {
        if (isFileTransferring()) {
            char statusMsg[96];
            uint32_t progress = getTransferProgress();
            snprintf(statusMsg, sizeof(statusMsg), "%d:Transferring %lu bytes", 
                     STATUS_RECEIVING_FILE, progress);
            pCharacteristic->setValue(statusMsg);
        }
        Serial.println("[BLE] Status characteristic read");
    }

    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();
        Serial.printf("[BLE] Status write received: %s\n", rxValue.c_str());
        
        // Could be used for acknowledgements or control messages
        if (rxValue.find("ACK") != std::string::npos) {
            Serial.println("[BLE] Received acknowledgement");
        }
    }
};

// Time sync characteristic callbacks - receives Unix timestamp from phone
class TimeSyncCharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();
        
        if (rxValue.length() < 8) {
            Serial.println("[BLE TIME] ERROR: Invalid time format");
            updateBLEStatus(STATUS_ERROR, "Invalid time format");
            return;
        }
        
        uint64_t unixTimestamp = 0;
        
        // Try parsing with "TIME:" prefix first
        if (sscanf(rxValue.c_str(), "TIME:%llu", &unixTimestamp) == 1) {
            syncTimeFromPhone(unixTimestamp);
            Serial.printf("[BLE TIME] ✓ Time synced: %llu\n", unixTimestamp);
            updateBLEStatus(STATUS_SUCCESS, "Time synced");
        }
        // Try parsing as plain number (no prefix)
        else if (sscanf(rxValue.c_str(), "%llu", &unixTimestamp) == 1) {
            syncTimeFromPhone(unixTimestamp);
            Serial.printf("[BLE TIME] ✓ Time synced: %llu\n", unixTimestamp);
            updateBLEStatus(STATUS_SUCCESS, "Time synced");
        }
        else {
            Serial.printf("[BLE TIME] ERROR: Failed to parse timestamp from: %s\n", rxValue.c_str());
            updateBLEStatus(STATUS_ERROR, "Parse failed");
        }
    }
};

/**
 * Initialize the BLE service with all characteristics
 */
void initBLEService() {
    Serial.println("Initializing BLE Service...");
    
    // Initialize BLE device
    BLEDevice::init("CrockerDisplay");
    BLEDevice::setMTU(512);
    
    // Create BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    
    // Create BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);
    
    // Create Config Characteristic (JSON configuration)
    pConfigChar = pService->createCharacteristic(
        CONFIG_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pConfigChar->setCallbacks(new ConfigCharacteristicCallbacks());
    pConfigChar->setValue("Ready for config");
    pConfigChar->addDescriptor(new BLE2902());
    
    Serial.println("  ✓ Config Characteristic created");
    
    // Create File Transfer Characteristic (binary image files)
    pFileChar = pService->createCharacteristic(
        FILE_TRANSFER_CHAR_UUID,
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pFileChar->setCallbacks(new FileTransferCharacteristicCallbacks());
    pFileChar->addDescriptor(new BLE2902());
    
    Serial.println("  ✓ File Transfer Characteristic created");
    
    // Create Status Characteristic (device status & handshake)
    pStatusChar = pService->createCharacteristic(
        STATUS_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pStatusChar->setCallbacks(new StatusCharacteristicCallbacks());
    pStatusChar->addDescriptor(new BLE2902());
    pStatusChar->setValue("IDLE");
    
    Serial.println("  ✓ Status Characteristic created");
    
    // Create Time Sync Characteristic (receive current time from phone)
    pTimeSyncChar = pService->createCharacteristic(
        TIME_SYNC_CHAR_UUID,
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_READ
    );
    pTimeSyncChar->setCallbacks(new TimeSyncCharacteristicCallbacks());
    pTimeSyncChar->setValue("TIME:0000000000");
    
    Serial.println("  ✓ Time Sync Characteristic created");
    
    // Start service
    pService->start();
    
    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    pAdvertising->start();
    
    Serial.println("✓ BLE Service initialized and advertising");
    Serial.printf("  Service UUID: %s\n", SERVICE_UUID);
    Serial.printf("  Device Name: CrockerDisplay\n");
}

/**
 * Update the status characteristic and notify connected clients
 */
void updateBLEStatus(BLEStatus status, const char* message) {
    if (!pStatusChar) {
        return;
    }
    
    char statusMsg[128];
    if (message) {
        snprintf(statusMsg, sizeof(statusMsg), "%d:%s", status, message);
    } else {
        snprintf(statusMsg, sizeof(statusMsg), "%d", status);
    }
    
    pStatusChar->setValue(statusMsg);
    pStatusChar->notify();
    
    Serial.printf("[BLE Status] %s\n", statusMsg);
}

/**
 * Send configuration JSON over BLE (for clients to read)
 */
void sendConfigOverBLE(const char* jsonData) {
    if (!pConfigChar) {
        return;
    }
    
    pConfigChar->setValue(jsonData);
    pConfigChar->notify();
    
    Serial.println("[BLE] Config sent to connected clients");
}

/**
 * Process any pending BLE file data
 * Call this from the main loop to handle buffered data without stack overflow
 */
void processBLEFileData() {
    // Process all queued packets
    while (bleQueueHead != bleQueueTail) {
        // Get packet from queue
        BLEDataPacket* pkt = &bleQueue[bleQueueHead];
        
        // Process the buffered data in main loop context with full stack
        receiveFileChunk(pkt->data, pkt->length);
        
        // Move to next packet
        bleQueueHead = (bleQueueHead + 1) % BLE_QUEUE_SIZE;
        
        // Check if transfer is complete
        if (isFileTransferComplete()) {
            char statusMsg[64];
            snprintf(statusMsg, sizeof(statusMsg), "Complete: %s", getCurrentFilename());
            updateBLEStatus(STATUS_TRANSFER_COMPLETE, statusMsg);
            Serial.println("[BLE] File transfer complete");
        }
    }
}

/**
 * Process JSON config from main loop (safe SD card access with full stack)
 * Call this from the main loop, NOT from BLE callbacks
 */
void processBLEConfig() {
    if (!jsonConfigReady) {
        return;
    }
    
    jsonConfigReady = false;  // Clear flag immediately
    
    Serial.println("[BLE CONFIG] Processing buffered JSON from main loop...");
    
    // Save JSON to SD card (full stack available here)
    File f = SD_MMC.open("/duration.json", FILE_WRITE);
    if (!f) {
        updateBLEStatus(STATUS_ERROR, "Failed to open config file");
        Serial.println("[BLE CONFIG] ERROR: Failed to open /duration.json for writing");
        return;
    }
    
    size_t written = f.write((const uint8_t*)jsonConfigBuffer, jsonConfigLength);
    f.close();
    
    if (written == jsonConfigLength) {
        updateBLEStatus(STATUS_SUCCESS, "Config saved");
        Serial.printf("[BLE CONFIG] ✓ Success: Saved %d bytes to /duration.json\n", (int)written);
        
        // Invalidate schedule cache so next call to updateScheduleDisplay reloads from SD card
        invalidateScheduleCache();
        
        // Reload schedule from the newly saved file and update Screen 2 display
        ui_Screen2_updateScheduleDisplay();
        Serial.println("[BLE CONFIG] ✓ Updated Screen 2 with new schedule");
    } else {
        updateBLEStatus(STATUS_ERROR, "Write failed");
        Serial.printf("[BLE CONFIG] ✗ Error: Only wrote %d of %d bytes\n", (int)written, (int)jsonConfigLength);
    }
}

/**
 * Check if a BLE client is connected
 */
bool isBLEConnected() {
    return deviceConnected;
}

/**
 * Sync time from phone - called when phone sends current Unix timestamp
 * Stores in NVS for persistence across reboots
 * 
 * Logic:
 * - First sync after connection: always accept (device may have stale NVS data)
 * - Subsequent syncs: if future time, accept (device time drifted slow)
 * - Subsequent syncs: if within ±2 min, accept with note
 * - Subsequent syncs: if >2 min in past, reject (phone data suspect)
 */
void syncTimeFromPhone(uint64_t unixTimestamp) {
    Serial.println("\n======================================");
    Serial.printf("[TIME SYNC] Received Unix timestamp: %llu\n", unixTimestamp);
    
    // Validate timestamp (should be reasonable - between 2024 and 2030)
    if (unixTimestamp < 1704067200 || unixTimestamp > 1893456000) {
        Serial.println("[TIME SYNC] ✗ Timestamp out of valid range!");
        updateBLEStatus(STATUS_ERROR, "Invalid timestamp");
        return;
    }
    
    // Check for drift, but skip on first sync after connection
    if (!firstSyncSinceConnection && timeIsValid) {
        uint64_t estimatedTime = getEstimatedUnixTime();
        int64_t driftSeconds = (int64_t)unixTimestamp - (int64_t)estimatedTime;
        
        Serial.printf("[TIME SYNC] Comparing to estimated time...\n");
        
        // If future time, accept it (device time drifted backwards)
        if (driftSeconds > 0) {
            Serial.printf("[TIME SYNC] ✓ Phone time is ahead by %lld seconds - accepting (device time was slow)\n", driftSeconds);
        }
        // If past time within ±2 minutes, accept with notification
        else if (driftSeconds > -120) {
            Serial.printf("[TIME SYNC] ℹ️  Clock drift: %lld seconds - accepting (within ±2 min tolerance)\n", driftSeconds);
        }
        // If past time more than 2 minutes, REJECT (phone time is suspect)
        else {
            Serial.printf("[TIME SYNC] ✗ Phone time is %lld seconds in the past - REJECTING\n", -driftSeconds);
            Serial.printf("[TIME SYNC] Device time is more reliable, keeping current time\n");
            updateBLEStatus(STATUS_ERROR, "Phone time rejected (too old)");
            Serial.println("======================================\n");
            return;  // Don't update
        }
    } else if (firstSyncSinceConnection) {
        Serial.println("[TIME SYNC] First sync after connection - accepting (skipping drift check)");
        firstSyncSinceConnection = false;  // Mark that we've done first sync
    }
    
    // Update globals
    lastSyncedUnixTime = unixTimestamp;
    millisAtSync = millis();  // Record for reference, but won't use for calculation after reboot
    timeIsValid = true;
    
    // Set system time
    struct timeval tv;
    tv.tv_sec = (time_t)unixTimestamp;
    tv.tv_usec = 0;
    settimeofday(&tv, nullptr);
    
    // Save to NVS for persistence - ONLY save the Unix timestamp, not millis()
    nvs_handle_t nvsHandle;
    esp_err_t err = nvs_open(TIME_SYNC_NVS_NAMESPACE, NVS_READWRITE, &nvsHandle);
    if (err == ESP_OK) {
        // Write Unix timestamp and valid flag
        esp_err_t ret1 = nvs_set_u64(nvsHandle, "unix_time", unixTimestamp);
        esp_err_t ret2 = nvs_set_u8(nvsHandle, "time_valid", 1);
        
        if (ret1 != ESP_OK || ret2 != ESP_OK) {
            Serial.printf("[TIME SYNC] ✗ NVS write failed: ret1=%d, ret2=%d\n", ret1, ret2);
        } else {
            // Commit and verify
            esp_err_t commitErr = nvs_commit(nvsHandle);
            if (commitErr != ESP_OK) {
                Serial.printf("[TIME SYNC] ✗ NVS commit failed: %d\n", commitErr);
            } else {
                Serial.println("[TIME SYNC] ✓ Saved to NVS");
            }
        }
        nvs_close(nvsHandle);
    } else {
        Serial.printf("[TIME SYNC] ✗ Failed to open NVS for writing: %d\n", err);
    }
    
    // Log the synced time
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    Serial.printf("[TIME SYNC] ✓ System time set to: %04d-%02d-%02d %02d:%02d:%02d\n",
        timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
        timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    
    Serial.println("======================================\n");
    
    // Set flag to update Screen 2 display in main loop (thread-safe)
    // This avoids calling LVGL functions from BLE callback context
    updateScreen2AfterTimeSync = true;
}

/**
 * Get estimated Unix time
 * After reboot, this will be the saved Unix time (may drift by a few seconds)
 * Once phone syncs, this will be accurate
 */
uint64_t getEstimatedUnixTime() {
    if (!timeIsValid) {
        return 0;  // Time not yet synced
    }
    
    // Calculate elapsed milliseconds since this process's last sync
    // (NOT using saved millis() which would break after reboot)
    uint32_t elapsedMillis = millis() - millisAtSync;
    
    // Return estimated Unix time
    uint64_t estimatedUnixTime = lastSyncedUnixTime + (elapsedMillis / 1000);
    return estimatedUnixTime;
}

/**
 * Check if time is valid (has been synced recently)
 */
bool isTimeValid() {
    return timeIsValid;
}

/**
 * Initialize time from NVS on boot
 * Simply restores the last synced Unix timestamp
 * NOTE: Does NOT save millis() because it wraps every 49 days and breaks after reboot
 */
void initTimeFromNVS() {
    Serial.println("[TIME] Initializing time from NVS...");
    
    // Initialize NVS if not already done
    static bool nvsInitialized = false;
    if (!nvsInitialized) {
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            Serial.println("[TIME] NVS partition was truncated, erasing...");
            nvs_flash_erase();
            nvs_flash_init();
        }
        nvsInitialized = true;
    }
    
    nvs_handle_t nvsHandle;
    esp_err_t err = nvs_open(TIME_SYNC_NVS_NAMESPACE, NVS_READONLY, &nvsHandle);
    
    if (err != ESP_OK) {
        Serial.printf("[TIME] ✗ Failed to open NVS namespace: %d\n", err);
        timeIsValid = false;
        return;
    }
    
    uint64_t savedUnixTime = 0;
    uint8_t timeValid = 0;
    
    // Read Unix timestamp and valid flag
    esp_err_t ret1 = nvs_get_u64(nvsHandle, "unix_time", &savedUnixTime);
    esp_err_t ret2 = nvs_get_u8(nvsHandle, "time_valid", &timeValid);
    
    nvs_close(nvsHandle);
    
    // Check if both reads succeeded and data is valid
    if (ret1 != ESP_OK || ret2 != ESP_OK || timeValid != 1) {
        Serial.printf("[TIME] No valid time in NVS (ret1=%d, ret2=%d, valid=%d)\n", 
                     ret1, ret2, timeValid);
        timeIsValid = false;
        return;
    }
    
    // Validate the timestamp makes sense (2024-2030)
    if (savedUnixTime < 1704067200 || savedUnixTime > 1893456000) {
        Serial.printf("[TIME] ✗ Saved timestamp out of valid range: %llu\n", savedUnixTime);
        timeIsValid = false;
        return;
    }
    
    // Set system time to exactly what was saved (no millis() adjustment)
    struct timeval tv;
    tv.tv_sec = (time_t)savedUnixTime;
    tv.tv_usec = 0;
    settimeofday(&tv, nullptr);
    
    lastSyncedUnixTime = savedUnixTime;
    millisAtSync = millis();  // Just record current millis for future reference
    timeIsValid = true;
    
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    Serial.printf("[TIME] ✓ Restored from NVS: %04d-%02d-%02d %02d:%02d:%02d\n",
        timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
        timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    Serial.println("[TIME] (Note: Time will drift slowly without phone sync)\n");
}

/**
 * Periodically update NVS with current system time
 * Call from main loop to ensure saved time is never more than ~10 min old
 * This way, if device reboots, restored time is always recent
 */
void updateNVSTimeIfNeeded() {
    if (!timeIsValid) {
        return;  // No valid time to save
    }
    
    uint32_t currentMillis = millis();
    
    // Check if enough time has passed since last update (10 minutes)
    if (currentMillis - lastNVSUpdateMillis < NVS_UPDATE_INTERVAL_MS) {
        return;  // Not time to update yet
    }
    
    // Get current system time
    uint64_t currentUnixTime = (uint64_t)time(nullptr);
    
    // Save to NVS
    nvs_handle_t nvsHandle;
    esp_err_t err = nvs_open(TIME_SYNC_NVS_NAMESPACE, NVS_READWRITE, &nvsHandle);
    if (err == ESP_OK) {
        esp_err_t ret1 = nvs_set_u64(nvsHandle, "unix_time", currentUnixTime);
        esp_err_t ret2 = nvs_set_u8(nvsHandle, "time_valid", 1);
        
        if (ret1 == ESP_OK && ret2 == ESP_OK) {
            esp_err_t commitErr = nvs_commit(nvsHandle);
            if (commitErr == ESP_OK) {
                lastNVSUpdateMillis = currentMillis;
                Serial.printf("[TIME] ✓ NVS updated with current time: %llu\n", currentUnixTime);
            }
        }
        nvs_close(nvsHandle);
    }
}

/**
 * Check for 2 AM schedule sync - call from main loop
 * If it's 2 AM and we haven't synced today, request fresh schedule from app
 */
void checkAndSyncScheduleIfNeeded() {
    if (!timeIsValid) {
        return;  // Can't check time without valid time
    }
    
    static uint8_t lastSyncedDay = 255;  // Track which day we synced on
    
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    
    uint8_t currentDay = timeinfo->tm_mday;
    uint8_t currentHour = timeinfo->tm_hour;
    
    // Check if it's 2 AM (hour == 2) and we haven't synced today
    if (currentHour == 2 && currentDay != lastSyncedDay) {
        Serial.println("\n[SCHEDULE] ======================================");
        Serial.println("[SCHEDULE] 2 AM detected - requesting fresh schedule from app");
        Serial.println("[SCHEDULE] ======================================\n");
        
        // Send request via Status characteristic (app should be listening)
        if (pStatusChar && deviceConnected) {
            pStatusChar->setValue("SCHEDULE_SYNC_REQUEST");
            pStatusChar->notify();
            Serial.println("[SCHEDULE] ✓ Sync request sent to app");
        }
        
        // Mark today as synced
        lastSyncedDay = currentDay;
    }
}

/**
 * Check if Screen 2 needs update after time sync
 * Called from main loop to safely update UI
 */
bool shouldUpdateScreen2AfterTimeSync() {
    if (updateScreen2AfterTimeSync) {
        updateScreen2AfterTimeSync = false;  // Clear flag
        return true;
    }
    return false;
}
