# esp32-surveilence-cam
Project for turning an esp32-cam into a surveilence camera

Note for setup:
Copy secrets.example.h to secrets.h and fill in your actual SSID, password, and upload endpoint. The .h file is ignored by Git.

Make a .env file in the flask-api folder with IMMICH_API_KEY, IMMICH_UPLOAD_URL, and SECRET_KEY