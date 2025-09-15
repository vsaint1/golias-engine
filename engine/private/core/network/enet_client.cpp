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

    constexpr int E_CONNECT_TIMEOUT_MS = 5000;

    ENetEvent event;
    if (enet_host_service(client, &event, E_CONNECT_TIMEOUT_MS) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        LOG_INFO("[CLIENT] - Connected to %s:%d", host.c_str(), port);
        _is_connected = true;

    } else {
        LOG_ERROR("[CLIENT] - Failed while trying connect to %s:%d", host.c_str(), port);
        enet_peer_reset(peer);
    }
}


void ENetClient::pool() {

    if (!client) {
        return;
    }

    ENetEvent event;
    while (enet_host_service(client, &event, 0) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            _is_connected = true;
            LOG_INFO("[CLIENT] - Connected to server.");
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            {
                if (event.packet->dataLength < 3) {
                    enet_packet_destroy(event.packet);
                    break;
                }

                uint8_t type  = event.packet->data[0];
                uint16_t size = (event.packet->data[1] << 8) | event.packet->data[2];

                if (event.packet->dataLength < size + 3) {
                    enet_packet_destroy(event.packet);
                    break;
                }

                Packet msg{type, {event.packet->data + 3, event.packet->data + 3 + size}};

                if (_handlers.contains(type)) {
                    _handlers[type](event.peer, msg);
                }

                enet_packet_destroy(event.packet);
                break;
            }

        case ENET_EVENT_TYPE_DISCONNECT:
            _is_connected = false;
            LOG_INFO("[CLIENT] - Disconnected from server.");
            break;

        default:
            break;
        }
    }
}


void ENetClient::send(uint8_t type, const std::string& message) {
    if (!peer) {
        return;
    }

    std::vector<uint8_t> payload(message.begin(), message.end());

    uint16_t size     = static_cast<uint16_t>(payload.size());
    uint8_t header[3] = {type, static_cast<uint8_t>((size >> 8) & 0xFF), static_cast<uint8_t>(size & 0xFF)};

    std::vector<uint8_t> packet;
    packet.insert(packet.end(), header, header + 3);
    packet.insert(packet.end(), payload.begin(), payload.end());

    ENetPacket* p = enet_packet_create(packet.data(), packet.size(), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, p);
    enet_host_flush(client);
}

void ENetClient::on_message(uint8_t type, std::function<void(ENetPeer*, const Packet&)> handler) {
    _handlers[type] = handler;
}


ENetClient::~ENetClient() {
    LOG_INFO("[CLIENT] - Shutting down ENet client.");

    if (peer) {
        enet_peer_disconnect(peer, 0);
        peer = nullptr;
    }

    if (client) {
        enet_host_destroy(client);
        client = nullptr;
    }

    _is_connected = false;

    enet_deinitialize();
}


void ENetClient::send_packet(uint8_t type, const std::vector<uint8_t>& payload) {
    uint16_t size = static_cast<uint16_t>(payload.size());

    uint8_t header[3];
    header[0] = type;
    header[1] = (size >> 8) & 0xFF; // HBYTE
    header[2] = size & 0xFF; // LBYTE

    std::vector<uint8_t> packet;
    packet.insert(packet.end(), header, header + 3);
    packet.insert(packet.end(), payload.begin(), payload.end());

    ENetPacket* p = enet_packet_create(packet.data(), packet.size(), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, p);
    enet_host_flush(client);
}
