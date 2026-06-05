#include <physics_sim/feedback.hpp>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <iostream>

namespace
{
[[noreturn]] void fail(const char* message, const char* file, int line)
{
    std::fprintf(stderr, "FAIL %s:%d: %s\n", file, line, message);
    std::fflush(stderr);
    std::exit(1);
}

#define REQUIRE(condition, message) \
    do \
    { \
        if (!(condition)) \
        { \
            fail((message), __FILE__, __LINE__); \
        } \
    } while (false)
} // namespace

int main()
{
    volatile int zero = 0;
    volatile int one = 1;
    volatile int two = 2;
    volatile int three = 3;

    const float start = physics_sim::feedback_message_alpha(
        std::chrono::milliseconds{zero},
        std::chrono::milliseconds{two * 1000},
        false);
    REQUIRE(std::fabs(start - 1.0f) <= 0.0001f, "feedback alpha should start at full opacity");

    const float mid = physics_sim::feedback_message_alpha(
        std::chrono::milliseconds{one * 1000},
        std::chrono::milliseconds{two * 1000},
        false);
    REQUIRE(mid < 1.0f, "feedback alpha should fade over time");
    REQUIRE(mid > 0.35f, "feedback alpha faded too far before the message expired");

    const float reduced_motion = physics_sim::feedback_message_alpha(
        std::chrono::milliseconds{one * 1000},
        std::chrono::milliseconds{two * 1000},
        true);
    REQUIRE(std::fabs(reduced_motion - 1.0f) <= 0.0001f, "reduced-motion feedback alpha should stay constant");

    const float clamped = physics_sim::feedback_message_alpha(
        std::chrono::milliseconds{three * 1000},
        std::chrono::milliseconds{two * 1000},
        false);
    REQUIRE(clamped >= 0.35f, "feedback alpha should clamp to the minimum readable opacity");

    std::cout << "feedback tests passed" << std::endl;
    return 0;
}
