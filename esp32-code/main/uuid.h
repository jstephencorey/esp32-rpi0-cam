#pragma once

void generate_uuid_v4(uint8_t uuid[16]);

void uuid_to_str(const uint8_t uuid[16], char* str_out);

void get_uuid_str(char *out_str);