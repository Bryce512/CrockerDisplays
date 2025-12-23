#include <Arduino.h>
#include "FS.h"
#include "SD_MMC.h"
#include "JSON_writer.h"

static bool writeDurationJson_SDMMC(const writeConfig& cfg)
{
    const char* path = "/duration.json";

    // Make sure SD_MMC is still mounted
    if (SD_MMC.cardType() == CARD_NONE) {
        Serial.println("SD_MMC not mounted at write time");
        return false;
    }

    // Overwrite cleanly
    if (SD_MMC.exists(path)) {
        SD_MMC.remove(path);
    }

    File f = SD_MMC.open(path, FILE_WRITE);
    if (!f) {
        Serial.println("Failed to open /duration.json for writing (SD_MMC)");
        return false;
    }

    f.print("{\n");
    f.print("  \"start\": ");
    f.print(cfg.start);
    f.print(",\n");

    f.print("  \"duration\": ");
    f.print(cfg.duration);
    f.print(",\n");

    f.print("  \"label\": \"");
    f.print(cfg.label);
    f.print("\",\n");

    f.print("  \"path\": \"");
    f.print(cfg.path);
    f.print("\"\n");

    f.print("}\n");
    f.close();

    return true;
}

void writeJSON(const writeConfig& cfg)
{
    if (!writeDurationJson_SDMMC(cfg)) {
        Serial.println("Failed to create JSON");
    } else {
        Serial.println("JSON file created successfully");
    }
}
