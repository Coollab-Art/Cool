#include "Variable_TextureDescriptor_SpoutSyphon.h"

namespace Cool {

auto imgui_widget(Variable<TextureDescriptor_SpoutSyphon>& var) -> bool
{
    return var.value().imgui_widget();
}

auto imgui_widget(VariableMetadata<TextureDescriptor_SpoutSyphon>&) -> bool
{
    return false;
}

} // namespace Cool