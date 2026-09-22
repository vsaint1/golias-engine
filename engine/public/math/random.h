#pragma once

#include <cstdint>

namespace golias {

    class Random {
    public:
        static Random& GetInstance();

        uint32_t NextUint();
        float NextFloat();
        float NextSigned();

        int Range(int min, int max);
        float Range(float min, float max);

    private:
        Random();

        uint64_t GenerateRandomSeed();

        uint64_t mState = 0;
    };

} // namespace golias
