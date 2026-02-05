#pragma once

#include "behaviour.h"

namespace golias {

    // NativeBehaviour is a base class for C++ script-based game objects.
    // Inherits lifecycle methods (Awake, Start, Update, OnDestroy) from Behaviour.
    class NativeBehaviour : public Behaviour {
    public:
        virtual ~NativeBehaviour() = default;

    protected:
        NativeBehaviour() = default;
    };

} // namespace golias
