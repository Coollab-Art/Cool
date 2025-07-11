#include "DumpStringGenerator.hpp"

namespace Cool {

static auto spaces(size_t n) -> std::string
{
    std::string res;
    res.resize(n, ' ');
    return res;
}

auto DumpStringGenerator::generate() const -> std::string
{
    auto res = ""s;
    res += "---INFO---\n";

    size_t max_key_size = 0;
    for (auto const& [key, val] : _entries)
        max_key_size = std::max(max_key_size, key.size());

    for (auto const& [key, val] : _entries)
    {
        res += key;
        res += spaces(max_key_size - key.size());
        res += " : ";
        res += val;
        res += '\n';
    }

    res += "----------";
    return res;
}

} // namespace Cool