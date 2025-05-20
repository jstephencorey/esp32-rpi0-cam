#include "sdcard.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"
#include <sys/stat.h>
#include <sys/types.h>

static const char *TAG = "SDCARD";

void init_sdcard(void) {
    esp_err_t ret;

    ESP_LOGI(TAG, "Initializing SD card...");

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 0
    };

    sdmmc_card_t *card;
    const char mount_point[] = "/sdcard";

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    // For boards without card detect pin, set to 1
    gpio_set_pull_mode(15, GPIO_PULLUP_ONLY); // CMD
    gpio_set_pull_mode(2, GPIO_PULLUP_ONLY);  // D0
    gpio_set_pull_mode(4, GPIO_PULLUP_ONLY);  // D1
    gpio_set_pull_mode(12, GPIO_PULLUP_ONLY); // D2
    gpio_set_pull_mode(13, GPIO_PULLUP_ONLY); // D3
    gpio_set_pull_mode(14, GPIO_PULLUP_ONLY); // CLK

    ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount filesystem. Error: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "SD card mounted.");
    sdmmc_card_print_info(stdout, card);

    struct stat st;
    if (stat("/sdcard", &st) != 0) {
        ESP_LOGE(TAG, "/sdcard does not exist or is not mounted");
    } else {
        ESP_LOGI(TAG, "/sdcard is mounted and ready");
    }
}

esp_err_t create_directory(const char *path) {
    ESP_LOGI(TAG, "Making directory: %s", path);
    struct stat st;
    if (stat("/sdcard", &st) != 0) {
        ESP_LOGE(TAG, "/sdcard not accessible before mkdir. errno=%d: %s", errno, strerror(errno));
    }

    if (mkdir(path, 0777) == 0) {
        return ESP_OK;
    } 
    else if (errno == EEXIST) {
        ESP_LOGW(TAG, "Directory already exists: %s", path);
        return ESP_OK;
    }
    else {
        ESP_LOGE(TAG, "mkdir failed: %s", path);
        perror("mkdir");
        return ESP_FAIL;
    }
}

void write_text_file(const char *path, const char *text) {
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        printf("Failed to open file for writing: %s\n", path);
        return;
    }
    fprintf(f, "%s\n", text);
    fclose(f);
    printf("File written: %s\n", path);
}