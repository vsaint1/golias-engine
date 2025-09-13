#include "core/network/enet_client.h"

ENetClient::ENetClient(const std::string& host, uint16_t port) {
    if (enet_initialize() != 0) {
        LOG_ERROR("[CLIENT] - Failed to initialize ENetClient");
        return;
    }

    client = enet_host_create(nullptr, 1, 2, 0, 0);
    if (!client) {
        LOG_ERROR("[CLIENT] - Failed to create ENet client!");
        return;
    }

    ENetAddress address;
    enet_address_set_host(&address, host.c_str());
    address.port = port;

    peer = enet_host_connect(client, &address, 2, 0);

    constexpr  int E_CONNECT_TIMEOUT_MS = 5000;

    ENetEvent event;
    if (enet_host_service(client, &event, E_CONNECT_TIMEOUT_MS) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        LOG_INFO("[CLIENT] - Connected to %s:%d", host.c_str(), port);
    } else {
        LOG_ERROR("[CLIENT] - Failed while trying connect to %s:%d", host.c_str(), port);
        enet_peer_reset(peer);
    }
}


void ENetClient::pool() {
}


ENetClient::~ENetClient() {
    if (peer) {
        enet_peer_disconnect(peer, 0);
        peer = nullptr;
    }

    if (client) {
        enet_host_destroy(client);
        client = nullptr;
    }

    enet_deinitialize();
}


void ENetClient::send_packet(uint8_t type, const std::vector<uint8_t>& payload) {
    uint16_t size = static_cast<uint16_t>(payload.size());
    uint8_t header[3];
    header[0] = type;
    header[1] = (size >> 8) & 0xFF;
    header[2] = size & 0xFF;

    std::vector<uint8_t> packet;
    packet.insert(packet.end(), header, header + 3);
    packet.insert(packet.end(), payload.begin(), payload.end());

    ENetPacket* p = enet_packet_create(packet.data(), packet.size(), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, p);
    enet_host_flush(client);
}
