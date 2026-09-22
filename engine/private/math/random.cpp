#include "math/random.h"

#include <random>

namespace golias {

    namespace {

        // https://en.wikipedia.org/wiki/Permuted_congruential_generator
        constexpr uint64_t kPcg32Multiplier = 6364136223846793005ULL;
        constexpr uint64_t kPcg32Increment  = 1442695040888963407ULL;
        constexpr uint64_t kPcg32Seed       = 0x853C49E6748FEA9BULL;

    } // namespace

    Random::Random() {
        // mState = GenerateRandomSeed();
        mState = kPcg32Seed;
    }

    Random& Random::GetInstance() {
        static Random instance;
        return instance;
    }

    uint32_t Random::NextUint() {
        const uint64_t oldState = mState;
        mState                  = oldState * kPcg32Multiplier + kPcg32Increment;

        const uint32_t xorshifted = static_cast<uint32_t>(((oldState >> 18u) ^ oldState) >> 27u);
        const uint32_t rotation   = static_cast<uint32_t>(oldState >> 59u);
        return (xorshifted >> rotation) | (xorshifted << ((32u - rotation) & 31u));
    }

    float Random::NextFloat() {
        return static_cast<float>(NextUint() & 0x00FFFFFFu) * 1.0f / 16777216.0f;
    }

    int Random::Range(int min, int max) {
        return min + (NextUint() % (max - min + 1));
    }

    float Random::Range(float min, float max) {
        return min + (NextFloat() * (max - min));
    }

    float Random::NextSigned() {
        return NextFloat() * 2.0f - 1.0f;
    }

    uint64_t Random::GenerateRandomSeed() {
        std::random_device rd;
        return (static_cast<uint64_t>(rd()) << 32) | static_cast<uint64_t>(rd());
    }
} // namespace golias
