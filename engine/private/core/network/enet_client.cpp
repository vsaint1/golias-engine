#include "core/network/enet_client.h"

ENetClient::ENetClient() : client(nullptr), peer(nullptr) {
}

ENetClient::~ENetClient() {
    disconnect();
}

bool ENetClient::connect(const char* host, int port) {
    if (client) {
        LOG_ERROR("[CLIENT] - Already connected or connecting");
        return false;
    }

    if (enet_initialize() != 0) {
        LOG_ERROR("[CLIENT] - Failed to initialize ENet");
        return false;
    }

    client = enet_host_create(nullptr, 1, 2, 0, 0);
    if (!client) {
        LOG_ERROR("[CLIENT] - Failed to create ENet host");
        enet_deinitialize();
        return false;
    }

    ENetAddress addr;
    if (enet_address_set_host(&addr, host) != 0) {
        LOG_ERROR("[CLIENT] - Failed to resolve host: %s", host);
        enet_host_destroy(client);
        client = nullptr;
        enet_deinitialize();
        return false;
    }

    addr.port = port;
    peer = enet_host_connect(client, &addr, 2, 0);

    if (!peer) {
        LOG_ERROR("[CLIENT] - Failed to initiate connection");
        enet_host_destroy(client);
        client = nullptr;
        enet_deinitialize();
        return false;
    }

    LOG_INFO("[CLIENT] - Connecting to %s:%d", host, port);
    return true;
}

void ENetClient::disconnect() {
    if (peer && peer->state == ENET_PEER_STATE_CONNECTED) {
        enet_peer_disconnect(peer, 0);

        ENetEvent event;
        bool disconnected = false;
        int timeout = 3000; // 3 seconds

        while (!disconnected && timeout > 0) {
            while (enet_host_service(client, &event, 100) > 0) {
                if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
                    LOG_INFO("[CLIENT] - Gracefully disconnected");
                    disconnected = true;
                    break;
                }
            }
            timeout -= 100;
        }

        if (!disconnected) {
            LOG_INFO("[CLIENT] - Force disconnecting");
            enet_peer_reset(peer);
        }
    }

    if (client) {
        enet_host_destroy(client);
        client = nullptr;
    }

    peer = nullptr;
    enet_deinitialize();
}

void ENetClient::poll() {
    if (!client) return;

    ENetEvent event;
    while (enet_host_service(client, &event, 0) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            LOG_INFO("[CLIENT] - Connected to server");
            if (on_connected) {
                on_connected();
            }
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            LOG_INFO("[CLIENT] - Received packet of length %zu", event.packet->dataLength);
            handle_packet(event.packet);
            enet_packet_destroy(event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            LOG_INFO("[CLIENT] - Disconnected from server");
            peer = nullptr;
            if (on_disconnected) {
                on_disconnected();
            }
            break;

        default:
            break;
        }
    }
}

bool ENetClient::is_connected() const {
    return peer && peer->state == ENET_PEER_STATE_CONNECTED;
}

void ENetClient::rpc(const std::string& method, const std::vector<uint8_t>& args) {
    if (!peer || peer->state != ENET_PEER_STATE_CONNECTED) {
        LOG_ERROR("[CLIENT] - Cannot send RPC: not connected");
        return;
    }

    // RPC payload: [method_size_high][method_size_low][method][args...]
    std::vector<uint8_t> rpc_payload;
    uint16_t method_size = static_cast<uint16_t>(method.size());

    rpc_payload.reserve(2 + method.size() + args.size());
    rpc_payload.push_back(static_cast<uint8_t>((method_size >> 8) & 0xFF));
    rpc_payload.push_back(static_cast<uint8_t>(method_size & 0xFF));
    rpc_payload.insert(rpc_payload.end(), method.begin(), method.end());
    rpc_payload.insert(rpc_payload.end(), args.begin(), args.end());

    // Packet: [type=1][payload_size_high][payload_size_low][rpc_payload]
    std::vector<uint8_t> packet;
    uint16_t payload_size = static_cast<uint16_t>(rpc_payload.size());

    packet.reserve(3 + rpc_payload.size());
    packet.push_back(1); // RPC type
    packet.push_back(static_cast<uint8_t>((payload_size >> 8) & 0xFF));
    packet.push_back(static_cast<uint8_t>(payload_size & 0xFF));
    packet.insert(packet.end(), rpc_payload.begin(), rpc_payload.end());

    ENetPacket* p = enet_packet_create(packet.data(), packet.size(), ENET_PACKET_FLAG_RELIABLE);
    if (p) {
        enet_peer_send(peer, 0, p);
        enet_host_flush(client);
        LOG_INFO("[CLIENT] - Sent RPC '%s' with %zu bytes of args", method.c_str(), args.size());
    }
}

void ENetClient::rpc(const std::string& method, const std::string& arg) {
    std::vector<uint8_t> args(arg.begin(), arg.end());
    rpc(method, args);
}


void ENetClient::register_rpc(const std::string& method, std::function<void(const std::vector<uint8_t>&)> handler) {
    rpc_methods[method] = handler;
}

void ENetClient::handle_packet(ENetPacket* packet) {
    if (!packet) return;

    LOG_INFO("[CLIENT] - Processing packet, length=%u", packet->dataLength);

    // Validate minimum packet size (header = 3 bytes)
    if (packet->dataLength < 3) {
        LOG_INFO("[CLIENT] - Dropped: packet too small (<3 bytes)");
        return;
    }

    uint8_t type = packet->data[0];
    uint16_t size = (static_cast<uint16_t>(packet->data[1]) << 8) | packet->data[2];

    LOG_INFO("[CLIENT] - Packet type=%u payload_size=%u", type, size);

    // Validate packet size matches header
    if (packet->dataLength < static_cast<size_t>(size + 3)) {
        LOG_INFO("[CLIENT] - Dropped: size mismatch (expected %u, got %u)", size + 3, packet->dataLength);
        return;
    }

    const uint8_t* payload = packet->data + 3;

    // Handle RPC packets (type = 1)
    if (type == 1) {
        handle_rpc_packet(payload, size);
    } else {
        LOG_INFO("[CLIENT] - Unknown packet type: %u", type);
    }
}

void ENetClient::handle_rpc_packet(const uint8_t* payload, uint16_t size) {
    // Validate RPC packet structure
    if (size < 2) {
        LOG_INFO("[CLIENT] - Dropped RPC: payload too small");
        return;
    }

    uint16_t method_len = (static_cast<uint16_t>(payload[0]) << 8) | payload[1];
    LOG_INFO("[CLIENT] - RPC method_len=%u", method_len);

    if (method_len + 2 > size) {
        LOG_INFO("[CLIENT] - Dropped RPC: method_len=%u exceeds payload size=%u", method_len, size);
        return;
    }

    // Extract method name and arguments
    std::string method(reinterpret_cast<const char*>(payload + 2), method_len);
    std::vector<uint8_t> args(payload + 2 + method_len, payload + size);

    LOG_INFO("[CLIENT] - RPC call method='%s' args_size=%zu", method.c_str(), args.size());

    // Dispatch to registered handler
    auto it = rpc_methods.find(method);
    if (it != rpc_methods.end()) {
        LOG_INFO("[CLIENT] - Dispatching RPC to handler");
        it->second(args);
    } else {
        LOG_INFO("[CLIENT] - Unknown RPC method: %s", method.c_str());
    }
}