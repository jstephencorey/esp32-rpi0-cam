# esp32-rpi0-cam
Project for turning an esp32-cam into a surveilence camera, with a raspberry pi zero 2 w as a mobile backend/server, and immich as the ultimate destination for these videos

Note for setup:
Copy secrets.example.h to secrets.h and fill in your actual SSID, password, and upload endpoint. The .h file is ignored by Git.

Make a .env file in the flask-api folder with IMMICH_API_KEY, IMMICH_UPLOAD_URL, and SECRET_KEY

## To run:
### Esp32:
Open a shell for ESP-IDF 5.4
Plug in the esp32 camera to the computer
Navigate to the folder (in my case, `S:\Dev\esp32-rpi0-cam\esp32-code`)
run `idf.py set-target esp32`
run `idf.py build`?

### To set up the RPI zero 2 w:
#### Setup the OS:
Download Raspberry Pi Imager
Choose OS:Raspberry Pi OS Lite (32-bit)
Choose storage: your SD card
Before flashing, click gear icon (Advanced Settings):
    Set hostname (e.g., pi-flask.local)
    Enable SSH
    Set username/password
    Configure Wi-Fi
Flash the SD card and insert into the Pi Zero 2 W

SSH into the rpi
