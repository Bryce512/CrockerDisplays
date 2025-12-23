#include "JSON_reader.h"
#include <Arduino.h>
#include "FS.h"
#include "SD_MMC.h"
#include <cctype>
#include <cstring>

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


bool readJSON(readConfig& cfg)
{
    if (SD_MMC.cardType() == CARD_NONE) {
        Serial.println("SD_MMC not mounted");
        return false;
    }

    File f = SD_MMC.open("/duration.json", FILE_READ);
    if (!f) {
        Serial.println("Failed to open duration.json");
        return false;
    }

    char buf[256];
    size_t n = f.readBytes(buf, sizeof(buf) - 1);
    buf[n] = '\0';
    f.close();

    return parseConfig(buf, cfg);
}