# 🔄 Firmware Update via OTA Bootloader

## 👥 Group Members

- **Varun Krishnamurthy** – u1530331  
- **Alex Wang** – u1545779  
- **Tyler Allison** – u1343396

---

## 🛠️ Prerequisites: ESP-IDF Installation

1. **Clone the ESP-IDF repository**  
   ```bash
   git clone --recursive https://github.com/espressif/esp-idf.git
   cd esp-idf
   ```

2. **Install required tools**  
   ```bash
   ./install.sh
   ```

3. **Set up the environment**  
   ```bash
   . export.sh
   ```

---

## 📁 Project Setup

1. Navigate to the project directory:
   ```bash
   cd /path/to/file_serving_stm
   ```

2. Set the ESP32 target (choose based on your chip):
   ```bash
   idf.py set-target esp32
   ```

3. Configure project settings (e.g., Wi-Fi SSID and password):
   ```bash
   idf.py menuconfig
   ```
   - Navigate to the appropriate menu and enter your Wi-Fi credentials.
   - Save and exit.

---

## 🔌 Flashing the Firmware

1. Connect the ESP32 and flash the firmware:
   ```bash
   idf.py -p /dev/ttyUSB0 -b 115200 flash
   ```

2. Start monitoring the serial output:
   ```bash
   idf.py -p /dev/ttyUSB0 monitor
   ```

3. After startup, look for the **IP address** assigned to the ESP32.  
   This address will be used to upload firmware files from your browser.

---

## 🌐 Uploading the `.bin` File Over-the-Air

- Ensure your computer is **on the same Wi-Fi network** as the ESP32.
- Open a browser and go to the IP address shown in the monitor.
- Upload your STM32 `.bin` firmware file via the web interface.
- Click **Flash** — your firmware will now be updated wirelessly.