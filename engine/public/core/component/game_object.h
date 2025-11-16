#pragma once

#include  "components.h"



/*!
 * @brief  Represents a game object in the ECS world.
 * @ingroup Core
 */
class GameObject {
public:

    GameObject() = default;

    explicit GameObject(const flecs::world& world) : _id(world.entity()) {
    }

    explicit GameObject(const flecs::entity entity) : _id(entity) {
    }

    Uint32 get_id() const;

    bool is_valid() const;

    const char* get_name() const;

    void set_name(const char* name);

    bool compare_tag(const char* tag) const;

    template <typename T, typename... Args>
    T& add_component(Args&&... args);

    template <typename T>
    void add_component() {
        _id.add<T>();
    }

    template <typename T>
    void remove_component() {
        _id.remove<T>();
    }

    template <typename T>
    T* get_component() {
        return const_cast<T*>(_id.try_get<T>());
    }

    void free() const;


    flecs::entity& entity();

    const flecs::entity& entity() const;


    operator flecs::entity() const;

private:
    flecs::entity _id;

};


template <typename T, typename... Args>
T& GameObject::add_component(Args&&... args) {
    if constexpr (sizeof...(Args) == 0) {
        return _id.ensure<T>();
    } else if constexpr (sizeof...(Args) == 1 && (std::is_same_v<std::decay_t<Args>, T> && ...)) {
        _id.set<T>(std::forward<Args>(args)...);
        return _id.ensure<T>();
    } else {
        _id.set<T>(T(std::forward<Args>(args)...));
        return _id.ensure<T>();
    }
}
