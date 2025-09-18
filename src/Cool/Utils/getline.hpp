#pragma once

namespace Cool {

inline auto getline(std::istream& stream, std::string& line) -> auto&
{
    auto& res = std::getline(stream, line);
    if (!line.empty() && line.back() == '\r') // Remove the extra \r that std::getline might leave when reading a windows-style file (with lines ending with \r\n) on Linux (which expects lines to end with \n only)
        line.pop_back();
    return res;
}

} // namespace Cool