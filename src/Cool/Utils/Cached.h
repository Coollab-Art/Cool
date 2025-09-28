#pragma once

namespace Cool {

template<typename T>
class Cached {
public:
    /// Returns the last computed value,
    /// or computes a new one using `compute_value` if `invalidate_cache()` has been called after the previous call to `get_value()`.
    auto get_value(std::function<T()> const& compute_value) -> T const&
    {
        if (!_value.has_value())
            _value = compute_value();
        return *_value;
    }

    /// Returns the last computed value,
    /// or computes a new one using `compute_value` if `invalidate_cache()` has been called after the previous call to `get_value()`.
    auto get_value(std::function<void(T&)> const& compute_value) -> T const&
    {
        if (!_value.has_value())
        {
            _value.emplace();
            compute_value(*_value);
        }
        return *_value;
    }

    void invalidate_cache() { _value.reset(); }

private:
    std::optional<T> _value{};
};

} // namespace Cool