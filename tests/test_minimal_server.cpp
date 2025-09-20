#include "core/network/enet_server.h"

struct PlayerPosPacket {
    int x = 0;
    int y = 0;
};

int main() {
    ENetServer server;
    server.initialize("127.0.0.1", 1234);

    server.on_peer_connected = [&](ENetPeer* peer) {
        LOG_INFO("[SERVER] - Peer connected: %d, Host %u", peer->connectID, peer->address.host);
    };

    server.on_peer_disconnected = [&](ENetPeer* peer) { LOG_INFO("[SERVER] - Peer disconnected: %u", peer->address.host); };

    server.register_rpc("hello", [](ENetPeer* peer, const std::vector<uint8_t>& args) {
        std::string data(args.begin(), args.end());
        LOG_INFO("[SERVER] - RPC 'hello' called by peer %d with data: %s", peer->connectID, data.c_str());
    });


    while (true) {
        server.poll();
        SDL_Delay(16);
    }


    return 0;
}
