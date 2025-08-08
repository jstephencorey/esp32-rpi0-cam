#include "uploader.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include "secrets.h"

#define TAG "UPLOADER"
#define BOUNDARY "Esp32CamBoundary"
#define MAX_IMAGES 640

static esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    return ESP_OK;
}

static esp_err_t write_file_part(esp_http_client_handle_t client, const char *filepath, const char *filename) {
    char header[512];
    snprintf(header, sizeof(header),
        "--%s\r\n"
        "Content-Disposition: form-data; name=\"images\"; filename=\"%s\"\r\n"
        "Content-Type: image/jpeg\r\n\r\n",
        BOUNDARY, filename);

    if (esp_http_client_write(client, header, strlen(header)) < 0) {
        ESP_LOGE(TAG, "Failed to write header for %s", filename);
        return ESP_FAIL;
    }

    FILE *f = fopen(filepath, "rb");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open %s", filepath);
        return ESP_FAIL;
    }

    char buffer[1024];
    size_t read_bytes;
    while ((read_bytes = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        if (esp_http_client_write(client, buffer, read_bytes) < 0) {
            ESP_LOGE(TAG, "Failed to write image data for %s", filename);
            fclose(f);
            return ESP_FAIL;
        }
    }

    fclose(f);
    esp_http_client_write(client, "\r\n", 2);  // Close part
    return ESP_OK;
}

esp_err_t upload_photos_from_dir(const char *dir_path) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        ESP_LOGE(TAG, "Could not open directory: %s", dir_path);
        return ESP_FAIL;
    }

    // Collect file paths
    char *filepaths[MAX_IMAGES];
    int count = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL && count < MAX_IMAGES) {
        if (entry->d_type == DT_REG && strstr(entry->d_name, ".JPG")) {
            char *path = malloc(PATH_MAX);
            if (!path) break;
            snprintf(path, PATH_MAX, "%s/%s", dir_path, entry->d_name);
            filepaths[count++] = path;
        }
    }
    closedir(dir);

    if (count == 0) {
        ESP_LOGW(TAG, "No .JPG files found.");
        return ESP_OK;
    }
    ESP_LOGI(TAG, "Num Photos found and sending: %d", count);


    // Calculate Content-Length
    size_t total_size = 0;

    // Device ID part
    char device_part[256];
    snprintf(device_part, sizeof(device_part),
        "--%s\r\n"
        "Content-Disposition: form-data; name=\"deviceId\"\r\n\r\n"
        "%s\r\n",
        BOUNDARY, DEVICE_ID);
    total_size += strlen(device_part);

    // Image parts
    for (int i = 0; i < count; ++i) {
        struct stat st;
        if (stat(filepaths[i], &st) != 0) continue;

        const char *filename = strrchr(filepaths[i], '/');
        filename = filename ? filename + 1 : filepaths[i];

        char header[512];
        snprintf(header, sizeof(header),
            "--%s\r\n"
            "Content-Disposition: form-data; name=\"images\"; filename=\"%s\"\r\n"
            "Content-Type: image/jpeg\r\n\r\n",
            BOUNDARY, filename);
        total_size += strlen(header);
        total_size += st.st_size;
        total_size += 2;  // \r\n after file
    }

    total_size += strlen("--" BOUNDARY "--\r\n");

    ESP_LOGI(TAG, "total size of sent data: %d", total_size);

    // Setup HTTP
    esp_http_client_config_t config = {
        .url = UPLOAD_URL,
        .method = HTTP_METHOD_POST,
        .event_handler = _http_event_handler,
        .timeout_ms = 100000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(TAG, "Failed to init HTTP client");
        goto cleanup;
    }

    char content_type[64];
    snprintf(content_type, sizeof(content_type), "multipart/form-data; boundary=%s", BOUNDARY);
    esp_http_client_set_header(client, "Content-Type", content_type);

    if (esp_http_client_open(client, total_size) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection");
        esp_http_client_cleanup(client);
        goto cleanup;
    }

    // Write deviceId field
    esp_http_client_write(client, device_part, strlen(device_part));

    // Write each image
    for (int i = 0; i < count; ++i) {
        const char *filename = strrchr(filepaths[i], '/');
        filename = filename ? filename + 1 : filepaths[i];

        if (write_file_part(client, filepaths[i], filename) != ESP_OK) {
            ESP_LOGW(TAG, "Skipping file %s", filepaths[i]);
        }
    }

    // Final boundary
    const char *end = "--" BOUNDARY "--\r\n";
    esp_http_client_write(client, end, strlen(end));

    // Finish
    int status = esp_http_client_fetch_headers(client);
    char buf[128];
    int read_len = esp_http_client_read(client, buf, sizeof(buf) - 1);
    if (read_len > 0) {
        buf[read_len] = '\0';
        ESP_LOGI(TAG, "Response: %s", buf);
    }

    if (status >= 0) {
        int code = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "Upload finished with status code: %d", code);
    } else {
        ESP_LOGE(TAG, "Failed to fetch response");
    }

    esp_http_client_cleanup(client);

cleanup:
    for (int i = 0; i < count; ++i) {
        free(filepaths[i]);
    }
    return ESP_OK;
}
