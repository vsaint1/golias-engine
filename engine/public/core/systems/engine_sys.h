#pragma once


class EngineSystem {

public:
    virtual ~EngineSystem() = default;

    virtual bool initialize() = 0;

    virtual void update(double delta_time) = 0;

    virtual void shutdown() = 0;
};
