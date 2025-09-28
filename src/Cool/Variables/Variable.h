#pragma once

namespace Cool {

// TODO(Variable) Make many things in Coollab variables, so that users can customize GUI and reset to default values. This shouldn't only be for nodes params.

template<typename T>
struct VariableMetadata; // You need to create a specialization of this template for each variable type that you want to use.

template<typename T>
struct VariableData {
    std::string         name{};
    T                   value{};
    VariableMetadata<T> metadata{};

    friend auto operator==(VariableData const&, VariableData const&) -> bool = default;

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Name", name),
            ser20::make_nvp("Value", value),
            ser20::make_nvp("Metadata", metadata)
        );
    }
};

struct ImGuiVariableCallbacks {
    std::function<void()> on_value_changed             = []() {};
    std::function<void()> on_metadata_changed          = []() {};
    std::function<void()> on_value_editing_finished    = []() {};
    std::function<void()> on_metadata_editing_finished = []() {};
};

/// A simple object that wraps a value with some metadata,
/// allowing the user to customize the UI,
/// and reset to the default value.
template<typename T>
class Variable {
public:
    explicit Variable(VariableData<T> const& data = {})
        : _name{data.name}
        , _value{data.value}
        , _metadata{data.metadata}
        , _default_value{data.value}
        , _default_metadata{data.metadata}
    {}

    auto name() const -> std::string const& { return _name; }
    auto value() const -> T const& { return _value; }
    auto value() -> T& { return _value; }
    auto metadata() const -> VariableMetadata<T> const& { return _metadata; }
    auto metadata() -> VariableMetadata<T>& { return _metadata; }
    auto default_value() const -> T const& { return _default_value; }
    auto default_value() -> T& { return _default_value; }
    auto default_metadata() const -> VariableMetadata<T> const& { return _default_metadata; }
    auto default_metadata() -> VariableMetadata<T>& { return _default_metadata; }

    /// Calls the corresponding callback if either `value` or `metadata` changes.
    void imgui(ImGuiVariableCallbacks const& callbacks = {});

private:
    void imgui_reset_buttons(ImGuiVariableCallbacks const& callbacks);

private:
    std::string         _name{};
    T                   _value{};
    VariableMetadata<T> _metadata{};
    T                   _default_value{};
    VariableMetadata<T> _default_metadata{};

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Name", _name),
            ser20::make_nvp("Value", _value),
            ser20::make_nvp("Metadata", _metadata),
            ser20::make_nvp("Default Value", _default_value),
            ser20::make_nvp("Default Metadata", _default_metadata)
        );
    }
};

} // namespace Cool

#include "Variable.tpp"