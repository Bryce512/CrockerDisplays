#ifndef BLE_SERVICE_H
#define BLE_SERVICE_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// BLE UUIDs - Custom service for CrockerDisplay
#define SERVICE_UUID           "550e8400-e29b-41d4-a716-446655440000"
#define CONFIG_CHAR_UUID       "550e8400-e29b-41d4-a716-446655440001"
#define FILE_TRANSFER_CHAR_UUID "550e8400-e29b-41d4-a716-446655440002"
#define STATUS_CHAR_UUID       "550e8400-e29b-41d4-a716-446655440003"
#define TIME_SYNC_CHAR_UUID    "550e8400-e29b-41d4-a716-446655440004"

// BLE MTU size (typically 512 bytes, minus overhead leaves ~480 for payload)
#define BLE_FILE_CHUNK_SIZE 480

typedef enum {
    STATUS_IDLE = 0,
    STATUS_RECEIVING_JSON = 1,
    STATUS_RECEIVING_FILE = 2,
    STATUS_SUCCESS = 3,
    STATUS_ERROR = 4,
    STATUS_TRANSFER_COMPLETE = 5,
    STATUS_PROCESSING_CONFIG = 6
} BLEStatus;

// Main BLE functions
void initBLEService();
void updateBLEStatus(BLEStatus status, const char* message = nullptr);
void sendConfigOverBLE(const char* jsonData);
void processBLEConfig();    // Call from main loop to process JSON config (safe with full stack)
void processBLEFileData();  // Call from main loop to process file transfers
void checkAndSyncScheduleIfNeeded();  // Call from main loop to check for 2 AM sync
bool isBLEConnected();

// Time sync functions
void initTimeFromNVS();  // Call from system_state_init()
void syncTimeFromPhone(uint64_t unixTimestamp);  // Phone sends current time
void updateNVSTimeIfNeeded();  // Call from main loop to keep NVS time fresh
uint64_t getEstimatedUnixTime();  // Get current time (from system clock + millis elapsed)
bool isTimeValid();  // Check if time has been synced recently
void checkAndSyncScheduleIfNeeded();  // Call from main loop to check for 2 AM sync
bool shouldUpdateScreen2AfterTimeSync();  // Check if Screen 2 needs update after time change

#endif
