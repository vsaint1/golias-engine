#include "core/component/game_object.h"



Uint32 GameObject::get_id() const {
    return static_cast<Uint32>(_id.id());
}

bool GameObject::is_valid() const {
    return _id.is_valid() && _id.is_alive();
}

const char* GameObject::get_name() const {
    return _id.name().c_str();
}

void GameObject::set_name(const char* name) {
    _id.set_name(name);
}

bool GameObject::compare_tag(const char* tag) const {
    return _id.has<Tag>() && _id.name() == tag;
}

void GameObject::free() const {
    _id.destruct();
}

flecs::entity& GameObject::entity() {
    return _id;
}

const flecs::entity& GameObject::entity() const {
    return _id;
}

GameObject::operator flecs::entity() const {
    return _id;
}
