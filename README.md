## 🔄 Firmware Update via OTA Bootloader

### 👥 Group Members

- **Varun Krishnamurthy** – u1530331  
- **Alex Wang** – u1545779  
- **Tyler Allison** – u1343396

---

## 📖 Table of Contents
1. [Project Overview](#project-overview)  
2. [Key Features](#key-features)  
3. [Hardware Connections](#hardware-connections)  
4. [Software Components](#software-components)  
5. [Usage Instructions](#usage-instructions)  
6. [Limitations](#limitations--future-work)  
7. [Flowcharts](#flowcharts)
8. [Demo Video](#demo-video)

---

## 📝 Project Overview

This repository demonstrates two methods for performing firmware updates on an STM32 microcontroller via an ESP32 (or PC + FTDI) over-the-air (OTA) bootloader:

1. **System Bootloader**: Utilizes the built-in STM32 system bootloader (enter by setting BOOT0 = High) to flash via UART.  
2. **Custom OTA Bootloader**: A bespoke STM32 bootloader that leverages DMA and CRC validation to receive and flash new firmware in the background, then toggles memory banks for near-zero downtime.

The primary goal is to showcase a robust custom bootloader that:
- **Validates** incoming packets with CRC.  
- **Erases** and **Programs** flash banks via DMA.  
- **Toggles** active bank by remapping the vector table.

---

## ✨ Key Features

- **Dual-Bank Flashing** with STM32U series.  
- **CRC-Checked** Data Integrity.  
- **DMA-Based** Programming for efficiency.  
- **Zero-Downtime** Bank Toggle on Transfer Complete.  
- **Python Server** for easy `.bin` upload and packet framing.

---

## 🔌 Hardware Connections

| Host     | STM32 Pin | Signal |
| -------- | --------- | ------ |
| **ESP32**| PA7 (TX)  | RX2    |
|          | PA5 (RX)  | TX2    |
| **FTDI** | TX        | RX     |
|          | RX        | TX     |
| **Both** | GND       | GND    |

---

## 💾 Software Components

- **STM32**  
  - `flash_control.c` & `serial_port.c` – UART reception, CRC check, flash erase/program.  
  - Vector Table remap via `SCB->VTOR = FLASH_BANK2_BASE;`.  
- **Python Host**  
  - Flask server for file upload & framing.  
  - Packetizer: splits `.bin` into WINDOW_SIZE (128B) + CRC.

---

## 🚀 Usage Instructions

1. **Clone Repo**  
   ```bash
   git clone https://github.com/uofu-emb-25/Firmware_Update_OTA-Bootloader.git
   cd Firmware_Update_OTA-Bootloader
   ```
2. **Build STM32 Projects** in STM32CubeIDE for both system and custom bootloaders.  
3. **Hardware Setup**:  
   - For **System Bootloader**, set BOOT0 = `High` & reset.  
   - For **Custom OTA**, ensure FTDI/ESP32 is connected as per diagram.  
4. **Run Python Code**:  
5. **Upload & Flash**: Navigate to `http://<host_ip>:5000`, choose `.bin`, and click **Upload**.  
6. **Observe**: Green LED → packets → Red LED on new firmware.

---

## 🛠️ Limitations

- **Edge Cases**: CRC errors not fully retried/backoff.  
- **Security**: No authentication/encryption over UART.  
- **Robustness**: Need better error handling on erase/program failures.  

---

## 📊 Flowcharts

Original Flowchart: (https://github.com/uofu-emb-25/Firmware_Update_OTA-Bootloader/blob/main/Original_Flowchart.png)

Updated Flowchart: (https://github.com/uofu-emb-25/Firmware_Update_OTA-Bootloader/blob/main/Updated_Flowchart.png)

## 📹 Demo Video

See the live demo: 
[![Watch Demo]

(https://github.com/uofu-emb-25/Firmware_Update_OTA-Bootloader/blob/main/PXL_20250425_171607655.TS%20(1).mp4)

