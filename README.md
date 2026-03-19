# meteo_station
# 🌦️ Weather Station System

**🌐 Live Dashboard:** You can observe the real-time measurements and historical data from this weather station here: 👉 **[Live Dashboard](https://s2.mateuszz.pl/weather/)**  

An end-to-end, distributed Internet of Things (IoT) project for environmental monitoring. The system consists of a remote sensor node, a dedicated client display, and a full-stack web server handling data processing, storage, and visualization.

## 🏗️ System Architecture

The project is divided into three main components communicating over Wi-Fi via a custom REST API:

1. **Sensor Node (ESP8266)** 📡 -> Reads environmental data and pushes it to the server.
2. **Backend & Frontend Server** 💻 -> PHP/MySQL server that processes data, handles caching (e.g., Sunrise/Sunset API), stores history, and serves a web dashboard with Chart.js.
3. **Client Display (ESP32)** 📟 -> Fetches processed data from the server and displays real-time weather conditions with sunrise/sunset data.

## 🔌 Hardware Components

* **Microcontrollers:** ESP8266 (NodeMCU/Wemos D1), ESP32  
* **Sensors:** **BME280** (Temperature, Humidity, Barometric Pressure via I2C)  **Sharp GP2Y1010AU0F** (Experimental Optical Dust Sensor / Air Quality monitoring)
* **Display:** [ESP32 Cheap Yellow Display (CYD)](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)

## 💻 Software Stack & Technologies

* **Embedded:** C/C++, Arduino IDE, I2C communication, Analog signal processing
* **Backend:** PHP, MySQL (phpMyAdmin), REST API architecture
* **Frontend:** HTML5, CSS3, JavaScript, Chart.js for data visualization
* **External APIs:** Sunrise-Sunset API (with custom PHP caching mechanism)

## 📁 Repository Structure

```text
├── sensor-box/      # C++ code for ESP8266 data acquisition and HTTP POST
├── meteo_client/     # C++ code for ESP32 fetching and displaying data
├── meteo_backend/           # PHP scripts (API endpoints) and MySQL database schema
└── meteo_frontend/             # HTML/JS files for the interactive browser dashboard
