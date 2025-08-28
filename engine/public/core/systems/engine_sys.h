#pragma once


class EngineSystem {

public:
    virtual ~EngineSystem() = default;

    virtual bool initialize() = 0;

    virtual void update(double delta_time) = 0;

    virtual void shutdown() = 0;

    [[nodiscard]] const char* get_name() const {
        return name;
    }

protected:
    const char* name = "EngineSystem";
};
