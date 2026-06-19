#include <Cool/ImGui/ImGuiExtras.h>
#include <stringify/stringify.hpp>
#include "Cool/ImGui/IcoMoonCodepoints.h"
#include "Cool/ImGui/icon_fmt.h"

namespace Cool {

template<typename T>
void Variable<T>::imgui_reset_buttons(ImGuiVariableCallbacks const& callbacks)
{
    ImGuiExtras::disabled_if(_value == _default_value, "Disabled because it is already equal to the default value", [&]() {
        if (ImGui::Button(("Reset to default value (" + stringify(_default_value) + ")").c_str()))
        {
            _value = _default_value;
            callbacks.on_value_changed();
            callbacks.on_value_editing_finished();
            ImGui::CloseCurrentPopup();
        }
    });

    if (_metadata != _default_metadata)
    {
        ImGui::SameLine();
        if (ImGui::Button("Reset Metadata"))
        {
            _metadata = _default_metadata;
            callbacks.on_metadata_changed();
            callbacks.on_metadata_editing_finished();
        }
    }
}

template<typename T>
void Variable<T>::imgui(ImGuiVariableCallbacks const& callbacks)
{
    ImGui::PushID(this);
    ImGui::BeginGroup();
    {
        if (imgui_widget(*this))
            callbacks.on_value_changed();

        ImGui::SameLine();
        ImGui::Dummy(ImVec2(5.0f, 0.0f));
        ImGui::SameLine();

        ImGuiExtras::disabled_if(_value == _default_value, "Disabled because it is already equal to the default value", [&]() {
            if (ImGui::Button("  " ICOMOON_UNDO2))
            {
                _value = _default_value;
                callbacks.on_value_changed();
                callbacks.on_value_editing_finished();
            }
            ImGui::SetItemTooltip("%s", ("Reset to default value (" + stringify(_default_value) + ")").c_str());
        });
    }
    ImGui::EndGroup();
    ImGui::PopID();
    if (ImGui::IsItemDeactivatedAfterEdit())
        callbacks.on_value_editing_finished();

    if (ImGui::BeginPopupContextItem(_name.c_str()))
    {
        ImGui::PushID(this + 1);
        ImGui::BeginGroup();
        {
            if (imgui_widget(_metadata))
                callbacks.on_metadata_changed();
        }
        ImGui::EndGroup();
        ImGui::PopID();
        if (ImGui::IsItemDeactivatedAfterEdit())
            callbacks.on_metadata_editing_finished();

        imgui_reset_buttons(callbacks);

        ImGui::EndPopup();
    }
}

} // namespace Cool