#pragma once

/*!
 * @brief Base class for all engine systems.
 *
 * @details All engine systems should inherit from this class and implement the virtual methods.
 *
 * @version 1.1.0
 */
class EngineManager {

public:
    virtual ~EngineManager() = default;

    virtual bool initialize() = 0;

    virtual void update(double delta_time = 0) = 0;

    virtual void shutdown() = 0;

    [[nodiscard]] const char* get_name() const {
        return name;
    }

protected:
    const char* name = "EngineSystem";
};
