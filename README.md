# Network_simulator

# 🌐 Network Protocol Stack Simulator

**Submission 1: Physical + Data Link Layer**

---

## 📌 Project Overview

A **console-based C++ network simulator** that implements the **Physical Layer** and **Data Link Layer** of the OSI model **from scratch**, without using any external networking libraries.

This project demonstrates how data travels through network layers, including encoding, transmission, error detection, and flow control mechanisms.

---

## ⚙️ Tech Stack

* **Language:** C++17
* **Compiler:** g++ (MinGW / GCC)
* **IDE:** Visual Studio Code
* **Input:** Interactive console menu
* **Output:** Structured logs showing layer-by-layer behavior

---

## 📂 Project Structure

```
NetworkSimulator/
├── include/
│   └── network.h      # All class definitions (single-header design)
├── src/
│   └── main.cpp       # Main function + test cases
├── .vscode/
│   ├── tasks.json     # Build configuration
│   └── launch.json    # Debug configuration
├── docs/
│   └── SPECIFICATION.md
└── simulator          # Compiled executable
```

---

## 🔌 Layer Implementations

### 🟢 Physical Layer

* ASCII → **8-bit binary conversion**
* **NRZ-L line encoding**
* **Noise simulation** (10% random bit flips)
* Devices:

  * `EndDevice`
  * `Hub`
* **Signal structure:**

  * Raw bits
  * Encoded signal
  * Error flag

**Hub Behavior:**

* Floods incoming signals to all ports
* No addressing (pure Layer 1)

---

### 🔵 Data Link Layer

#### 📍 Features

* **MAC Addressing:** 48-bit format (`AA:BB:CC:DD:EE:FF`)
* **Frame Structure:**

  * Source MAC
  * Destination MAC
  * Data
  * Sequence Number
  * Checksum

#### 🛡 Error Control

* CRC-style checksum:

  ```
  sum(data bytes) % 65536
  ```

#### 📡 Access Control

* **CSMA/CD**

  * Carrier sensing
  * Collision detection
  * Random backoff

#### 🔄 Flow Control

* **Go-Back-N**

  * Window size = 4
  * Retransmits full window on error

* **Selective Repeat**

  * Retransmits only failed frames
  * Supports out-of-order buffering

#### 🔀 Devices

* **Switch**

  * MAC learning table
  * Unicast & broadcast forwarding

* **Bridge**

  * Two-port forwarding

* **Broadcast Address:**

  ```
  FF:FF:FF:FF:FF:FF
  ```

---

## 🧪 Test Cases

### 🔹 Test 1: Point-to-Point

* 2 devices, direct link
* Demonstrates encoding & transmission

**Domains:**

* Collision: 1
* Broadcast: 1

---

### 🔹 Test 2: Hub Star Topology

* 5 devices connected via hub
* Signal flooding behavior

**Domains:**

* Collision: 1
* Broadcast: 1

---

### 🔹 Test 3: Switch + 5 Devices

* Full Data Link Layer demo:

  * CSMA/CD collisions
  * MAC learning
  * CRC validation
  * Go-Back-N retransmission
  * Selective Repeat recovery

**Domains:**

* Collision: 5
* Broadcast: 1

---

### 🔹 Test 4: Two Hub Networks via Switch

* 10 devices (5 per hub)
* Inter-network communication

**Domains:**

* Collision: 2
* Broadcast: 1

---

## 📜 Protocol Details

### 📡 CSMA/CD

* Checks channel before transmission
* Detects collisions → retries with backoff

---

### 🔁 Go-Back-N

* Window size: 4
* Sequence range: 0–7
* Retransmits all frames after error

---

### 🔂 Selective Repeat

* Retransmits only failed frames
* Stores out-of-order packets

---

### ✅ CRC Checksum

* Algorithm:

  ```
  sum(bytes) % 65536
  ```
* Used for error detection

---

## 🛠 Build & Run

### ✅ Prerequisites

* g++ (C++17 support)
* VS Code with C/C++ extension

---

### ▶️ Steps (VS Code)

1. Open project folder
2. Press **Ctrl + Shift + B** to build
3. Open terminal
4. Run:

   * Linux/Mac:

     ```
     ./simulator
     ```
   * Windows:

     ```
     simulator.exe
     ```

---

### ⚡ Manual Build

```bash
g++ -std=c++17 -Wall -o simulator src/main.cpp
./simulator
```

---

## ⚠️ Assumptions & Limitations

* No real network communication (simulation only)
* Static MAC address assignment
* Random noise model
* No higher layers (IP, TCP, etc.)
* No MAC table aging
* Simplified ACK handling (no timers)

---

## 🚀 Future Enhancements

* Network topology visualizer
* Advanced encoding (Manchester, 4B5B)
* Error correction (Hamming Code)
* Additional protocols:

  * ALOHA / Slotted ALOHA
  * Token Ring
* Spanning Tree Protocol
* IP Layer implementation (Next phase)

---

## ⭐ Summary

This project provides a **hands-on simulation of core networking concepts**, helping understand how real-world communication happens at the **bit and frame level**.

---

