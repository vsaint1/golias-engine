#pragma once

#include "vector.h"
#include <array>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

template <typename T>
using Vector = TypedArray<T>;

template<typename K, typename V>
using HashMap = std::unordered_map<K, V>;

template<typename K, typename V>
using Map = std::map<K, V>;

template<typename T>
using HashSet = std::unordered_set<T>;

template<typename T>
using Set = std::set<T>;

template<typename T, size_t N>
using Array = std::array<T, N>;

template<typename T>
using Queue = std::queue<T>;

template<typename T>
using Stack = std::stack<T>;

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T>
using WeakRef = std::weak_ptr<T>;

template<typename T>
using Scope = std::unique_ptr<T>;

template <typename T>
using Option = std::optional<T>;

template <typename... Types>
using Variant = std::variant<Types...>;