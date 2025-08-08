#include <stdio.h>
#include <string.h>
#include "esp_random.h"

#include "uuid.h"

void generate_uuid_v4(uint8_t uuid[16]) {
    for (int i = 0; i < 16; i += 4) {
        uint32_t r = esp_random();
        uuid[i] = (r >> 24) & 0xFF;
        uuid[i+1] = (r >> 16) & 0xFF;
        uuid[i+2] = (r >> 8) & 0xFF;
        uuid[i+3] = r & 0xFF;
    }
    uuid[6] = (uuid[6] & 0x0F) | 0x40;  // Version 4
    uuid[8] = (uuid[8] & 0x3F) | 0x80;  // Variant 10xxxxxx
}

void uuid_to_str(const uint8_t uuid[16], char* str_out) {
    sprintf(str_out,
        "%02x%02x%02x%02x-"
        "%02x%02x-"
        "%02x%02x-"
        "%02x%02x-"
        "%02x%02x%02x%02x%02x%02x",
        uuid[0], uuid[1], uuid[2], uuid[3],
        uuid[4], uuid[5],
        uuid[6], uuid[7],
        uuid[8], uuid[9],
        uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
}

void get_uuid_str(char *out_str) {
    uint8_t uuid[16];
    generate_uuid_v4(uuid);
    uuid_to_str(uuid, out_str);
}