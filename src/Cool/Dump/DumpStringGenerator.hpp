#pragma once

namespace Cool {

class DumpStringGenerator {
public:
    auto add(std::string key, std::string value) -> DumpStringGenerator&
    {
        _entries.emplace_back(std::move(key), std::move(value));
        return *this;
    }
    auto generate() const -> std::string;

private:
    std::vector<std::pair<std::string, std::string>> _entries;
};

} // namespace Cool