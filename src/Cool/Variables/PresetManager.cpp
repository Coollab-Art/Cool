#include "PresetManager.h"
#include <Cool/ImGui/ImGuiExtras.h>
#include <imgui.h>
#include "Cool/Log/boxer_show.hpp"
#include "Cool/Variables/PresetManager.h"
#include "Cool/internal/Serialization/SPresetManager.h"

// TODO(JF) In case of a change of Settings definition show a merge window that allows user to explicit the link between old and new names (for each old name that doesn't have a match in the new names, show a dropdown that allows to link it to one of the new names that don't correspond to any old names)

namespace Cool {

PresetManager::PresetManager(PresetsPaths const& paths)
    : _serializer{paths.user_defined_presets, "Presets"}
{
    auto const maybe_err = do_load(_user_defined_presets, _serializer);
    std::ignore          = maybe_err; // Ignore errors when file not found
    do_load_default_presets(_default_presets, paths.default_presets);
}

auto PresetManager::find_preset(std::function<bool(const Preset&)> const& predicate) const -> PresetId
{
    {
        std::shared_lock lock{_default_presets.mutex()};
        for (auto const& [id, preset] : _default_presets)
        {
            if (predicate(preset))
                return id;
        }
    }
    {
        std::shared_lock lock{_user_defined_presets.mutex()};
        for (auto const& [id, preset] : _user_defined_presets)
        {
            if (predicate(preset))
                return id;
        }
    }
    return PresetId{};
}

auto PresetManager::find_preset_with_given_values(Settings_ConstRef values) const -> PresetId
{
    return find_preset([&](const Preset& preset) {
        return values.are_equal_to(preset.values);
    });
}

auto PresetManager::find_preset_with_given_name(std::string_view name) const -> PresetId
{
    return find_preset([&](const Preset& preset) {
        return preset.name == name;
    });
}

auto PresetManager::add(Preset preset, bool show_warning_messages) -> PresetId
{
    if (find_preset_with_given_values(preset.values) != PresetId{})
    {
        if (show_warning_messages)
        {
            boxer_show(
                "There is already a preset with the same values.",
                "Adding failed",
                boxer::Style::Warning,
                boxer::Buttons::OK
            );
        }
        return PresetId{};
    }
    else if (find_preset_with_given_name(preset.name) != PresetId{})
    {
        if (show_warning_messages)
        {
            boxer_show(
                "There is already a preset with the same name.",
                "Adding failed",
                boxer::Style::Warning,
                boxer::Buttons::OK
            );
        }
        return PresetId{};
    }
    else
    {
        auto id = _user_defined_presets.create_raw(preset);
        save_to_file();
        return id;
    }
}

void PresetManager::remove(const PresetId& id)
{
    // _default_presets.destroy(id); // Users are not allowed to destroy default presets (+ they are likely stored alongside the executable, e.g. in "Program Files" on Windows, where the app is not allowed to edit files.)
    _user_defined_presets.destroy(id);
    save_to_file();
}

void PresetManager::edit(PresetId const& id, Settings const& new_values)
{
    if (find_preset_with_given_values(new_values) == PresetId{}) // Make sure there isn't already a preset with the same values.
    {
        _user_defined_presets.with_mutable_ref(id, [&](Preset& preset) {
            preset.values = new_values;
        });
        save_to_file();
    }
    else
    {
        boxer_show(
            "There is already a preset with the same values.",
            "Editing failed",
            boxer::Style::Warning,
            boxer::Buttons::OK
        );
    }
}

void PresetManager::rename(const PresetId& id, std::string_view new_name)
{
    const auto do_rename = [&]() {
        _user_defined_presets.with_mutable_ref(id, [&](Preset& preset) {
            preset.name = new_name;
        });
        save_to_file();
    };

    const auto preset_with_same_name = find_preset_with_given_name(new_name);

    if (preset_with_same_name == PresetId{}) // Make sure there isn't already a preset with the same name.
    {
        do_rename();
    }
    else if (preset_name(id) != new_name) // Don't warn if we are trying to rename a preset with its current name
    {
        const auto choice = boxer_show(
            fmt::format("There is already a preset called \"{}\", do you want to overwrite it?", new_name).c_str(),
            "Are you sure?",
            boxer::Style::Warning,
            boxer::Buttons::OKCancel
        );
        if (choice == boxer::Selection::OK)
        {
            remove(preset_with_same_name);
            do_rename();
        }
    }
}

auto PresetManager::preset_name(const PresetId& id) const -> std::optional<std::string>
{
    // Default return value
    std::optional<std::string> name{};

    // Replace nullopt with preset name if it is found in the registry
    _default_presets.with_ref(id, [&](const Preset& preset) {
        name = preset.name;
    });
    _user_defined_presets.with_ref(id, [&](const Preset& preset) {
        name = preset.name;
    });

    return name;
}

auto PresetManager::apply(const PresetId& id, Settings_Ref settings) const -> bool
{
    return _default_presets.with_ref(id, [&](const Preset& preset) {
        settings.assign_from(preset.values);
    }) || _user_defined_presets.with_ref(id, [&](const Preset& preset) {
        settings.assign_from(preset.values);
    });
}

void PresetManager::apply_first_preset_if_there_is_one(Settings_Ref settings) const
{
    {
        std::shared_lock lock{_default_presets.mutex()};
        if (!_default_presets.is_empty())
        {
            settings.assign_from(_default_presets.begin()->second.values);
            return;
        }
    }
    {
        std::shared_lock lock{_user_defined_presets.mutex()};
        if (!_user_defined_presets.is_empty())
        {
            settings.assign_from(_user_defined_presets.begin()->second.values);
            return;
        }
    }
}

auto PresetManager::dropdown(
    std::string_view dropdown_name,
    std::string_view current_preset_name,
    ImGuiComboFlags  combo_flags,
    ImDrawFlags      draw_flags
) -> PresetId
{
    auto selected_id = PresetId{};

    if (ImGui::BeginCombo(
            dropdown_name.data(),
            current_preset_name.data(),
            combo_flags,
            draw_flags
        ))
    {
        for (const auto& [id, preset] : _default_presets)
        {
            ImGui::PushID(&preset);
            const bool is_selected = _current_preset_id == id;
            if (ImGui::Selectable(preset.name.c_str(), is_selected))
                selected_id = id;
            if (is_selected) // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                ImGui::SetItemDefaultFocus();
            ImGui::PopID();
        }
        for (const auto& [id, preset] : _user_defined_presets)
        {
            ImGui::PushID(&preset);
            const bool is_selected = _current_preset_id == id;
            if (ImGui::Selectable(preset.name.c_str(), is_selected))
                selected_id = id;
            if (is_selected) // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                ImGui::SetItemDefaultFocus();
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    return selected_id;
}

void PresetManager::name_selector()
{
    const auto id = dropdown(
        "New preset name",
        "",
        ImGuiComboFlags_NoPreview | ImGuiComboFlags_PopupAlignLeft, // Draw just the arrow of the dropdown
        ImDrawFlags_RoundCornersRight
    );

    // Update _new_preset_name if a preset was selected in the dropdown
    _default_presets.with_ref(id, [&](const Preset& preset) {
        _new_preset_name = preset.name;
    });
    _user_defined_presets.with_ref(id, [&](const Preset& preset) {
        _new_preset_name = preset.name;
    });
}

static void delete_button(const PresetId& id, std::string_view name, PresetManager& preset_manager)
{
    if (ImGui::Button("Delete Preset"))
    {
        const auto choice = boxer_show(
            fmt::format("You are about to delete \"{}\", are you sure?", name).c_str(),
            "Are you sure?",
            boxer::Style::Warning,
            boxer::Buttons::OKCancel
        );
        if (choice == boxer::Selection::OK)
        {
            preset_manager.remove(id);
        }
    }
}

void PresetManager::RenamerWidget::imgui(const PresetId& id, std::string_view current_name, PresetManager& preset_manager)
{
    const bool was_open_last_frame{_popup_is_open};
    _popup_is_open = ImGuiExtras::begin_popup_context_menu_from_button("Rename");
    if (_popup_is_open)
    {
        if (!was_open_last_frame)
        {
            _new_name = current_name;
            ImGui::SetKeyboardFocusHere();
        }
        if (ImGui::InputText("##645", &_new_name, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if (was_open_last_frame && !_popup_is_open) // Was just closed
    {
        preset_manager.rename(id, _new_name);
    }
}

static auto add_button(bool the_name_is_not_already_used, std::string_view name) -> bool
{
    auto is_clicked{false};
    Cool::ImGuiExtras::disabled_if(name == "", "Can't create a preset with an empty name.", [&]() {
        if (the_name_is_not_already_used)
        {
            is_clicked = ImGui::Button("Save Preset");
        }
        else
        {
            is_clicked = Cool::ImGuiExtras::colored_button("Overwrite", 0.11f);
        }
    });
    return is_clicked;
}

auto PresetManager::imgui_name_input() -> bool
{
    ImGui::SetNextItemWidth(ImGuiExtras::calc_custom_dropdown_input_width());
    bool const wants_to_save = ImGui::InputText("##New preset name", &_new_preset_name, ImGuiInputTextFlags_EnterReturnsTrue, nullptr, nullptr, ImDrawFlags_RoundCornersLeft);
    ImGui::SameLine(0.f, 0.f);
    name_selector();
    return wants_to_save;
}

static auto make_sure_the_user_wants_to_overwrite_the_preset(std::string_view new_preset_name) -> bool
{
    return boxer_show(
               fmt::format("You are about to overwrite \"{}\", are you sure?", new_preset_name).c_str(),
               "Are you sure?",
               boxer::Style::Warning,
               boxer::Buttons::OKCancel
           )
           == boxer::Selection::OK;
}

void PresetManager::save_preset(Settings_ConstRef new_preset_values, const PresetId& id)
{
    if (_default_presets.contains(id))
        return; // Can't modify default presets

    if (_user_defined_presets.contains(id))
    {
        if (!make_sure_the_user_wants_to_overwrite_the_preset(_new_preset_name))
            return;

        _user_defined_presets.with_mutable_ref(id, [&](Preset& preset) {
            preset.values    = new_preset_values.as_settings();
            _new_preset_name = "";
        });
    }
    else
    {
        _current_preset_id = add({
            .name   = _new_preset_name,
            .values = new_preset_values.as_settings(),
        });
        _new_preset_name   = "";
    }

    save_to_file();
}

void PresetManager::save_to_file()
{
    do_save(_user_defined_presets, _serializer);
}

auto PresetManager::imgui_adding_preset(Settings_ConstRef settings) -> bool
{
    auto const preset_id = find_preset_with_given_name(_new_preset_name);

    bool wants_to_save{false};
    wants_to_save |= imgui_name_input();
    wants_to_save |= add_button(preset_id == PresetId{}, _new_preset_name);

    if (wants_to_save)
    {
        save_preset(settings, preset_id);
        return true;
    }
    return false;
}

auto PresetManager::imgui_presets(Settings_Ref settings) -> bool
{
    _current_preset_id               = find_preset_with_given_values(settings);
    auto const current_preset_name   = preset_name(_current_preset_id);
    auto const selected_id           = dropdown("Presets", current_preset_name.value_or("Unsaved Settings..."));
    auto       settings_have_changed = apply(selected_id, settings);

    if (!current_preset_name)
    {
        settings_have_changed |= imgui_adding_preset(settings); // Change settings_have_changed, to force applying the value as the default value when saving current values as a preset
    }
    else
    {
        Cool::ImGuiExtras::disabled_if(_default_presets.contains(_current_preset_id), "You can only edit the presets you created, not the default ones that come with Coollab.", [&]() {
            delete_button(_current_preset_id, *current_preset_name, *this);
            ImGui::SameLine();
            _rename_widget.imgui(_current_preset_id, *current_preset_name, *this);
        });
    }

    return settings_have_changed;
}

} // namespace Cool