#!/bin/bash
set -e

### === VARIABLES (adjust as needed) ===
PI_USER="jstephencorey"
PI_GROUP="root"
PROJECT_DIR="/home/jstephencorey/esp32-rpi0-cam/rpi-api"   # path where app.py, requirements.txt, etc. live
SERVICE_NAME="myapi"
SSID="PiZeroHotspot"
WIFI_PASSWORD="password"    # at least 8 chars
STATIC_IP="192.168.4.1"

echo "=== Updating and installing dependencies ==="
apt update
apt install -y python3-venv python3-pip hostapd dnsmasq gunicorn

echo "=== Configuring static IP for wlan0 ==="
cat <<EOF >> /etc/dhcpcd.conf

interface wlan0
    static ip_address=${STATIC_IP}/24
    nohook wpa_supplicant
EOF

systemctl restart dhcpcd

echo "=== Configuring dnsmasq ==="
mv /etc/dnsmasq.conf /etc/dnsmasq.conf.orig || true
cat <<EOF > /etc/dnsmasq.conf
interface=wlan0
dhcp-range=192.168.4.2,192.168.4.20,255.255.255.0,24h
EOF

echo "=== Configuring hostapd ==="
cat <<EOF > /etc/hostapd/hostapd.conf
interface=wlan0
driver=nl80211
ssid=${SSID}
hw_mode=g
channel=7
wmm_enabled=0
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
wpa=2
wpa_passphrase=${WIFI_PASSWORD}
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP
EOF

sed -i "s|#DAEMON_CONF=\"\"|DAEMON_CONF=\"/etc/hostapd/hostapd.conf\"|" /etc/default/hostapd

echo "=== Enabling hostapd and dnsmasq ==="
systemctl unmask hostapd
systemctl enable hostapd
systemctl enable dnsmasq
systemctl start hostapd
systemctl start dnsmasq

echo "=== Setting up Python virtual environment and installing requirements ==="
sudo -u ${PI_USER} bash <<EOSU
cd ${PROJECT_DIR}
python3 -m venv venv
source venv/bin/activate
pip install --upgrade pip
pip install -r requirements.txt
EOSU

echo "=== Creating systemd service for API ==="
cat <<EOF > /etc/systemd/system/${SERVICE_NAME}.service
[Unit]
Description=Gunicorn instance to serve Flask API
After=network.target

[Service]
User=${PI_USER}
Group=${PI_GROUP}
WorkingDirectory=${PROJECT_DIR}
Environment="PATH=${PROJECT_DIR}/venv/bin"
ExecStart=${PROJECT_DIR}/venv/bin/gunicorn -c ${PROJECT_DIR}/gunicorn_config.py app:app
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
EOF

echo "=== Enabling and starting API service ==="
systemctl daemon-reload
systemctl enable ${SERVICE_NAME}
systemctl start ${SERVICE_NAME}

echo "=== Setup complete ==="
echo "Hotspot SSID: ${SSID}"
echo "Password: ${WIFI_PASSWORD}"
echo "Static IP of Pi: ${STATIC_IP}"
echo "Flask API service: ${SERVICE_NAME} (systemd)"
