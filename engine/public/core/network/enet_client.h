#pragma once

#include "core/systems/logging_sys.h"


class ENetClient {

public:
    ENetClient(const std::string& host, uint16_t port);

    ~ENetClient();

    void pool();

    void send(uint8_t type, const std::string& message);

    template <typename T>
    void send(uint8_t type, const T& obj);

private:
    ENetHost* client = nullptr;
    ENetPeer* peer   = nullptr;

    void send_packet(uint8_t type, const std::vector<uint8_t>& payload);
};


template <typename T>
void ENetClient::send(uint8_t type, const T& obj) {
    std::vector<uint8_t> payload(sizeof(T));
    SDL_memcpy(payload.data(), &obj, sizeof(T));
    send_packet(type, payload);
}
