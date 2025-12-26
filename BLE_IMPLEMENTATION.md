# BLE Implementation Guide - CrockerDisplay

## Overview

This implementation provides a complete BLE service for your CrockerDisplay with three characteristics:

1. **Config Characteristic** - Receive/send JSON duration configuration
2. **File Transfer Characteristic** - Receive LVGL binary image files (.bin)
3. **Status Characteristic** - Device status updates and handshake

## Architecture

```
BLE Service (550e8400-e29b-41d4-a716-446655440000)
├── Config Char (550e8400-e29b-41d4-a716-446655440001)
│   └── Receives JSON config, saves to SD card
├── File Transfer Char (550e8400-e29b-41d4-a716-446655440002)
│   └── Receives image chunks, writes to SD card
└── Status Char (550e8400-e29b-41d4-a716-446655440003)
    └── Reports transfer progress and device state
```

## Device Behavior

### Initialization
- BLE service starts advertising as "CrockerDisplay"
- All characteristics are ready to receive data
- SD card must be mounted before file transfers

### JSON Configuration Transfer
1. App writes JSON to **Config Characteristic**
   ```json
   {
     "events": [
       {"start": 0, "duration": 300, "label": "Morning"},
       {"start": 300, "duration": 600, "label": "Afternoon"}
     ]
   }
   ```
2. Device parses and saves to `/duration.json`
3. Device updates **Status Characteristic** with result:
   - `3:Config saved` (success)
   - `4:Failed to save config` (error)

### LVGL Image File Transfer

#### Protocol

**Step 1: Send START command**
```
Characteristic: File Transfer
Value: "START:image.bin:1048576"
```
- Filename: `image.bin` (saved to `/lvgl_images/image.bin`)
- File size: `1048576` bytes

**Step 2: Send file chunks**
```
Characteristic: File Transfer
Value: [binary data, up to 480 bytes per chunk]
```
- Device receives chunks sequentially
- Each chunk updates transfer progress
- No need to wait between chunks (device handles buffering)

**Step 3: Monitor progress (optional)**
```
Read from Status Characteristic
Response: "2:Transferring 204800 bytes"
```

**Step 4: Transfer complete**
- Device automatically closes file when `bytesReceived >= fileSize`
- **Status Characteristic** updates with: `5:Complete: image.bin`
- Checksum calculated (XOR of all bytes)

**Step 5: Optional cancellation**
```
Characteristic: File Transfer
Value: "CANCEL:"
```
- Stops transfer and closes file

## Status Codes

| Code | Meaning | Notes |
|------|---------|-------|
| 0 | IDLE | Device ready, no transfers |
| 1 | RECEIVING_JSON | Processing config |
| 2 | RECEIVING_FILE | File transfer in progress |
| 3 | SUCCESS | Last operation successful |
| 4 | ERROR | Last operation failed |
| 5 | TRANSFER_COMPLETE | File transfer finished |

## File Storage

```
SD Card Structure:
/
├── duration.json          (JSON configuration)
├── lvgl_images/
│   ├── image1.bin         (LVGL binary image format)
│   ├── image2.bin
│   └── ...
└── [other files]
```

## LVGL Binary Image Format

The `.bin` files must be in LVGL's binary image format. Convert using:

1. **Online Tool**: https://lvgl.io/tools/imageconverter
2. **Steps**:
   - Upload JPG/PNG image
   - Select Output Format: "Binary" (.bin)
   - Set Color Format: RGB565 (recommended) or RGB888
   - Download `.bin` file

## Example Usage in LVGL

```c
// After transferring image1.bin to device
lv_obj_t * img = lv_img_create(parent_screen);
lv_img_set_src(img, "/lvgl_images/image1.bin");
lv_obj_set_pos(img, 0, 0);
```

## BLE Testing with NRF Connect

### Setup
1. Download nRF Connect app (iOS/Android)
2. Open app, scan for "CrockerDisplay"
3. Connect to device

### Send Config
1. Locate "Config Char" (550e8400-e29b-41d4-a716-446655440001)
2. Tap "Write"
3. Paste JSON text
4. Send
5. Check "Status Char" for response

### Send Image File
1. Locate "File Transfer Char" (550e8400-e29b-41d4-a716-446655440002)
2. First write: `START:image.bin:1048576`
3. Then write the binary file in 480-byte chunks
4. Monitor "Status Char" for progress

## Debugging

Enable serial monitor (115200 baud) to see:
```
[BLE] Config characteristic read
[BLE Status] 1:Processing config
[BLE] Config saved to connected clients
[FILE TRANSFER] Started: /lvgl_images/image.bin (1048576 bytes)
[FILE TRANSFER] Progress: 204800/1048576 bytes (19.5%)
[FILE TRANSFER] Complete: image.bin
```

## Known Limitations

1. **BLE MTU Size**: 512 bytes (actual data ~480 bytes per chunk)
2. **File Size**: Limited by SD card size (typically 32GB+)
3. **Transfer Speed**: ~60-100 KB/s depending on radio conditions
4. **Checksum**: Simple XOR (adequate for testing, consider CRC32 for production)

## Future Enhancements

- [ ] CRC32 checksum validation
- [ ] Resume interrupted transfers
- [ ] Batch JSON + image transfers
- [ ] Over-the-air firmware updates
- [ ] React Native app integration

## API Reference

### ble_service.h
```cpp
void initBLEService();
void updateBLEStatus(BLEStatus status, const char* message = nullptr);
void sendConfigOverBLE(const char* jsonData);
bool isBLEConnected();
```

### ble_file_transfer.h
```cpp
void initFileTransfer(const char* filename, uint32_t fileSize);
void receiveFileChunk(const uint8_t* data, size_t length);
bool isFileTransferComplete();
bool isFileTransferring();
uint32_t getTransferProgress();
const char* getCurrentFilename();
void cancelFileTransfer();
uint32_t getFileChecksum();
```

## Troubleshooting

**Q: BLE won't advertise**
- Check if `initBLEService()` is being called
- Verify BLE library is installed in platformio.ini
- Check serial output for errors

**Q: File transfers timeout**
- Increase BLE MTU if device supports it
- Reduce chunk size to 256 bytes for slower connections
- Check SD card speed

**Q: JSON not saving**
- Verify SD card is mounted (`SD_MMC mounted` in serial)
- Check SD card has free space
- Verify JSON format is valid

**Q: Images not displaying**
- Confirm file was transferred completely (Status = 5)
- Verify image.bin is in LVGL binary format
- Check file path in lv_img_set_src() matches transfer filename
