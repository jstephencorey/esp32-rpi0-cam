from flask import Flask, request
import os, tempfile, subprocess, shutil, logging
from datetime import datetime
import pytz
import uuid

# Flask app setup
app = Flask(__name__)
app.config['MAX_CONTENT_LENGTH'] = 200 * 1024 * 1024  # allow larger uploads

# Setup logging
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    handlers=[
        logging.FileHandler("app.log"),
        logging.StreamHandler()
    ]
)

# Directory to store final videos
VIDEO_DIR = os.path.abspath("videos")
os.makedirs(VIDEO_DIR, exist_ok=True)

def get_iso8601_zoned_timestamp():
    tz = pytz.timezone("America/Denver")  # adjust if you want another timezone
    now = datetime.now(tz)
    return now.isoformat()

@app.route('/test', methods=['GET'])
def test_endpoint():
    logging.info("Received GET /test request")
    return "hello world"

@app.route('/upload', methods=['POST'])
def receive_images():
    logging.info("Received POST /upload request")
    temp_dir = tempfile.mkdtemp()
    filenames = []

    try:
        # Expect deviceId in form data
        device_id = request.form.get("deviceId", "unknown_device")
        logging.info(f"Device ID: {device_id}")

        images = request.files.getlist("images")
        if not images:
            logging.warning("No images received")
            return {"status": "error", "message": "No images provided"}, 400

        logging.info(f"Number of images received: {len(images)}")

        for i, file in enumerate(images):
            path = os.path.join(temp_dir, f"frame_{i:03}.jpg")
            file.save(path)
            filenames.append(path)
            logging.debug(f"Saved frame: {path}")

        # Final video name (timestamp + device_id)
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        video_name = f"{device_id}_{timestamp}_{uuid.uuid4().hex[:8]}.mp4"
        video_path = os.path.join(VIDEO_DIR, video_name)

        logging.info(f"Generating video at: {video_path}")

        result = subprocess.run([
            "ffmpeg", "-y",
            "-framerate", "10",  # adjust fps if needed
            "-i", os.path.join(temp_dir, "frame_%03d.jpg"),
            "-c:v", "libx264",   # x264 is lighter than x265 on the Pi
            "-pix_fmt", "yuv420p",
            video_path
        ], check=True, capture_output=True, text=True)

        if result.returncode != 0:
            logging.error(f"ffmpeg failed with code {result.returncode}")
            logging.error(f"stderr: {result.stderr}")
            return {"status": "error", "message": "ffmpeg failed"}, 500
        else:
            logging.info(f"ffmpeg stdout: {result.stdout}")
            logging.info("Video generation complete")
        return {"status": "success", "video_path": video_path}, 200

    except Exception as e:
        logging.error("Error during /upload", exc_info=True)
        return {"status": "error", "message": str(e)}, 500

    finally:
        shutil.rmtree(temp_dir)
        logging.info(f"Cleaned up temp directory: {temp_dir}")

if __name__ == "__main__":
    app.run(debug=True, host='0.0.0.0')
