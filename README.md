# 🔍 Smart X-Ray Luggage Detection System

> Real-Time Dangerous Object Detection · Automated Hardware Response via STM32 · Cloud Monitoring

[![Python](https://img.shields.io/badge/Python-3.10-blue?logo=python)](https://python.org)
[![YOLOv8](https://img.shields.io/badge/YOLOv8-Ultralytics-purple)](https://ultralytics.com)
[![FastAPI](https://img.shields.io/badge/FastAPI-0.111-green?logo=fastapi)](https://fastapi.tiangolo.com)
[![CUDA](https://img.shields.io/badge/CUDA-12-76B900?logo=nvidia)](https://developer.nvidia.com/cuda-toolkit)
[![Railway](https://img.shields.io/badge/Deploy-Railway-0B0D0E?logo=railway)](https://railway.app)
[![License](https://img.shields.io/badge/License-MIT-yellow)](LICENSE)

A full-stack **AI-powered security platform** for automated baggage scanning. Combines a GPU-accelerated YOLOv8 detection engine, STM32 embedded hardware automation, and a globally accessible cloud dashboard — built as a final year B.E. project at **Rajiv Gandhi Institute of Technology, Mumbai**.

**Team:** Harshal Dhavan · Chirag Pingale · Yash Fanse · Laxman Birajdar  
**Guide:** Dr. Viplav Soliv · Dept. of Electronics & Telecommunication Engineering

---

## 📸 Screenshots

| Safe State | Danger State Detected |
|---|---|
| ![Safe](images/safe_state.png) | ![Danger](images/danger_state.png) |

---

## 🎯 Key Results

| Metric | Value |
|---|---|
| mAP@50 (Overall) | **0.91** |
| Inference Speed | **25 FPS** (RTX 2060 + CUDA) |
| Confidence Threshold | **≥ 0.80** |
| End-to-End Latency | **150–200 ms** |
| FSM Confirmation Buffer | **12 consecutive frames** |
| UART Baud Rate | **9600 bps** |

---

## 🧠 Detectable Threat Classes

🔫 **Gun** · 🔪 **Knife** · ✂️ **Scissors** · 🔧 **Pliers** · 🔩 **Wrench**

---

## 🏗️ System Architecture

```
USB Camera (1280×720)
       ↓
OpenCV Frame Capture
       ↓
Image Pre-processing (Brightness · Flip · Rotate · Sharpen)
       ↓
YOLOv8m Inference (best.pt · CUDA GPU · conf ≥ 0.80)
       ↓
12-Frame FSM Confirmation Buffer
       ↓
  ┌────────────────────────────┐
  │                            │
UART "1" → STM32F4         Cloud Push (WebSocket)
  │                            │
  ├── Stop Belt Motor       FastAPI Backend (Railway)
  ├── Eject Solenoid            │
  ├── Sound Buzzer (1kHz)   PostgreSQL DB
  ├── Red LED ON                │
  └── LCD: DANGER MODE      Admin Dashboard (JWT)
```

**Three Operating Modes:**
- 🟢 **SAFE** — Belt running, green LED, LCD shows "SYSTEM SAFE"
- 🔴 **DANGER** — Belt stopped, solenoid ejects, buzzer + red LED + cloud alert
- 🚨 **EMERGENCY STOP** — Hardware button (PB10), immediate halt, manual reset required

---

## 📁 Repository Structure

```
Smart-X-Ray-Luggage-Detection-System/
│
├── ai-engine/                  # Local PC detection software
│   ├── gpulocal3.py            # Main app: Flask + pywebview + YOLOv8 + UART
│   ├── .env.example            # Environment variable template
│   └── requirements.txt        # Python dependencies
│
├── cloud-backend/              # FastAPI cloud server (Railway)
│   ├── main.py                 # FastAPI app entry point
│   ├── auth.py                 # JWT authentication
│   ├── admin.py                # Admin login route
│   ├── database.py             # SQLAlchemy models + async DB setup
│   ├── detections.py           # Detection push, list, export, clear
│   ├── settings.py             # Remote settings sync
│   ├── stream.py               # WebSocket + MJPEG stream manager
│   ├── __init__.py
│   ├── requirements.txt        # FastAPI dependencies
│   ├── .env.example            # Cloud environment variables
│   └── SETUP.md                # Local dev setup guide
│
├── stm32-firmware/             # STM32F103C8T6 embedded firmware
│   └── xray_scanner/           # Arduino IDE project
│
├── docs/                       # Architecture diagrams & project poster
│   └── system_architecture.pdf
│
├── images/                     # UI screenshots for README
│
├── .gitignore
├── LICENSE
└── README.md
```

---

## ⚡ Quick Start

### 1. AI Engine (Local PC)

**Requirements:** Windows 10/11, NVIDIA GPU with CUDA, Python 3.10 (Anaconda)

```bash
cd ai-engine

# Create and activate environment
conda create -n xraysim python=3.10
conda activate xraysim

# Install dependencies
pip install -r requirements.txt

# Configure environment
cp .env.example .env
# Edit .env with your cloud URL and credentials

# Run
python gpulocal3.py
```

> 📌 The app opens as a desktop window (pywebview) and is also accessible from your phone via `http://<local-ip>:5000`

### 2. Cloud Backend (Railway / Local)

```bash
cd cloud-backend

pip install -r requirements.txt

# Configure environment
cp .env.example .env
# Fill in SECRET_KEY, ADMIN_PASSWORD, DATABASE_URL, Cloudinary keys

# Run locally
uvicorn main:app --host 0.0.0.0 --port 8000 --reload
```

**Deploy to Railway:** Connect the `cloud-backend/` folder as a Railway service and set the environment variables from `.env.example` in the Railway dashboard.

### 3. STM32 Firmware

1. Open `stm32-firmware/xray_scanner/` in Arduino IDE 2.0+
2. Install board package: **STM32duino** by STMicroelectronics
3. Connect STM32F103C8T6 via USB-to-serial adapter
4. Upload firmware
5. Connect UART TX/RX to the PC's COM port (default: COM5, 9600 baud)

---

## 🔌 STM32 Pin Reference

| Pin | Function |
|---|---|
| PA4 | Belt Motor Relay (NC) |
| PA5 | Solenoid (NO) |
| PB1 | Red LED |
| PA7 | Green LED |
| PB0 | Yellow LED |
| PA0 | Buzzer (1kHz tone) |
| PB12 | Toggle Switch (input) |
| PB10 | Emergency Stop Button |
| UART | RX/TX Serial (9600 baud) |
| I²C | 0x27 — 16×2 LCD |

**Power:** 230V AC → SMPS 12V/10A → Buck Conv. 5V/3A (STM32 + logic) · 12V direct (motor + solenoid)

---

## 🤖 AI Model

- **Architecture:** YOLOv8m (Ultralytics)
- **Dataset:** 8,295 labeled airport X-ray images (Roboflow) — 70/20/10 train/val/test split
- **Training:** 35 epochs · 640×640 · NVIDIA RTX 2060 · COCO pretrained weights
- **Augmentation:** Brightness, rotation, horizontal flip, sharpening, mosaic

> ⚠️ **Model weights (`best.pt`) are not included** in this repo due to file size. Download from [Google Drive / HuggingFace — link here] and place in `ai-engine/`.

---

## ☁️ Cloud API Endpoints

| Method | Endpoint | Description |
|---|---|---|
| POST | `/api/admin/login` | Get JWT token |
| POST | `/api/detections/push` | PC pushes detection + image |
| GET | `/api/detections/list` | Paginated detection log |
| DELETE | `/api/detections/clear` | Clear all detections |
| GET | `/api/detections/export` | Download CSV |
| GET | `/api/settings/` | Get system settings |
| POST | `/api/settings/update` | Update conf threshold / camera |
| POST | `/api/settings/update_image` | Update image processing params |
| WS | `/ws/pc?token=` | Scanner PC WebSocket |
| WS | `/ws/watch?token=` | Admin dashboard WebSocket |
| GET | `/ws/mjpeg?token=` | MJPEG live stream |

---

## 🔒 Security Notes

- All credentials are loaded from **environment variables** — never hardcoded
- Dashboard protected with **JWT authentication** (72-hour tokens)
- Video frames are **not permanently stored** — only threat snapshots are saved
- UART communication is **local only** — not network accessible
- See `.env.example` in each folder for required variables

---

## 🧪 Detection Performance

| Class | Precision | Recall | F1 | mAP@50 |
|---|---|---|---|---|
| Gun | 0.94 | 0.87 | 0.90 | 0.92 |
| Knife | 0.93 | 0.85 | 0.89 | 0.91 |
| Pliers | 0.92 | 0.86 | 0.89 | 0.90 |
| Scissors | 0.91 | 0.84 | 0.87 | 0.89 |
| Wrench | 0.92 | 0.87 | 0.89 | 0.91 |
| **Mean** | **0.924** | **0.858** | **0.888** | **0.91** |

---

## 🔧 Software Stack

| Layer | Technology |
|---|---|
| AI Model | YOLOv8m (Ultralytics) |
| Local Backend | Flask |
| Desktop UI | pywebview |
| Vision | OpenCV |
| Cloud API | FastAPI |
| Cloud DB | PostgreSQL |
| Local DB | SQLite |
| Realtime | WebSocket |
| Image Storage | Cloudinary |
| Auth | JWT |
| Deploy | Railway |
| GPU | CUDA 12 |
| Embedded | STM32duino (Arduino IDE) |

---

## 📄 License

MIT License — see [LICENSE](LICENSE)

---

## 📚 Citation

If you use this project in academic work:

```
Dhavan H., Pingale C., Fanse Y., Birajdar L. (2026).
Smart X-Ray Luggage Detection System.
B.E. Project, Dept. of Electronics & Telecommunication,
Rajiv Gandhi Institute of Technology, Mumbai.
```
