#include "JSON_reader.h"
#include <Arduino.h>
#include "FS.h"
#include "SD_MMC.h"
#include <cctype>
#include <cstring>

//static bool parseEvents(const char* buf, readConfig* out_events, size_t max_events, size_t& out_count);

static void skip_separators(const char*& p)
{
    while (*p && (isspace((unsigned char)*p) || *p == ',')) {
        p++;
    }
}

static bool extract_u16(const char* buf, const char* key, uint16_t& out)
{
    char pattern[32];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);

    const char* p = strstr(buf, pattern);
    if (!p) return false;
    p += strlen(pattern);

    skip_separators(p);

    if (*p != ':') return false;
    p++;

    skip_separators(p);

    if (!isdigit((unsigned char)*p)) return false;

    uint64_t value = 0;
    while (*p && isdigit((unsigned char)*p)) {
        value = value * 10 + (uint64_t)(*p - '0');
        if (value > 0xFFFFFFFFULL) return false;
        p++;
    }

    out = (uint32_t)value;
    Serial.print("Int Extracted: ");
    Serial.println(out);
    return true;
}

static bool extract_string(const char* buf, const char* key, char* out, size_t outSize)
{
    char pattern[32];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);

    const char* p = strstr(buf, pattern);
    if (!p) return false;
    p += strlen(pattern);

    skip_separators(p);

    if (*p != ':') return false;
    p++;
    
    skip_separators(p);

    if (*p != '"') return false;
    p++;  // past opening quote

    size_t i = 0;
    while (*p && *p != '"') {
        if (i + 1 >= outSize) return false;
        out[i++] = *p++;
    }
    if (*p != '"') return false;

    out[i] = '\0';
    Serial.print("String Extracted: ");
    Serial.println(out);
    return true;
}

static bool parseConfig(const char* buf, readConfig& cfg)
{
    bool ok1 = extract_u16(buf, "start", cfg.start);
    bool ok2 = extract_u16(buf, "duration", cfg.duration);
    bool ok3 = extract_string(buf, "label", cfg.label, sizeof(cfg.label));
    bool ok4 = extract_string(buf, "path", cfg.path, sizeof(cfg.path));
    return ok1 && ok2 && ok3 && ok4;
}

static bool parseEvents(const char* buf, readConfig* out_events, size_t max_events, size_t& out_count)
{
    out_count = 0;
    const char* p = strstr(buf, "\"events\"");
    if (!p) return false;

    p = strchr(p, '[');
    if (!p) return false;
    p++;

    while (*p && out_count < max_events) {
        while (*p && *p != '{' && *p != ']') p++;
        if (*p == ']') break;
        if (*p != '{') break;

        const char* obj_start = p;
        int depth = 0;
        while (*p) {
            if (*p == '{') depth++;
            else if (*p == '}') {
                depth--;
                if (depth == 0) {
                    const char* obj_end = p;
                    char tmp[256];
                    size_t len = (size_t)(obj_end - obj_start + 1);
                    if (len >= sizeof(tmp)) len = sizeof(tmp) - 1;
                    memcpy(tmp, obj_start, len);
                    tmp[len] = '\0';

                    readConfig cfg;
                    if (parseConfig(tmp, cfg)) {
                        out_events[out_count++] = cfg;
                    }
                    p++;
                    break;
                }
            }
            p++;
        }
        if (depth != 0) break;
    }

    return out_count > 0;
}

// Add this helper function to initialize the JSON file
static bool initializeJSONFile()
{
    if (SD_MMC.cardType() == CARD_NONE) {
        Serial.println("SD_MMC not mounted");
        return false;
    }

    // Check if file exists
    if (!SD_MMC.exists("/duration.json")) {
        Serial.println("duration.json not found, creating default file...");
        
        File f = SD_MMC.open("/duration.json", FILE_WRITE);
        if (!f) {
            Serial.println("Failed to create duration.json");
            return false;
        }

        // Write a default empty events structure
        const char* defaultJSON = R"({
  "events": []
})";
        
        f.write((const uint8_t*)defaultJSON, strlen(defaultJSON));
        f.close();
        Serial.println("Default duration.json created successfully");
        return true;
    }
    
    return true;
}

bool readJSON(readConfig& cfg)
{
    // Initialize file if needed
    if (!initializeJSONFile()) {
        return false;
    }

    if (SD_MMC.cardType() == CARD_NONE) {
        Serial.println("SD_MMC not mounted");
        return false;
    }

    File f = SD_MMC.open("/duration.json", FILE_READ);
    if (!f) {
        Serial.println("Failed to open duration.json");
        return false;
    }

    char buf[2048];
    size_t n = f.readBytes(buf, sizeof(buf) - 1);
    buf[n] = '\0';
    f.close();

    size_t count = 0;
    return parseEvents(buf, &cfg, 1, count);
}

bool readJSONQueue(readConfig* out_events, size_t max_events, size_t& out_count)
{
    // Initialize file if needed
    if (!initializeJSONFile()) {
        out_count = 0;
        return false;
    }

    out_count = 0;
    if (SD_MMC.cardType() == CARD_NONE) {
        Serial.println("SD_MMC not mounted");
        return false;
    }

    File f = SD_MMC.open("/duration.json", FILE_READ);
    if (!f) {
        Serial.println("Failed to open duration.json");
        return false;
    }

    char buf[4096];
    size_t n = f.readBytes(buf, sizeof(buf) - 1);
    buf[n] = '\0';
    f.close();

    return parseEvents(buf, out_events, max_events, out_count);
}
