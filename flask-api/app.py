from flask import Flask, request
import os, tempfile, subprocess, shutil
import requests
from dotenv import load_dotenv
from datetime import datetime
import pytz

load_dotenv()

IMMICH_API_KEY = os.getenv("IMMICH_API_KEY")
IMMICH_UPLOAD_URL = os.getenv("IMMICH_UPLOAD_URL")

app = Flask(__name__)

def get_iso8601_zoned_timestamp():
    tz = pytz.timezone(os.getenv("TIMEZONE", "America/Denver"))  # fallback to Mountain Time
    now = datetime.now(tz)
    return now.isoformat()

@app.route('/test', methods=['GET'])
def test_endpoint():
    return "hello world"

@app.route('/upload', methods=['POST'])
def receive_images():
    temp_dir = tempfile.mkdtemp()
    filenames = []

    try:
        # Expect deviceId in form data
        device_id = request.form.get("deviceId")
        if not device_id:
            return {"status": "error", "message": "Missing deviceId"}, 400

        for i, file in enumerate(request.files.getlist("images")):
            path = os.path.join(temp_dir, f"frame_{i:03}.jpg")
            file.save(path)
            filenames.append(path)

        video_path = os.path.join(temp_dir, "output.mp4")
        subprocess.run([
            "ffmpeg", "-y",
            "-framerate", "10",
            "-i", os.path.join(temp_dir, "frame_%03d.jpg"),
            "-c:v", "libx264",
            "-pix_fmt", "yuv420p",
            video_path
        ], check=True)

        # Upload video to Immich
        with open(video_path, 'rb') as f:
            headers = {'x-api-key': IMMICH_API_KEY}
            files = {'assetData': f}
            timestamp = get_iso8601_zoned_timestamp()
            data = {
                'deviceId': device_id,
                'fileCreatedAt': timestamp,
                'fileModifiedAt': timestamp
            }
            response = requests.post(IMMICH_UPLOAD_URL, files=files, data=data, headers=headers)

        return {"status": "success", "immich_response": response.json()}, response.status_code
    except Exception as e:
        return {"status": "error", "message": str(e)}, 500
    finally:
        shutil.rmtree(temp_dir)
