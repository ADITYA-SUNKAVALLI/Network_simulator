# Network_simulator

📡 Network Protocol Stack Simulator
(Physical + Data Link Layer Implementation in C++)

🚀 Overview

This project is a console-based network simulator built in C++17, implementing the Physical Layer (Layer 1) and Data Link Layer (Layer 2) of the OSI model from scratch, without using any external networking libraries.

The simulator demonstrates how data is transmitted, encoded, controlled, and managed at the lowest levels of networking using real protocol logic.

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
