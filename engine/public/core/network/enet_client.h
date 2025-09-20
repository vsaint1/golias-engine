#pragma once

#include "enet_server.h"


class ENetClient {
public:
    ENetClient();
    ~ENetClient();

    bool connect(const char* host, int port);
    void disconnect();
    void poll();

    [[nodiscard]] bool is_connected() const;


    template <typename T>
    void rpc(const std::string& method, const T& arg);
    void rpc(const std::string& method, const std::vector<uint8_t>& args);
    void rpc(const std::string& method, const std::string& arg);

    std::function<void()> on_connected;
    std::function<void()> on_disconnected;

    void register_rpc(const std::string& method, std::function<void(const std::vector<uint8_t>&)> handler);

    ENetClient(const ENetClient&)            = delete;
    ENetClient& operator=(const ENetClient&) = delete;
private:
    ENetHost* client;
    ENetPeer* peer;

    std::unordered_map<std::string, std::function<void(const std::vector<uint8_t>&)>> rpc_methods;

    void handle_packet(ENetPacket* packet);
    void handle_rpc_packet(const uint8_t* payload, uint16_t size);


};

template <typename T>
void ENetClient::rpc(const std::string& method, const T& arg) {
    std::vector<uint8_t> payload(sizeof(T));
    SDL_memcpy(payload.data(), &arg, sizeof(T));
    rpc(method, payload);
}
