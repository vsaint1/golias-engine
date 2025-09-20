#include "core/network/enet_server.h"
#include <csignal>

std::atomic<bool> running(true);

void handle_signal(int) {
    running = false;
}

int main() {
    ENetServer server;
    server.initialize("127.0.0.1", 1234);

    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);

    server.on_peer_connected = [&](ENetPeer* peer) {
        LOG_INFO("[SERVER] - Peer connected: %d, Host %u", peer->connectID, peer->address.host);
    };

    server.on_peer_disconnected = [&](ENetPeer* peer) { LOG_INFO("[SERVER] - Peer disconnected: %u", peer->address.host); };

    while (running) {
        server.poll();
        SDL_Delay(16);
    }


    return 0;
}
