#pragma once

#include <bit>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>

namespace physics_sim
{
inline constexpr std::uint32_t StateDigestFormatVersion = 2;

enum class StateDigestDomain : std::uint32_t
{
    SceneDefinition = 1,
    PhysicsRuntime = 2,
    RenderedOutput = 3,
};

class StateDigestBuilder
{
  public:
    explicit StateDigestBuilder(StateDigestDomain domain) noexcept
    {
        mix_u64(0x5053494D44494745ULL); // "PSIMDIGE"
        mix_u64(StateDigestFormatVersion);
        mix_enum(domain);
    }

    void mix_u64(std::uint64_t value) noexcept
    {
        for (int byte = 0; byte < 8; ++byte)
        {
            hash_ ^= static_cast<std::uint8_t>((value >> (byte * 8)) & 0xffU);
            hash_ *= 1099511628211ULL;
        }
    }

    void mix_i64(std::int64_t value) noexcept
    {
        mix_u64(static_cast<std::uint64_t>(value));
    }
    void mix_bool(bool value) noexcept
    {
        mix_u64(value ? 1U : 0U);
    }
    void mix_float(float value) noexcept
    {
        mix_u64(std::bit_cast<std::uint32_t>(value));
    }
    void mix_double(double value) noexcept
    {
        mix_u64(std::bit_cast<std::uint64_t>(value));
    }

    template <typename Enum> void mix_enum(Enum value) noexcept
    {
        static_assert(std::is_enum_v<Enum>);
        using Underlying = std::underlying_type_t<Enum>;
        mix_u64(static_cast<std::uint64_t>(static_cast<Underlying>(value)));
    }

    void mix_string(std::string_view value) noexcept
    {
        mix_u64(value.size());
        for (const unsigned char byte : value)
        {
            hash_ ^= byte;
            hash_ *= 1099511628211ULL;
        }
    }

    [[nodiscard]] std::string finish() const
    {
        std::ostringstream stream;
        stream << std::uppercase << std::hex << std::setw(16) << std::setfill('0') << hash_;
        return stream.str();
    }

  private:
    std::uint64_t hash_ = 14695981039346656037ULL;
};
} // namespace physics_sim
