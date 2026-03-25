// ================================================================
//  Network Protocol Stack Simulator
//  Submission 1 — Physical + Data Link Layer
//
//  Covers:
//   • End devices, Hubs, Switches, Bridges
//   • Physical layer: bit encoding, NRZ-L, noise model
//   • Data link: CRC checksum, MAC learning, broadcast/collision domains
//   • Access control: CSMA/CD
//   • Flow control: Go-Back-N + Selective Repeat
//
//  Build (VS Code terminal):
//    g++ -std=c++17 -o simulator main.cpp && ./simulator
// ================================================================

#include "../include/network.h"

// ─────────────────────────────────────────────
//  UTILITY HELPERS
// ─────────────────────────────────────────────
void divider(const std::string& title) {
    std::cout << "\n";
    std::cout << "═══════════════════════════════════════════════════════\n";
    std::cout << "  " << title << "\n";
    std::cout << "═══════════════════════════════════════════════════════\n";
}

// ─────────────────────────────────────────────
//  TEST 1 — Point-to-Point (two devices)
// ─────────────────────────────────────────────
void test1_pointToPoint() {
    divider("TEST 1: Point-to-Point Physical Layer");

    Network net;
    auto* A = net.addEndDevice("PC-A", "AA:AA:AA:AA:AA:01");
    auto* B = net.addEndDevice("PC-B", "BB:BB:BB:BB:BB:02");
    net.connect(A, B);

    std::cout << "\nTopology: PC-A ─────── PC-B\n";
    std::cout << "Goal    : PC-A sends a raw signal to PC-B\n";

    A->sendSignal("Hello");

    std::cout << "\n[Result] Physical transmission complete.\n";
    std::cout << "  Note: Hub/Switch not involved — direct dedicated link.\n";
    std::cout << "  Collision domains: 1  |  Broadcast domains: 1\n";
}

// ─────────────────────────────────────────────
//  TEST 2 — Star topology with Hub (5 devices)
// ─────────────────────────────────────────────
void test2_hubStar() {
    divider("TEST 2: Star Topology via Hub (5 End Devices)");

    Network net;
    auto* hub = net.addHub("HUB-1");
    std::vector<EndDevice*> pcs;
    for (int i = 1; i <= 5; i++) {
        std::string name = "PC-" + std::to_string(i);
        std::string mac  = "CC:CC:CC:CC:CC:0" + std::to_string(i);
        auto* pc = net.addEndDevice(name, mac);
        net.connect(pc, hub);
        pcs.push_back(pc);
    }

    std::cout << "\nTopology:\n";
    std::cout << "  PC-1 ┐\n";
    std::cout << "  PC-2 ┤\n";
    std::cout << "  PC-3 ┼── HUB-1\n";
    std::cout << "  PC-4 ┤\n";
    std::cout << "  PC-5 ┘\n\n";
    std::cout << "Goal: PC-1 sends signal; HUB floods to PC-2..PC-5\n\n";

    pcs[0]->sendSignal("BroadcastTest");

    std::cout << "\n[Analysis]\n";
    std::cout << "  Hub = 1 Collision Domain (shared medium)\n";
    std::cout << "  Hub = 1 Broadcast Domain\n";
    std::cout << "  All 5 PCs are in the SAME collision domain.\n";
}

// ─────────────────────────────────────────────
//  TEST 3 — Switch with 5 devices
//           Demonstrates MAC learning + CSMA/CD + Go-Back-N
// ─────────────────────────────────────────────
void test3_switchFiveDevices() {
    divider("TEST 3: Switch with 5 End Devices — MAC Learning + Protocols");

    Network net;
    auto* sw = net.addSwitch("SW-1");
    std::vector<EndDevice*> hosts;

    for (int i = 1; i <= 5; i++) {
        std::string name = "Host-" + std::to_string(i);
        std::string mac  = "DD:DD:DD:DD:DD:0" + std::to_string(i);
        auto* h = net.addEndDevice(name, mac);
        net.connect(h, sw);
        hosts.push_back(h);
    }

    std::cout << "\nTopology:\n";
    std::cout << "  Host-1 ┐\n";
    std::cout << "  Host-2 ┤\n";
    std::cout << "  Host-3 ┼── SW-1\n";
    std::cout << "  Host-4 ┤\n";
    std::cout << "  Host-5 ┘\n\n";

    // ── 3a. CSMA/CD ─────────────────────────────
    std::cout << "── 3a. Access Control: CSMA/CD ──────────────────────\n";
    CSMAChannel channel("SW-1-channel");
    bool ok1 = channel.tryTransmit("Host-1");
    if (ok1) {
        channel.tryTransmit("Host-2"); // collision scenario
        channel.release();
        channel.tryTransmit("Host-2"); // retry after back-off
        channel.release();
    }

    // ── 3b. MAC Learning + Unicast/Broadcast ────
    std::cout << "\n── 3b. Data Link — MAC Address Learning ─────────────\n";
    // broadcast first (unknown destination)
    net.switchSend(sw, hosts[0], hosts[4], "Hello Host-5!");
    // now Host-5's MAC is known; send reply
    net.switchSend(sw, hosts[4], hosts[0], "Reply from Host-5");
    // unicast between known MACs
    net.switchSend(sw, hosts[1], hosts[2], "Host-2 to Host-3");

    sw->showMACTable();

    // ── 3c. Error Control (CRC) ─────────────────
    std::cout << "\n── 3c. Error Control: CRC Checksum ──────────────────\n";
    Frame f(hosts[0]->mac, hosts[1]->mac, "DataWithCRC", 0);
    f.computeChecksum();
    std::cout << "Original  : "; f.display();
    // corrupt data
    Frame corrupted = f;
    corrupted.data = "DataWithCRX"; // tampered
    std::cout << "Corrupted : "; corrupted.display();
    std::cout << "Verification (original) : "
              << (f.verifyChecksum() ? "PASS" : "FAIL") << "\n";
    std::cout << "Verification (corrupted): "
              << (corrupted.verifyChecksum() ? "PASS" : "FAIL") << "\n";

    // ── 3d. Go-Back-N ───────────────────────────
    std::cout << "\n── 3d. Flow Control: Go-Back-N ──────────────────────\n";
    GoBackN::simulate(*hosts[0], *hosts[4],
        {"Msg-1", "Msg-2", "Msg-3", "Msg-4", "Msg-5"});

    // ── 3e. Selective Repeat ────────────────────
    std::cout << "\n── 3e. Flow Control: Selective Repeat ───────────────\n";
    SelectiveRepeat::simulate(*hosts[1], *hosts[3],
        {"Alpha", "Beta", "Gamma", "Delta"});

    // ── 3f. Domain analysis ─────────────────────
    sw->collisionDomains = static_cast<int>(sw->connections.size());
    sw->showDomains();
    std::cout << "  Explanation: Each switch port = its own collision domain.\n";
    std::cout << "  5 ports → 5 collision domains, 1 broadcast domain.\n";
}

// ─────────────────────────────────────────────
//  TEST 4 — Two Hub-stars connected by Switch
//           (10 devices, 2 hubs, 1 switch)
// ─────────────────────────────────────────────
void test4_twoStarsViaSwitch() {
    divider("TEST 4: Two Hub-Stars Connected via Switch (10 Devices)");

    Network net;
    auto* sw   = net.addSwitch("SW-CORE");
    auto* hub1 = net.addHub("HUB-LEFT");
    auto* hub2 = net.addHub("HUB-RIGHT");

    // Connect hubs to switch
    net.connect(hub1, sw);
    net.connect(hub2, sw);

    std::vector<EndDevice*> leftPCs, rightPCs;
    for (int i = 1; i <= 5; i++) {
        std::string name = "L" + std::to_string(i);
        std::string mac  = "EE:EE:EE:EE:01:0" + std::to_string(i);
        auto* pc = net.addEndDevice(name, mac);
        net.connect(pc, hub1);
        leftPCs.push_back(pc);
    }
    for (int i = 1; i <= 5; i++) {
        std::string name = "R" + std::to_string(i);
        std::string mac  = "FF:FF:FF:FF:02:0" + std::to_string(i);
        auto* pc = net.addEndDevice(name, mac);
        net.connect(pc, hub2);
        rightPCs.push_back(pc);
    }

    std::cout << "\nTopology:\n";
    std::cout << "  L1─┐             ┌─R1\n";
    std::cout << "  L2─┤             ├─R2\n";
    std::cout << "  L3─┼─HUB-LEFT─SW-CORE─HUB-RIGHT─┼─R3\n";
    std::cout << "  L4─┤             ├─R4\n";
    std::cout << "  L5─┘             └─R5\n\n";

    // ── Left side communication ──────────────────
    std::cout << "── Intra-left: L1 signals to all via HUB-LEFT ───────\n";
    leftPCs[0]->sendSignal("LeftBroadcast");

    // ── Cross-hub via switch ─────────────────────
    std::cout << "\n── Cross-hub: L2 → R3 (via HUB-LEFT → SW-CORE → HUB-RIGHT)\n";
    std::cout << "   Note: Switch learns HUB-LEFT port → floods to HUB-RIGHT\n";
    std::cout << "   HUB-RIGHT then floods to all 5 right-side PCs.\n";
    leftPCs[1]->sendSignal("CrossHubMessage");

    // ── Domain analysis ──────────────────────────
    sw->collisionDomains = static_cast<int>(sw->connections.size());
    std::cout << "\n── Domain Analysis ───────────────────────────────────\n";
    std::cout << "  HUB-LEFT  → all 5 left PCs share 1 collision domain\n";
    std::cout << "  HUB-RIGHT → all 5 right PCs share 1 collision domain\n";
    std::cout << "  SW-CORE   → 2 ports = 2 collision domains (segments)\n";
    std::cout << "\n  TOTAL Collision Domains : 2 (one per hub segment)\n";
    std::cout << "  TOTAL Broadcast Domains : 1 (switch does NOT segment broadcast)\n";
    std::cout << "\n  Note: A Router would create 2 broadcast domains.\n";
    std::cout << "        A Switch alone keeps it at 1.\n";
}

// ─────────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────────
int main() {
    system("chcp 65001 > nul");
    srand(static_cast<unsigned>(time(nullptr)));

    std::cout << "╔═══════════════════════════════════════════════════════╗\n";
    std::cout << "║       NETWORK PROTOCOL STACK SIMULATOR v1.0           ║\n";
    std::cout << "║          Physical + Data Link Layer            ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════╝\n";

    int choice = 0;
    while (true) {
        std::cout << "\n┌─── MENU ──────────────────────────────────────────┐\n";
        std::cout << "│  1. Point-to-Point (2 devices, physical layer)    │\n";
        std::cout << "│  2. Hub Star topology (5 devices)                 │\n";
        std::cout << "│  3. Switch + 5 devices (full DLL demo)            │\n";
        std::cout << "│  4. Two Hub-Stars via Switch (10 devices)         │\n";
        std::cout << "│  5. Run ALL tests                                 │\n";
        std::cout << "│  0. Exit                                          │\n";
        std::cout << "└───────────────────────────────────────────────────┘\n";
        std::cout << "Choice: ";
        std::cin >> choice;

        switch (choice) {
            case 1: test1_pointToPoint();      break;
            case 2: test2_hubStar();           break;
            case 3: test3_switchFiveDevices(); break;
            case 4: test4_twoStarsViaSwitch(); break;
            case 5:
                test1_pointToPoint();
                test2_hubStar();
                test3_switchFiveDevices();
                test4_twoStarsViaSwitch();
                break;
            case 0:
                std::cout << "\nExiting simulator. Goodbye!\n";
                return 0;
            default:
                std::cout << "Invalid choice.\n";
        }
    }
}