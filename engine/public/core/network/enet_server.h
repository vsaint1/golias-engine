#pragma once

#include "core/systems/logging_sys.h"



struct ServerState {
    ENetHost* host;
    ENetAddress address;

    ServerState() : host(nullptr), address{} {}
};


class ENetServer {
public:
    ENetServer();
    ~ENetServer();

    bool initialize(const char* host, int port);
    void shutdown();
    void poll();

    void broadcast(uint8_t type, const std::string& message);
    void rpc(ENetPeer* peer, const std::string& method, const std::vector<uint8_t>& args);
    void rpc_all(const std::string& method, const std::vector<uint8_t>& args);

    std::function<void(ENetPeer*)> on_peer_connected;
    std::function<void(ENetPeer*)> on_peer_disconnected;

    void register_rpc(const std::string& method, std::function<void(ENetPeer*, const std::vector<uint8_t>&)> handler);

    ENetServer(const ENetServer&) = delete;
    ENetServer& operator=(const ENetServer&) = delete;
private:
    ServerState Server;

    std::unordered_map<std::string, std::function<void(ENetPeer*, const std::vector<uint8_t>&)>> rpc_methods;

    void handle_packet(ENetPeer* peer, ENetPacket* packet);
    void handle_rpc_packet(ENetPeer* peer, const uint8_t* payload, uint16_t size);

    [[nodiscard]] const char* decode_peer(enet_uint32 peer) const;

};

