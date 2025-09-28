#pragma once
#include "Cool/Dependencies/DirtyFlag.h"
#include "Cool/Dependencies/SharedVariableDefinition.h"
#include "Cool/Variables/Variable.h"
#include "Cool/Variables/Variables.h"
#include "ImGuiNotify/ImGuiNotify.hpp"

namespace Cool {

template<typename T>
struct SharedVariableStrongRef {
    std::shared_ptr<Variable<T>> variable; // shared_ptr to make sure the address stays stable in memory and we can reference it in Commands stored in the History + survive through serialization
    DirtyFlag                    dirty_flag;
    DirtyFlag                    secondary_dirty_flag;
    int                          desired_color_space{0}; // HACK in order to know which color space to convert to when sending the value to a shader. Only used by Color input.

    auto id() const -> uintptr_t { return reinterpret_cast<uintptr_t>(variable.get()); } // NOLINT(*reinterpret-cast)

private:
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Variable", variable),
            ser20::make_nvp("Dirty flag", dirty_flag),
            ser20::make_nvp("Secondary dirty flag", secondary_dirty_flag),
            ser20::make_nvp("Desired color space", desired_color_space)
        );
    }
};

template<typename T>
class SharedVariable {
public:
    SharedVariable() = default; // For serialization
    SharedVariable(
        Variable<T>                variable,
        DirtyFlag                  dirty_flag,
        std::optional<std::string> description          = {},
        DirtyFlag                  secondary_dirty_flag = {}
    )
        : _ref{
              std::make_shared<Variable<T>>(variable),
              std::move(dirty_flag),
              std::move(secondary_dirty_flag),
          }
        , _description{std::move(description)}
    {}
    SharedVariable(
        SharedVariableDefinition<T> def,
        DirtyFlag                   dirty_flag,
        DirtyFlag                   secondary_dirty_flag = {}
    )
        : _ref{
              std::make_shared<Variable<T>>(def.var_data),
              std::move(dirty_flag),
              std::move(secondary_dirty_flag),
              def.desired_color_space,
          }
        , _description{def.description}
    {}

    auto name() const -> std::string const& { return _ref.variable->name(); }
    auto value() const -> T const& { return _ref.variable->value(); }
    auto value() -> T& { return _ref.variable->value(); }
    auto dirty_flag() const -> DirtyFlag const& { return _ref.dirty_flag; }
    auto id() const -> uintptr_t { return _ref.id(); }
    auto variable_data() const -> VariableData<T> { return {name(), value(), _ref.variable->metadata()}; }
    auto variable() const -> Variable<T> const& { return *_ref.variable; }
    auto variable() -> Variable<T>& { return *_ref.variable; }
    auto get_ref() const -> SharedVariableStrongRef<T> const& { return _ref; }
    auto get_ref() -> SharedVariableStrongRef<T>& { return _ref; }
    auto description() const -> std::optional<std::string> const& { return _description; }
    auto description() -> std::optional<std::string>& { return _description; }
    auto notification_id() const -> ImGuiNotify::NotificationId& { return _notification_id; }

private:
    SharedVariableStrongRef<T>          _ref;
    std::optional<std::string>          _description; // TODO(Variables) Move to Variable?
    mutable ImGuiNotify::NotificationId _notification_id{};

private:
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            // ser20::make_nvp("Description", _description), // (JF): I don't think there is a need to serialize the description since it will be parsed from the shader each time, and applying presets and the like only affect the value of the variable.
            ser20::make_nvp("Ref", _ref)
        );
    }
};

} // namespace Cool