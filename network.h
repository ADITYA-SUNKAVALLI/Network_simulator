#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <bitset>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <memory>

// ─────────────────────────────────────────────
//  CONSTANTS
// ─────────────────────────────────────────────
const int MAX_FRAME_SIZE   = 1024;  // bits
const int WINDOW_SIZE      = 4;     // sliding window
const int MAX_SEQ          = 8;     // Go-Back-N sequence space
const double NOISE_PROB    = 0.1;   // 10% chance of bit error
const int SLOT_TIME_MS     = 10;    // CSMA/CD slot time (simulated)

// ─────────────────────────────────────────────
//  MAC ADDRESS
// ─────────────────────────────────────────────
struct MACAddress {
    std::string addr;   // e.g. "AA:BB:CC:DD:EE:01"
    MACAddress() : addr("00:00:00:00:00:00") {}
    explicit MACAddress(const std::string& a) : addr(a) {}
    bool operator==(const MACAddress& o) const { return addr == o.addr; }
    bool operator<(const MACAddress& o)  const { return addr < o.addr; }
    bool isBroadcast() const { return addr == "FF:FF:FF:FF:FF:FF"; }
    static MACAddress broadcast() { return MACAddress("FF:FF:FF:FF:FF:FF"); }
};

// ─────────────────────────────────────────────
//  FRAME (Data Link Layer PDU)
// ─────────────────────────────────────────────
struct Frame {
    MACAddress  src;
    MACAddress  dst;
    std::string data;
    int         seq      = 0;   // sequence number
    int         ack      = -1;  // acknowledgement number (-1 = data frame)
    bool        isAck    = false;
    bool        isNak    = false;
    uint16_t    checksum = 0;

    Frame() = default;
    Frame(MACAddress s, MACAddress d, const std::string& payload, int sq = 0)
        : src(s), dst(d), data(payload), seq(sq) {}

    // CRC-like simple checksum (sum of ASCII values mod 65536)
    void computeChecksum() {
        checksum = 0;
        for (char c : data) checksum += static_cast<uint8_t>(c);
        checksum %= 65536;
    }
    bool verifyChecksum() const {
        uint16_t cs = 0;
        for (char c : data) cs += static_cast<uint8_t>(c);
        return (cs % 65536) == checksum;
    }
    void display(const std::string& tag = "") const {
        if (!tag.empty()) std::cout << "[" << tag << "] ";
        std::cout << "Frame{seq=" << seq
                  << ", src=" << src.addr
                  << ", dst=" << dst.addr
                  << ", data=\"" << data << "\""
                  << ", checksum=" << checksum
                  << (isAck  ? ", ACK"  : "")
                  << (isNak  ? ", NAK"  : "")
                  << "}\n";
    }
};

// ─────────────────────────────────────────────
//  SIGNAL (Physical Layer PDU)
// ─────────────────────────────────────────────
struct Signal {
    std::string bits;       // binary representation
    std::string nrzEncoded; // NRZ-L line coding
    std::string src;        // source device name
    bool hasError = false;  // noise-induced error flag

    // Convert data string → bits (ASCII)
    void encode(const std::string& data) {
        bits.clear();
        for (char c : data) {
            std::bitset<8> b(static_cast<unsigned char>(c));
            bits += b.to_string();
        }
        nrzEncode();
    }

    // NRZ-L: '0' → low (-1), '1' → high (+1)
    void nrzEncode() {
        nrzEncoded.clear();
        for (char b : bits) nrzEncoded += (b == '1' ? "H" : "L");
    }

    void display() const {
        std::cout << "  Physical bits : " << bits       << "\n";
        std::cout << "  NRZ-L signal  : " << nrzEncoded << "\n";
        if (hasError) std::cout << "  [!] Bit error introduced by noise\n";
    }
};

// ─────────────────────────────────────────────
//  FORWARD DECLARATIONS
// ─────────────────────────────────────────────
class Connection;
class EndDevice;
class Hub;
class Switch;
class Bridge;

// ─────────────────────────────────────────────
//  BASE DEVICE
// ─────────────────────────────────────────────
class Device {
public:
    std::string            name;
    std::vector<Connection*> connections;

    explicit Device(const std::string& n) : name(n) {}
    virtual ~Device() = default;
    virtual void receiveSignal(Signal& sig, Device* from) = 0;
    virtual std::string type() const = 0;
    void addConnection(Connection* c) { connections.push_back(c); }
};

// ─────────────────────────────────────────────
//  CONNECTION (Physical Link)
// ─────────────────────────────────────────────
class Connection {
public:
    Device* devA;
    Device* devB;
    bool    full_duplex;
    int     bandwidth_mbps;

    Connection(Device* a, Device* b, bool fd = true, int bw = 100)
        : devA(a), devB(b), full_duplex(fd), bandwidth_mbps(bw) {
        a->addConnection(this);
        b->addConnection(this);
    }
    Device* other(Device* d) {
        return (d == devA) ? devB : devA;
    }
    // Simulate noise with probability NOISE_PROB
    void transmit(Signal& sig, Device* sender) {
        if ((rand() % 100) < static_cast<int>(NOISE_PROB * 100)) {
            sig.hasError = true;
            // flip a random bit
            int idx = rand() % static_cast<int>(sig.bits.size());
            sig.bits[idx] = (sig.bits[idx] == '0') ? '1' : '0';
        }
        Device* receiver = other(sender);
        receiver->receiveSignal(sig, sender);
    }
};

// ─────────────────────────────────────────────
//  SLIDING WINDOW: Go-Back-N State
// ─────────────────────────────────────────────
struct GoBackNState {
    int  sendBase   = 0;
    int  nextSeqNum = 0;
    int  recvBase   = 0;
    std::vector<Frame> window; // unacknowledged frames
};

// ─────────────────────────────────────────────
//  END DEVICE
// ─────────────────────────────────────────────
class EndDevice : public Device {
public:
    MACAddress   mac;
    GoBackNState gbn;
    std::vector<std::string> receivedData;

    EndDevice(const std::string& n, const std::string& macStr)
        : Device(n), mac(macStr) {}

    std::string type() const override { return "EndDevice"; }

    // Physical layer: send raw signal
    void sendSignal(const std::string& data, Device* target = nullptr) {
        std::cout << "\n[PHY] " << name << " → transmitting signal\n";
        Signal sig;
        sig.src = name;
        sig.encode(data);
        sig.display();
        for (auto* conn : connections) {
            Device* next = conn->other(this);
            if (target == nullptr || next == target || next->type() == "Hub" || next->type() == "Switch") {
                conn->transmit(sig, this);
            }
        }
    }

    // Data link: send frame (with CRC + Go-Back-N)
    void sendFrame(const MACAddress& dst, const std::string& data) {
        Frame f(mac, dst, data, gbn.nextSeqNum % MAX_SEQ);
        f.computeChecksum();
        gbn.window.push_back(f);
        gbn.nextSeqNum++;

        std::cout << "\n[DLL] " << name << " sending frame:\n  ";
        f.display();

        // Encode frame into signal and send
        std::string payload = f.src.addr + "|" + f.dst.addr + "|" + f.data
                              + "|seq=" + std::to_string(f.seq)
                              + "|cs=" + std::to_string(f.checksum);
        sendSignal(payload);
    }

    // Physical layer receive
    void receiveSignal(Signal& sig, Device* from) override {
        std::cout << "[PHY] " << name << " received signal from " << from->name << "\n";
        sig.display();
    }

    // Data link receive
    void receiveFrame(Frame& f) {
        std::cout << "[DLL] " << name << " received frame: ";
        f.display();
        if (!f.verifyChecksum()) {
            std::cout << "  [!] Checksum ERROR — sending NAK\n";
            return;
        }
        if (f.dst.isBroadcast() || f.dst == mac) {
            receivedData.push_back(f.data);
            std::cout << "  [OK] Data accepted: \"" << f.data << "\"\n";
        } else {
            std::cout << "  [--] Frame not for me, discarding.\n";
        }
    }

    void showReceived() const {
        std::cout << "\n[" << name << "] Received messages:\n";
        for (auto& d : receivedData) std::cout << "  → \"" << d << "\"\n";
    }
};

// ─────────────────────────────────────────────
//  HUB (Physical Layer device)
//  Floods signal to ALL ports except source
// ─────────────────────────────────────────────
class Hub : public Device {
public:
    explicit Hub(const std::string& n) : Device(n) {}
    std::string type() const override { return "Hub"; }

    void receiveSignal(Signal& sig, Device* from) override {
        std::cout << "[HUB:" << name << "] Flooding signal from " << from->name << " to all ports\n";
        for (auto* conn : connections) {
            Device* next = conn->other(this);
            if (next != from) {
                Signal copy = sig; // flood copy to every port
                conn->transmit(copy, this);
            }
        }
    }
};

// ─────────────────────────────────────────────
//  BRIDGE (Data Link Layer device)
//  Two ports, maintains a simple MAC table
// ─────────────────────────────────────────────
class Bridge : public Device {
public:
    std::map<MACAddress, Connection*> macTable;

    explicit Bridge(const std::string& n) : Device(n) {}
    std::string type() const override { return "Bridge"; }

    void receiveSignal(Signal& sig, Device* from) override {
        std::cout << "[BRIDGE:" << name << "] Received signal — forwarding based on MAC table\n";
        // Bridge operates on frames; for simulation we forward to opposite port
        for (auto* conn : connections) {
            Device* next = conn->other(this);
            if (next != from) {
                Signal copy = sig;
                conn->transmit(copy, this);
            }
        }
    }
};

// ─────────────────────────────────────────────
//  SWITCH (Data Link Layer device)
//  Learns MAC addresses, unicast or floods
// ─────────────────────────────────────────────
class Switch : public Device {
public:
    std::map<MACAddress, Connection*> macTable;
    int broadcastDomains = 1; // one per switch
    int collisionDomains = 0; // one per port

    explicit Switch(const std::string& n) : Device(n) {}
    std::string type() const override { return "Switch"; }

    void learnMAC(const MACAddress& mac, Connection* conn) {
        if (macTable.find(mac) == macTable.end()) {
            macTable[mac] = conn;
            std::cout << "  [SWITCH:" << name << "] Learned MAC " << mac.addr << "\n";
        }
    }

    void forwardFrame(Frame& f, Connection* inConn) {
        // Learn source
        learnMAC(f.src, inConn);
        collisionDomains = static_cast<int>(connections.size());

        if (f.dst.isBroadcast()) {
            std::cout << "  [SWITCH:" << name << "] Broadcast — flooding all ports\n";
            for (auto* conn : connections) {
                if (conn != inConn) {
                    Device* next = conn->other(this);
                    if (next->type() == "EndDevice") {
                        static_cast<EndDevice*>(next)->receiveFrame(f);
                    }
                }
            }
        } else if (macTable.count(f.dst)) {
            Connection* outConn = macTable[f.dst];
            Device* next = outConn->other(this);
            std::cout << "  [SWITCH:" << name << "] Unicast → " << next->name << "\n";
            if (next->type() == "EndDevice") {
                static_cast<EndDevice*>(next)->receiveFrame(f);
            }
        } else {
            std::cout << "  [SWITCH:" << name << "] Unknown MAC — flooding\n";
            for (auto* conn : connections) {
                if (conn != inConn) {
                    Device* next = conn->other(this);
                    if (next->type() == "EndDevice") {
                        static_cast<EndDevice*>(next)->receiveFrame(f);
                    }
                }
            }
        }
    }

    // Switch receives signal → parse frame → forward
    void receiveSignal(Signal& sig, Device* from) override {
        std::cout << "[SWITCH:" << name << "] Processing signal from " << from->name << "\n";
        // For demo: forward signal-level to all other ports (then handle frame in sendFrameVia)
    }

    void showDomains() const {
        std::cout << "\n[SWITCH:" << name << "] Broadcast domains : " << broadcastDomains << "\n";
        std::cout << "[SWITCH:" << name << "] Collision domains  : " << collisionDomains
                  << " (one per port)\n";
    }

    void showMACTable() const {
        std::cout << "\n[SWITCH:" << name << "] MAC Address Table:\n";
        std::cout << "  ┌─────────────────────┬──────────────────┐\n";
        std::cout << "  │ MAC Address          │ Connected Device │\n";
        std::cout << "  ├─────────────────────┼──────────────────┤\n";
        for (auto& [mac, conn] : macTable) {
            Device* dev = conn->other(const_cast<Switch*>(this));
            std::cout << "  │ " << std::setw(20) << std::left << mac.addr
                      << " │ " << std::setw(16) << dev->name << " │\n";
        }
        std::cout << "  └─────────────────────┴──────────────────┘\n";
    }
};

// ─────────────────────────────────────────────
//  HELPER: Parse Frame from signal payload
// ─────────────────────────────────────────────
inline Frame parseFrameFromPayload(const std::string& payload) {
    // Format: "src|dst|data|seq=N|cs=M"
    Frame f;
    std::stringstream ss(payload);
    std::string token;
    std::vector<std::string> parts;
    while (std::getline(ss, token, '|')) parts.push_back(token);
    if (parts.size() >= 5) {
        f.src  = MACAddress(parts[0]);
        f.dst  = MACAddress(parts[1]);
        f.data = parts[2];
        f.seq  = std::stoi(parts[3].substr(4));
        f.checksum = static_cast<uint16_t>(std::stoi(parts[4].substr(3)));
    }
    return f;
}

// ─────────────────────────────────────────────
//  CSMA/CD Simulation (Access Control)
// ─────────────────────────────────────────────
class CSMAChannel {
public:
    bool busy = false;
    std::string name;

    explicit CSMAChannel(const std::string& n) : name(n) {}

    bool tryTransmit(const std::string& sender) {
        if (busy) {
            std::cout << "[CSMA/CD:" << name << "] " << sender
                      << " senses carrier — COLLISION DETECTED, backing off\n";
            return false;
        }
        busy = true;
        std::cout << "[CSMA/CD:" << name << "] " << sender
                  << " senses no carrier — transmitting\n";
        return true;
    }
    void release() { busy = false; }
};

// ─────────────────────────────────────────────
//  Go-Back-N Protocol
// ─────────────────────────────────────────────
class GoBackN {
public:
    static void simulate(EndDevice& sender, EndDevice& receiver,
                         const std::vector<std::string>& messages) {
        std::cout << "\n╔══════════════════════════════════════╗\n";
        std::cout << "║   Go-Back-N Sliding Window Protocol  ║\n";
        std::cout << "╚══════════════════════════════════════╝\n";
        std::cout << "  Window size = " << WINDOW_SIZE
                  << "  |  Sequence space = 0.." << MAX_SEQ - 1 << "\n\n";

        int seq = 0;
        for (const auto& msg : messages) {
            Frame f(sender.mac, receiver.mac, msg, seq % MAX_SEQ);
            f.computeChecksum();
            std::cout << "Sender  → [seq=" << f.seq << "] \"" << msg << "\"\n";

            // Simulate possible error
            bool err = (rand() % 10 < 2); // 20% loss
            if (err) {
                std::cout << "  [X] Frame seq=" << f.seq << " lost in transit\n";
                std::cout << "  [!] Timeout — retransmitting from seq=" << f.seq << "\n";
                // retransmit same frame
                receiver.receiveFrame(f);
            } else {
                receiver.receiveFrame(f);
                // send ACK
                Frame ack;
                ack.isAck = true;
                ack.ack = f.seq;
                std::cout << "Receiver← ACK=" << f.seq << "\n";
            }
            seq++;
        }
        std::cout << "\n[GBN] Transfer complete.\n";
    }
};

// ─────────────────────────────────────────────
//  Selective Repeat Protocol
// ─────────────────────────────────────────────
class SelectiveRepeat {
public:
    static void simulate(EndDevice& sender, EndDevice& receiver,
                         const std::vector<std::string>& messages) {
        std::cout << "\n╔══════════════════════════════════════════╗\n";
        std::cout << "║   Selective Repeat Sliding Window        ║\n";
        std::cout << "╚══════════════════════════════════════════╝\n";
        std::cout << "  Window size = " << WINDOW_SIZE
                  << "  |  Each NAK triggers single retransmit\n\n";

        int seq = 0;
        for (const auto& msg : messages) {
            Frame f(sender.mac, receiver.mac, msg, seq % MAX_SEQ);
            f.computeChecksum();
            std::cout << "Sender  → [seq=" << f.seq << "] \"" << msg << "\"\n";

            bool err = (rand() % 10 < 2);
            if (err) {
                std::cout << "  [X] Error at seq=" << f.seq
                          << " — receiver sends NAK\n";
                std::cout << "  [↺] Selective retransmit seq=" << f.seq << "\n";
                receiver.receiveFrame(f);
            } else {
                receiver.receiveFrame(f);
                std::cout << "Receiver← ACK=" << f.seq << "\n";
            }
            seq++;
        }
        std::cout << "\n[SR] Transfer complete.\n";
    }
};

// ─────────────────────────────────────────────
//  NETWORK TOPOLOGY BUILDER (Convenience)
// ─────────────────────────────────────────────
class Network {
public:
    std::vector<std::unique_ptr<Device>>     devices;
    std::vector<std::unique_ptr<Connection>> links;

    EndDevice* addEndDevice(const std::string& name, const std::string& mac) {
        devices.push_back(std::make_unique<EndDevice>(name, mac));
        return static_cast<EndDevice*>(devices.back().get());
    }
    Hub* addHub(const std::string& name) {
        devices.push_back(std::make_unique<Hub>(name));
        return static_cast<Hub*>(devices.back().get());
    }
    Switch* addSwitch(const std::string& name) {
        devices.push_back(std::make_unique<Switch>(name));
        return static_cast<Switch*>(devices.back().get());
    }
    Bridge* addBridge(const std::string& name) {
        devices.push_back(std::make_unique<Bridge>(name));
        return static_cast<Bridge*>(devices.back().get());
    }
    Connection* connect(Device* a, Device* b) {
        links.push_back(std::make_unique<Connection>(a, b));
        return links.back().get();
    }
    // Send frame through switch
    void switchSend(Switch* sw, EndDevice* src, EndDevice* dst,
                    const std::string& data) {
        Frame f(src->mac, dst->mac, data, 0);
        f.computeChecksum();
        std::cout << "\n[NET] " << src->name << " → " << dst->name
                  << " : \"" << data << "\"\n";
        // Find the connection from src to switch
        Connection* inConn = nullptr;
        for (auto* c : src->connections) {
            if (c->other(src) == sw) { inConn = c; break; }
        }
        sw->forwardFrame(f, inConn);
    }
};