#ifndef BLE_FILE_TRANSFER_H
#define BLE_FILE_TRANSFER_H

#include <Arduino.h>
#include <cstdint>

typedef struct {
    char filename[128];
    uint32_t fileSize;
    uint32_t bytesReceived;
    uint32_t checksum;
    bool isTransferring;
} FileTransferState;

// File transfer control functions
void initFileTransfer(const char* filename, uint32_t fileSize);
void receiveFileChunk(const uint8_t* data, size_t length);
bool isFileTransferComplete();
bool isFileTransferring();
uint32_t getTransferProgress();
const char* getCurrentFilename();
void cancelFileTransfer();
uint32_t getFileChecksum();

#endif
