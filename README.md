# ⏻ ESP32 Web AC Remote Controller

A robust embedded systems project that transforms an ESP32 into a smart, web-controlled remote for my Electra air conditioning unit. This project showcases skills in reverse engineering, real-time operating systems (FreeRTOS), and full-stack embedded development, from hardware interfacing to a user-facing web server.

## ✨ Key Features

-   **🌐 Web-Based Control:** A sleek, mobile-friendly web interface to turn the AC on or off from any device on the local network.
-   **RTOS-Powered:** Leverages the **FreeRTOS** real-time operating system for efficient, concurrent task management.
-   **⚙️ Multi-Core Processing:** Intelligently separates tasks between the ESP32's dual cores—Core 0 handles Wi-Fi and the web server, while Core 1 is dedicated to precise, real-time IR signal generation.
-   **🔧 Reverse Engineered:** The AC remote's proprietary IR signal was captured, decoded, and reverse-engineered using an oscilloscope.
-   **🛡️ Robust Synchronization:** Employs **binary semaphores** to ensure safe and reliable communication between the web server task and the IR transmission task.
-   **🔌 Custom Hardware:** Housed in a custom-designed 3D printed enclosure for a clean and professional finish.

---

## 📖 Project Overview

The primary goal of this project was to create a reliable and convenient way to control my Electra air conditioner over a Wi-Fi network. The system is built around an ESP32 microcontroller, which hosts a web server. When a user toggles the control button on the web page, an HTTP request is sent to the ESP32. This triggers a dedicated task, which then generates and transmits the precise infrared (IR) signal to emulate the original remote control, turning the AC unit on or off.

The project's complexity lies in the precise timing required for IR signal transmission and the robust software architecture needed to handle network requests and real-time signal generation simultaneously without interference.

---

## 🛠️ Hardware

-   **Microcontroller:** ESP32-WROOM-32
-   **Transmitter:** 5mm 940nm IR LED
-   **Resistor:** Preferably 100-200 Ohm resistor. (Closer to 100 produces stronger signal)
-   **Power:** 5V USB-C Power Supply
-   **Enclosure:** Custom 3D Printed Case

###⚡️Electrical Scheme

Here is the simple electrical diagram for the project. The IR LED is connected to GPIO 2.

![Placeholder for Electrical Scheme](https://placehold.co/600x400/2d2d2d/ffffff?text=Electrical+Scheme+Here)
*A simple circuit connecting the IR LED to a GPIO pin on the ESP32.*

### 📦 3D Printed Enclosure

The custom-designed enclosure provides a compact and durable housing for the electronics.

![Placeholder for 3D Box Design](https://placehold.co/600x400/2d2d2d/ffffff?text=3D+Printed+Case+Image)
*Front and back view of the 3D printed case.*

---

## 💻 Software & Tools

-   **Framework:** ESP-IDF (Espressif IoT Development Framework)
-   **OS:** FreeRTOS
-   **Language:** C
-   **IDE / Editor:** PlatformIO with VS Code
-   **Debugging & Analysis:** Oscilloscope for signal capture

---

## 🏗️ System Design

The software architecture is designed for stability and real-time performance, leveraging the power of FreeRTOS and the ESP32's dual-core capabilities.

### Task Management with FreeRTOS

The application is divided into two primary tasks:

1.  **Web Server & Wi-Fi Task (Core 0):** This task is responsible for:
    -   Initializing and maintaining the Wi-Fi connection.
    -   Running the HTTP web server to serve the HTML control page.
    -   Listening for incoming `/toggle` GET requests from the user's browser.
    -   Upon receiving a request, it gives a **binary semaphore** to signal the IR task.

2.  **IR Transmission Task (Core 1):** This task is pinned to Core 1 to guarantee its execution is not preempted by the higher-priority Wi-Fi stack running on Core 0. Its responsibilities are:
    -   Waiting indefinitely to take the binary semaphore.
    -   Once the semaphore is received, it executes the `ac_power_code()` function to generate the precise sequence of IR pulses.

This separation ensures that the timing-critical IR signal generation is never delayed, which is crucial for the AC unit to correctly interpret the signal.

```c
// IR code sending task - Pinned to Core 1 for real-time precision
xTaskCreatePinnedToCore(
    toggle_ac_task,
    "Send AC code task",
    1024,
    NULL,
    0,
    NULL,
    1
);
```

---

## 🔬 Reverse Engineering the IR Signal
The most challenging aspect of this project was replicating the AC's IR signal. Since the protocol is proprietary, it had to be captured and decoded manually.

1. **Signal Capture:** An IR receiver was connected to an oscilloscope to visualize the signal transmitted by the original remote control when the power button was pressed.

2. **Signal Analysis:** The captured waveform was analyzed to determine the encoding scheme. The signal consists of a 38kHz carrier frequency modulated with specific pulse widths to represent logical '1's and '0's.

  - **Logical '1':** A ~950µs pulse of the 38kHz carrier wave.
  
  - **Logical '0':** A ~950µs period of silence.

3. **Code Replication:** The sequence of '1's and '0's forming the "power toggle" command was transcribed and implemented in the ac_power_code() function. A pulseIR() function generates the 38kHz carrier for the required duration by rapidly toggling a GPIO pin.

[The captured IR signal on the oscilloscope, showing the pulse train for the power command.]

---

## 🚀 Installation & Usage
Clone the repository:

git clone <your-repo-url>

Configure Wi-Fi Credentials:
Open main.c and update the SSID and PASS macros with your network details.

#define SSID "YOUR_WIFI_SSID"
#define PASS "YOUR_WIFI_PASSWORD"

Build and Flash:
Using PlatformIO, build and upload the project to the ESP32.

Find the IP Address:
Open the serial monitor to view the IP address assigned to the ESP32 once it connects to your Wi-Fi network.

Control Your AC:
Open a web browser on any device connected to the same network and navigate to the ESP32's IP address. Click the "Toggle AC" button to control your air conditioner.

---

##🔮 Future Improvements
Full Remote Emulation: Decode and implement all buttons from the remote (temperature, fan speed, mode).

Cloud Integration: Connect the device to an MQTT broker for control over the internet.

State Feedback: Use a non-contact temperature sensor or a smart plug with power monitoring to confirm the AC's state and display it on the web interface.

OTA Updates: Implement Over-The-Air firmware updates for easy maintenance.
