# Network_simulator


📡 Network Protocol Stack Simulator
(Physical + Data Link Layer Implementation in C++)
🚀 Overview

This project is a console-based network simulator built in C++17, implementing the Physical Layer (Layer 1) and Data Link Layer (Layer 2) of the OSI model from scratch, without using any external networking libraries.

The simulator demonstrates how data is transmitted, encoded, controlled, and managed at the lowest levels of networking using real protocol logic.

🎯 Objectives
Understand low-level network communication
Simulate bit-level transmission and encoding
Implement real-world data link protocols
Visualize how devices like Hub, Switch, and Bridge behave
🛠️ Tech Stack
Language: C++17
Compiler: g++ (MinGW / GCC)
IDE: Visual Studio Code
Interface: Console-based interactive menu

📡 Network Protocol Stack Simulator
(Physical + Data Link Layer Implementation in C++)
🚀 Overview

This project is a console-based network simulator built in C++17, implementing the Physical Layer (Layer 1) and Data Link Layer (Layer 2) of the OSI model from scratch, without using any external networking libraries.

The simulator demonstrates how data is transmitted, encoded, controlled, and managed at the lowest levels of networking using real protocol logic.

🎯 Objectives
Understand low-level network communication
Simulate bit-level transmission and encoding
Implement real-world data link protocols
Visualize how devices like Hub, Switch, and Bridge behave
🛠️ Tech Stack
Language: C++17
Compiler: g++ (MinGW / GCC)
IDE: Visual Studio Code
Interface: Console-based interactive menu
📂 Project Structure

NetworkSimulator/
├── include/
│   └── network.h        # All class implementations (single-header design)
├── src/
│   └── main.cpp         # Main function + test cases
├── .vscode/
│   ├── tasks.json       # Build configuration
│   └── launch.json      # Debug configuration
├── docs/
│   └── SPECIFICATION.md # Detailed specification
└── simulator            # Compiled executable



⚙️ Features
🔌 Physical Layer (Layer 1)
Bit Representation
ASCII → 8-bit binary conversion
Line Coding
NRZ-L encoding
1 → High, 0 → Low
Noise Simulation
10% probability of random bit flips
Devices
EndDevice
Hub (broadcasts to all ports)
Signal Structure

struct Signal {
    string bits;
    string nrzEncoded;
    bool hasError;
};

🔗 Data Link Layer (Layer 2)
MAC Addressing
48-bit format: AA:BB:CC:DD:EE:FF
Frame Structure
Source MAC
Destination MAC
Data
Sequence number
Checksum
Error Detection
CRC-style checksum (sum % 65536)
📡 Protocol Implementations
1. CSMA/CD (Carrier Sense Multiple Access with Collision Detection)
Sense channel before transmission
Detect collisions
Random back-off and retry
2. Go-Back-N ARQ
Window size: 4
Sequence space: 0–7
On timeout → retransmit all unacknowledged frames
3. Selective Repeat ARQ
Window size: 4
Retransmits only specific lost frames
Supports out-of-order buffering


4. Switching & Bridging
Switch
MAC learning table
Unicast + Flood forwarding
Bridge
Two-port forwarding
Broadcast Support
FF:FF:FF:FF:FF:FF
🧪 Test Cases
✅ Test 1 — Point-to-Point
Two devices directly connected
Demonstrates encoding & transmission
Collision Domains: 1
Broadcast Domains: 1
✅ Test 2 — Hub Star Topology
5 devices connected to a hub
Hub floods signal
Collision Domains: 1
Broadcast Domains: 1
✅ Test 3 — Switch + Devices (Full DLL Demo)
CSMA/CD simulation
MAC learning and unicast switching
CRC validation (pass/fail cases)
Go-Back-N with retransmission
Selective Repeat with NAK
Collision Domains: 5
Broadcast Domains: 1
✅ Test 4 — Hybrid Topology
5 PCs → HUB → SWITCH → HUB → 5 PCs

Cross-network communication
Demonstrates segmentation behavior
Collision Domains: 2
Broadcast Domains: 1
🧑‍💻 Build & Run
🔧 Prerequisites
g++ (C++17 support)
Visual Studio Code
C/C++ Extension

⚠️ Assumptions & Limitations
No real network communication (fully simulated)
MAC addresses are manually assigned
Noise model is probabilistic
No higher-layer protocols (IP, TCP, ARP)
No MAC table aging in switch
Simplified ACK/NAK handling (no real timers)
🔮 Future Enhancements
Topology visualization (ASCII / GUI)
Advanced encoding (Manchester, 4B5B)
Error correction (Hamming Code)
Access protocols (ALOHA, Token Ring)
Spanning Tree Protocol
Network Layer (IP routing)
📌 Key Learning Outcomes
Deep understanding of OSI Layer 1 & 2
Practical implementation of network protocols
Insight into real-world networking behavior
Hands-on experience with low-level system design
