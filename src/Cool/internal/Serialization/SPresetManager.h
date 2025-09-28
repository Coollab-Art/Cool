#pragma once
#include <filesystem>
#include "Cool/Log/ErrorMessage.hpp"
#include "Cool/Variables/PresetManager.h"
#include "reg/reg.hpp"

namespace Cool {

auto do_save(reg::RawOrderedRegistry<Preset> const&, Cool::SerializerOnDemand const&) -> bool;
auto do_load(reg::RawOrderedRegistry<Preset>&, Cool::SerializerOnDemand const&) -> tl::expected<void, ErrorMessage>;
void do_load_default_presets(reg::RawOrderedRegistry<Preset>&, std::filesystem::path const&);

} // namespace Cool