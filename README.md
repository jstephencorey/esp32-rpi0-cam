# esp32-rpi0-cam
Project for turning an esp32-cam into a surveilence camera, with a raspberry pi zero 2 w as a mobile backend/server, and immich as the ultimate destination for these videos

Note for setup:
Copy secrets.example.h to secrets.h and fill in your actual SSID, password, and upload endpoint. The .h file is ignored by Git.

Make a .env file in the flask-api folder with IMMICH_API_KEY, IMMICH_UPLOAD_URL, and SECRET_KEY

## To run:
### Esp32:
 - Open a shell for ESP-IDF 5.4
 - Plug in the esp32 camera to the computer
 - Navigate to the folder (in my case, `S:\Dev\esp32-rpi0-cam\esp32-code`)
 - run `idf.py fullclean`
 - run `idf.py set-target esp32`
 - run `idf.py build`
 - then run `python -m esptool --chip esp32 -b 460800 --before default_reset --after hard_reset write_flash --flash_mode dio --flash_size 2MB --flash_freq 40m 0x1000 build\bootloader\bootloader.bin 0x8000 build\partition_table\partition-table.bin 0x10000 build\esp32-surveillence-cam.bin`
 - Monitor the logs with `idf.py -p COM3 monitor` assuming you are on COM3
 - Or build, flash and monitor with `idf.py -p COM3 flash monitor`

#### Notes: 
    You may need to increase the CONFIG_ESP_MAIN_TASK_STACK_SIZE in sdkconfig to 16384 (or change it in menuconfig)
    You also may need to change the FATFS settings through `idf.py menuconfig` then going to `Component config  →  FAT Filesystem` support and enabling long filename support on heap (or stack?)

### To set up the RPI zero 2 w:
#### Setup the OS:
 - Download Raspberry Pi Imager
 - Choose OS:Raspberry Pi OS Lite (32-bit)
 - Choose storage: your SD card
 - Before flashing, click gear icon (Advanced Settings):
     - Set hostname (e.g., pi-flask.local)
     - Enable SSH
     - Set username/password
     - Configure Wi-Fi
 - Flash the SD card and insert into the Pi Zero 2 W

SSH into the rpi
run the following two commands:
`sudo apt update && sudo apt upgrade -y`
`sudo apt install python3 python3-pip ffmpeg git -y`
(optional:) `sudo apt install vim -y`

#### Setup the wifi hotspot
This will set up the RPI zero w 2 to be a constant hotspot, and have the endpoint start every time:
`git clone <this repo>`
`cd <this repo>`
`sudo ./setup_pi_hotspot_api.sh`


## Future plans:
  - ESP32Cam code:
    - Delete photos after successfully uploading them
    - Upload in chunks?
    - Background upload?/sensible handling of connection failures
  - RPI zero 2 W:
    - Run the flask endpoint
    - Change the flask endpoint to save things locally first/as well. 
    - Eventually:
      - make own wifi signal for ESP32Cam to latch on to
      - Save locally mostly
      - Periodically switch to a different wifi (if available) to upload the videos to immich
      - delete local videos upon success
