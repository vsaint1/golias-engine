#include "core/renderer/opengl/ember_gl.h"

struct PlayerPosPacket {
    int x = 0;
    int y = 0;
};


enum class PacketType : uint8_t {
    MESSAGE   = 1,
    PLAYER_POS = 2,
};

int main() {
    if (!GEngine->initialize_server("127.0.0.1", 1234)) {
        return SDL_APP_FAILURE;
    }

    GEngine->on_message(static_cast<uint8_t>(PacketType::MESSAGE), [](ENetPeer* peer, const Packet& packet) {
        std::string msg(packet.data.begin(), packet.data.end());
        LOG_INFO("Received message from peer %u: %s", peer->incomingPeerID, msg.c_str());
    });

    GEngine->on_message(static_cast<uint8_t>(PacketType::PLAYER_POS), [](ENetPeer* peer, const Packet& packet) {
        if (packet.data.size() != sizeof(PlayerPosPacket)) {
            LOG_WARN("Invalid PlayerPosPacket size from peer %u", peer->incomingPeerID);
            return;
        }

        PlayerPosPacket pos{};
        SDL_memcpy(&pos, packet.data.data(), sizeof(PlayerPosPacket));
        LOG_INFO("Player %u position: (%d, %d)", peer->incomingPeerID, pos.x, pos.y);
    });

    while (GEngine->is_running) {
        GEngine->time_manager()->update();

        const double dt = GEngine->time_manager()->get_delta_time();

        GEngine->update_server(dt);
    }

    GEngine->shutdown_server();

    return 0;
}
