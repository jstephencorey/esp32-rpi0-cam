FROM python:3.11-slim

WORKDIR /app

COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY . .

RUN apt-get update && apt-get install -y ffmpeg && apt-get clean

CMD ["flask", "--app", "app", "run", "--host=0.0.0.0", "--port=5000"]
