#include "ble_file_transfer.h"
#include "SD_MMC.h"
#include <Arduino.h>

// File transfer state
static FileTransferState transferState = {0};
static File currentFile;

/**
 * Initialize a new file transfer
 * Call this when receiving START: control message
 */
void initFileTransfer(const char* filename, uint32_t fileSize) {
    // Close any existing file
    if (currentFile) {
        currentFile.close();
    }
    
    strncpy(transferState.filename, filename, sizeof(transferState.filename) - 1);
    transferState.fileSize = fileSize;
    transferState.bytesReceived = 0;
    transferState.checksum = 0;
    transferState.isTransferring = true;
    
    // Ensure SD card is mounted
    if (SD_MMC.cardType() == CARD_NONE) {
        Serial.println("[FILE TRANSFER] ERROR: SD card not mounted");
        transferState.isTransferring = false;
        return;
    }
    
    // Create file path - store in /lvgl_images directory
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "/lvgl_images/%s", filename);
    
    // Create directory if it doesn't exist
    if (!SD_MMC.exists("/lvgl_images")) {
        Serial.println("[FILE TRANSFER] Creating /lvgl_images directory");
        // Note: ESP32 SD_MMC doesn't have mkdir, so we just try to open
    }
    
    // Open file for writing
    currentFile = SD_MMC.open(filepath, FILE_WRITE);
    if (!currentFile) {
        Serial.printf("[FILE TRANSFER] ERROR: Failed to open %s for writing\n", filepath);
        transferState.isTransferring = false;
        return;
    }
    
    Serial.printf("[FILE TRANSFER] Started: %s (%lu bytes)\n", filepath, fileSize);
    Serial.printf("[FILE TRANSFER] Waiting for %lu bytes...\n", fileSize);
}

/**
 * Receive a chunk of file data
 * Call this for each data packet from BLE
 */
void receiveFileChunk(const uint8_t* data, size_t length) {
    if (!transferState.isTransferring || !currentFile) {
        Serial.println("[FILE TRANSFER] ERROR: Transfer not active or file not open");
        return;
    }
    
    // Write data to file
    size_t written = currentFile.write(data, length);
    if (written != length) {
        Serial.printf("[FILE TRANSFER] WARNING: Only wrote %lu of %lu bytes\n", written, length);
    }
    
    transferState.bytesReceived += written;
    
    // Simple checksum (XOR of all bytes)
    for (size_t i = 0; i < length; i++) {
        transferState.checksum ^= data[i];
    }
    
    // Log progress every 5KB or at the end
    if (transferState.bytesReceived % 5120 == 0 || transferState.bytesReceived >= transferState.fileSize) {
        float progress = (float)transferState.bytesReceived / transferState.fileSize * 100.0f;
        Serial.printf("[FILE TRANSFER] Progress: %lu/%lu bytes (%.1f%%)\n", 
            transferState.bytesReceived, 
            transferState.fileSize,
            progress);
    }
}

/**
 * Check if file transfer is complete
 */
bool isFileTransferComplete() {
    if (!transferState.isTransferring) {
        return false;
    }
    
    if (transferState.bytesReceived >= transferState.fileSize) {
        // Close the file
        if (currentFile) {
            currentFile.close();
        }
        
        transferState.isTransferring = false;
        
        // Verify file was written
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "/lvgl_images/%s", transferState.filename);
        
        File verifyFile = SD_MMC.open(filepath);
        if (verifyFile) {
            uint32_t fileSize = verifyFile.size();
            verifyFile.close();
            Serial.printf("[FILE TRANSFER] ✓ File verified on SD card: %s (%lu bytes)\n", 
                transferState.filename, fileSize);
        } else {
            Serial.printf("[FILE TRANSFER] ✗ ERROR: Could not verify file on SD card: %s\n", 
                transferState.filename);
        }
        
        Serial.printf("[FILE TRANSFER] Complete: %s\n", transferState.filename);
        Serial.printf("[FILE TRANSFER] Total bytes: %lu\n", transferState.bytesReceived);
        Serial.printf("[FILE TRANSFER] Checksum: 0x%08lx\n", transferState.checksum);
        
        return true;
    }
    
    return false;
}

/**
 * Check if a file transfer is currently in progress
 */
bool isFileTransferring() {
    return transferState.isTransferring;
}

/**
 * Get transfer progress in bytes
 */
uint32_t getTransferProgress() {
    return transferState.bytesReceived;
}

/**
 * Get the current filename being transferred
 */
const char* getCurrentFilename() {
    return transferState.filename;
}

/**
 * Cancel the current file transfer
 */
void cancelFileTransfer() {
    if (currentFile) {
        currentFile.close();
    }
    
    transferState.isTransferring = false;
    transferState.bytesReceived = 0;
    
    Serial.printf("[FILE TRANSFER] Cancelled: %s\n", transferState.filename);
}

/**
 * Get the checksum of the transferred file
 */
uint32_t getFileChecksum() {
    return transferState.checksum;
}
