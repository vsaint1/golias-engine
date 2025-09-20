#include "core/network/enet_server.h"

#include "core/engine.h"


ENetServer::ENetServer() : Server() {
}

ENetServer::~ENetServer() {
    shutdown();
}

bool ENetServer::initialize(const char* host, int port) {
    LOG_INFO("[SERVER] - Initializing server on port: %d", port);

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

#if !defined(SDL_PLATFORM_EMSCRIPTEN)
    curl_global_init(CURL_GLOBAL_DEFAULT);
#endif

    Logger::initialize();

    if (enet_initialize() != 0) {
        LOG_ERROR("[SERVER] - Failed to initialize ENet");
        return false;
    }

    ENetAddress addr;
    if (host == nullptr || SDL_strcmp(host, "0.0.0.0") == 0) {
        addr.host = ENET_HOST_ANY;
    } else {
        if (enet_address_set_host(&addr, host) != 0) {
            LOG_ERROR("[SERVER] - Failed to set host address: %s", host);
            enet_deinitialize();
            return false;
        }
    }

    addr.port = port;

    constexpr int E_MAX_CLIENTS  = 32;
    constexpr int E_MAX_CHANNELS = 2;

    Server.host    = enet_host_create(&addr, E_MAX_CLIENTS, E_MAX_CHANNELS, 0, 0);
    Server.address = addr;

    if (!Server.host) {
        LOG_ERROR("[SERVER] - Failed to create ENet host!");
        enet_deinitialize();
        return false;
    }

    LOG_INFO("[SERVER] - Server initialized on %s:%d", host ? host : "0.0.0.0", Server.address.port);

    return true;
}

void ENetServer::shutdown() {
    LOG_INFO("[SERVER] - Shutting down server");

    if (Server.host) {
        for (size_t i = 0; i < Server.host->peerCount; i++) {
            ENetPeer* peer = &Server.host->peers[i];
            if (peer->state == ENET_PEER_STATE_CONNECTED) {
                enet_peer_disconnect(peer, 0);
            }
        }

        ENetEvent event;
        while (enet_host_service(Server.host, &event, 1000) > 0) {
            if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
                LOG_INFO("[SERVER] - Peer disconnected during shutdown");
            }
        }

        enet_host_destroy(Server.host);
        Server.host = nullptr;
    }

    enet_deinitialize();
    Logger::destroy();

#if !defined(SDL_PLATFORM_EMSCRIPTEN)
    curl_global_cleanup();
#endif
}

void ENetServer::poll() {
    if (!Server.host) {
        return;
    }

    ENetEvent event;
    while (enet_host_service(Server.host, &event, 0) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            LOG_INFO("[SERVER] - Peer connected: %s", decode_peer(event.peer->connectID));
            if (on_peer_connected) {
                on_peer_connected(event.peer);
            }
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            LOG_INFO("[SERVER] - Received packet of length %zu from peer %s:%d on channel %u", event.packet->dataLength,
                     decode_peer(event.peer->connectID), event.peer->address.port, event.channelID);
            handle_packet(event.peer, event.packet);
            enet_packet_destroy(event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            LOG_INFO("[SERVER] - Peer disconnected: %u", event.peer->connectID);
            if (on_peer_disconnected) {
                on_peer_disconnected(event.peer);
            }
            break;

        default:
            break;
        }
    }
}

void ENetServer::broadcast(uint8_t type, const std::string& message) {
    if (!Server.host) {
        return;
    }

    std::vector<uint8_t> payload(message.begin(), message.end());
    uint16_t size = static_cast<uint16_t>(payload.size());

    // Create packet with header [type][size_high][size_low][payload...]
    std::vector<uint8_t> packet;
    packet.reserve(3 + payload.size());
    packet.push_back(type);
    packet.push_back(static_cast<uint8_t>((size >> 8) & 0xFF));
    packet.push_back(static_cast<uint8_t>(size & 0xFF));
    packet.insert(packet.end(), payload.begin(), payload.end());

    ENetPacket* enet_p = enet_packet_create(packet.data(), packet.size(), ENET_PACKET_FLAG_RELIABLE);
    if (enet_p) {
        enet_host_broadcast(Server.host, 0, enet_p);
        enet_host_flush(Server.host);
        LOG_INFO("[SERVER] - Broadcasted message type=%u size=%u", type, size);
    }
}

void ENetServer::rpc(ENetPeer* peer, const std::string& method, const std::vector<uint8_t>& args) {
    if (!peer || peer->state != ENET_PEER_STATE_CONNECTED) {
        LOG_ERROR("[SERVER] - Cannot send RPC to invalid/disconnected peer");
        return;
    }

    // Build RPC payload: [method_size_high][method_size_low][method][args...]
    std::vector<uint8_t> rpc_payload;
    uint16_t method_size = static_cast<uint16_t>(method.size());

    rpc_payload.reserve(2 + method.size() + args.size());
    rpc_payload.push_back(static_cast<uint8_t>((method_size >> 8) & 0xFF));
    rpc_payload.push_back(static_cast<uint8_t>(method_size & 0xFF));
    rpc_payload.insert(rpc_payload.end(), method.begin(), method.end());
    rpc_payload.insert(rpc_payload.end(), args.begin(), args.end());

    // Build full packet: [type=1][payload_size_high][payload_size_low][rpc_payload...]
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
        enet_host_flush(Server.host);
        LOG_INFO("[SERVER] - Sent RPC '%s' to peer %u", method.c_str(), peer->connectID);
    }
}

void ENetServer::rpc_all(const std::string& method, const std::vector<uint8_t>& args) {
    if (!Server.host) {
        return;
    }

    for (size_t i = 0; i < Server.host->peerCount; i++) {
        ENetPeer* peer = &Server.host->peers[i];
        if (peer->state == ENET_PEER_STATE_CONNECTED) {
            rpc(peer, method, args);
        }
    }
}

void ENetServer::register_rpc(const std::string& method, std::function<void(ENetPeer*, const std::vector<uint8_t>&)> handler) {
    rpc_methods[method] = handler;
}

void ENetServer::handle_packet(ENetPeer* peer, ENetPacket* packet) {
    if (!packet || !peer) {
        return;
    }

    LOG_INFO("[SERVER] - Processing packet, length=%zu", packet->dataLength);

    // Min. packet (header = 3 bytes)
    if (packet->dataLength < 3) {
        LOG_WARN("[SERVER] - Dropped: packet too small (<3 bytes)");
        return;
    }

    uint8_t type  = packet->data[0];
    uint16_t size = (static_cast<uint16_t>(packet->data[1]) << 8) | packet->data[2];

    LOG_INFO("[SERVER] - Packet type=%u payload_size=%u", type, size);


    if (packet->dataLength < static_cast<size_t>(size + 3)) {
        LOG_WARN("[SERVER] - Dropped: size mismatch (expected %u, got %zu)", size + 3, packet->dataLength);
        return;
    }

    const uint8_t* payload = packet->data + 3;

    if (type == 1) {
        handle_rpc_packet(peer, payload, size);
    } else {
        LOG_WARN("[SERVER] - No handler for packet type: %u", type);
    }
}

void ENetServer::handle_rpc_packet(ENetPeer* peer, const uint8_t* payload, uint16_t size) {
    if (size < 2) {
        LOG_WARN("[SERVER] - Dropped RPC: payload too small");
        return;
    }

    uint16_t method_len = (static_cast<uint16_t>(payload[0]) << 8) | payload[1];
    // LOG_INFO("[SERVER] - RPC method_len=%u", method_len);

    if (method_len + 2 > size) {
        LOG_WARN("[SERVER] - Dropped RPC: method_len=%u exceeds payload size=%u", method_len, size);
        return;
    }

    std::string method(reinterpret_cast<const char*>(payload + 2), method_len);
    std::vector<uint8_t> args(payload + 2 + method_len, payload + size);

    LOG_INFO("[SERVER] - RPC call method='%s' method_len=%u args_size=%zu", method.c_str(),method_len, args.size());

    auto it = rpc_methods.find(method);
    if (it != rpc_methods.end()) {
        LOG_INFO("[SERVER] - Dispatching RPC to handler");
        it->second(peer, args);
    } else {
        LOG_WARN("[SERVER] - Unknown RPC method: %s", method.c_str());
    }
}

const char* ENetServer::decode_peer(enet_uint32 peer) const {
    static char host_str[16];
    SDL_snprintf(host_str, sizeof(host_str), "%u.%u.%u.%u", (peer) & 0xFF, (peer >> 8) & 0xFF, (peer >> 16) & 0xFF, (peer >> 24) & 0xFF);
    return host_str;
}
