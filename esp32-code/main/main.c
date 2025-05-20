#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "sdcard.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "camera.h"



static const char *TAG = "MAIN";

void app_main(void) {
    ESP_LOGI(TAG, "Booting up...");

    init_sdcard();
    init_camera();

    // Seed RNG from a basic source (time is fine for now, RTC/persistent values better for real apps)
    srand((unsigned int)esp_timer_get_time());

    // Generate a random directory name
    char dir_name[32];
    snprintf(dir_name, sizeof(dir_name), "/sdcard/c_%d", rand() % 10000);

    esp_err_t res = create_directory(dir_name);

    if (res == ESP_OK) {
        ESP_LOGI(TAG, "Created directory: %s", dir_name);
    } else {
        ESP_LOGE(TAG, "Failed to create directory");
    }

    char filepath[64];
    snprintf(filepath, sizeof(filepath), "%s/photo1.jpg", dir_name);
    capture_and_save_photo(filepath);
}
