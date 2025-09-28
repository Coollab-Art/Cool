#include "hash_project_path_for_info_folder.hpp"
#include "Cool/File/File.h"

namespace Cool {

//----------------------
// IMPORTANT: Don't ever change this implementation. Both Coollab and the launcher use it to know in which folder the infos of a project are stored.
//            Changing it would break compatibility between Coollab and the launcher, and between various versions of the apps.
//----------------------

static auto fnv128_hash(std::string const& path) -> std::string
{
    // FNV-1a constants for 128-bit
    constexpr std::uint64_t FNV_OFFSET_BASIS_HIGH = 0x6c62272e07bb0142ULL; // High part of 128-bit offset
    constexpr std::uint64_t FNV_OFFSET_BASIS_LOW  = 0x62776e7f868f6d9fULL; // Low part of 128-bit offset
    constexpr std::uint64_t FNV_PRIME             = 1099511628211ULL;

    std::uint64_t hashHigh = FNV_OFFSET_BASIS_HIGH;
    std::uint64_t hashLow  = FNV_OFFSET_BASIS_LOW;

    for (char const character : path)
    {
        auto const byte = static_cast<uint8_t>(character);
        hashLow ^= byte;
        hashLow *= FNV_PRIME;
        if (hashLow < FNV_PRIME)
            hashHigh ^= 1; // Carry over to high part if low part overflows
        hashHigh ^= byte;
        hashHigh *= FNV_PRIME;
    }

    std::ostringstream oss;
    oss << std::hex << std::setw(16) << std::setfill('0') << hashHigh
        << std::setw(16) << std::setfill('0') << hashLow;

    return oss.str();
}

auto hash_project_path_for_info_folder(std::filesystem::path const& file_path) -> std::string
{
    return fnv128_hash(Cool::File::weakly_canonical(file_path).string());
}

} // namespace Cool

#if COOL_ENABLE_TESTS
#include "doctest/doctest.h"

TEST_CASE("The hash of the project path must never change!")
{
    CHECK(Cool::fnv128_hash("some_test_string") == "0f5f4a4b6dfb5e0d0df189c8a29366d0");
}
#endif