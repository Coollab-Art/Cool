#include "gpu_info.h"
#include "Cool/WebGPU/WebGPUContext.hpp"
#include "magic_enum/include/magic_enum/magic_enum.hpp"
#include "webgpu/webgpu.hpp"

namespace Cool {

// TODO(WebGPU) Use the new formatting / alignement system for dumps
auto full_gpu_info() -> std::string
{
    wgpu::AdapterProperties props;
    webgpu_context().adapter.getProperties(&props);
    return fmt::format(
        R"STR(GPU               : {} ({} {})
GPU Type          : {}
GPU Backend       : {}
GPU Driver        : {})STR",
        props.name,
        props.vendorName,
        props.architecture,
        magic_enum::enum_name(props.adapterType),
        magic_enum::enum_name(props.backendType),
        props.driverDescription
    );
}

} // namespace Cool