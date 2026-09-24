#if defined(COOL_OPENGL)
#include "log_opengl_info.hpp"
#include "Cool/Gpu/gpu_info.h"
#include "Cool/Log/file_logger.hpp"
#include "Cool/Utils/Version.h"

namespace Cool {

static auto get_int(GLenum name) -> GLint
{
    GLint value{0};
    glGetIntegerv(name, &value);
    return value;
}

static auto get_indexed_int(GLenum name, GLuint index) -> GLint
{
    GLint value{0};
    glGetIntegeri_v(name, index, &value);
    return value;
}

void log_opengl_info()
{
    auto const major   = get_int(GL_MAJOR_VERSION);
    auto const minor   = get_int(GL_MINOR_VERSION);
    auto const version = major * 100 + minor * 10;

    // Querying a limit that the context is too old to know about would just generate a GL error, so
    // only ask for the OpenGL 4.3 ones when we actually got a 4.3 context.
    auto const limit_4_3 = [&](GLenum name) -> std::string {
        if (version < 430)
            return "n/a (needs OpenGL 4.3)";
        return std::to_string(get_int(name));
    };
    auto const indexed_limit_4_3 = [&](GLenum name, GLuint index) -> std::string {
        if (version < 430)
            return "n/a (needs OpenGL 4.3)";
        return std::to_string(get_indexed_int(name, index));
    };

    // This goes to the log file only, never to the message console: it is diagnostics that we want
    // in every user's log without them having to enable any debug option.
    with_file_logger([&](spdlog::logger& logger) {
        logger.info(fmt::format(
            R"STR(OpenGL context
{}
Context version    : {}.{} (we requested {}.{})
MAX_COMPUTE_SHADER_STORAGE_BLOCKS  : {}
MAX_VERTEX_SHADER_STORAGE_BLOCKS   : {}
MAX_FRAGMENT_SHADER_STORAGE_BLOCKS : {}
MAX_SHADER_STORAGE_BUFFER_BINDINGS : {}
MAX_COMPUTE_WORK_GROUP_INVOCATIONS : {}
MAX_COMPUTE_WORK_GROUP_SIZE[0]     : {}
MAX_VERTEX_ATTRIBS                 : {})STR",
            full_gpu_info(),
            major, minor, major_version(COOL_OPENGL_VERSION), minor_version(COOL_OPENGL_VERSION),
            limit_4_3(GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS),
            // The vertex and fragment ones are allowed to be 0 even on a conformant OpenGL 4.3
            // driver, so they are worth knowing about: any shader reading an SSBO in those stages
            // would fail to link on such a machine.
            limit_4_3(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS),
            limit_4_3(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS),
            limit_4_3(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS),
            limit_4_3(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS),
            indexed_limit_4_3(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0),
            std::to_string(get_int(GL_MAX_VERTEX_ATTRIBS))
        ));
    });

    if (version < COOL_OPENGL_VERSION)
    {
        // We only warn, we don't refuse to start: someone in this situation can currently use at
        // least part of the app, and bailing out would take that away without fixing anything.
        // It does mean that every function the driver doesn't implement is a null pointer though,
        // so a crash is likely.
        Log::internal_warning(
            "OpenGL",
            fmt::format(
                "We requested an OpenGL {}.{} context but the driver gave us {}.{}. Some features won't work, and the app might crash.",
                major_version(COOL_OPENGL_VERSION), minor_version(COOL_OPENGL_VERSION), major, minor
            )
        );
    }
}

} // namespace Cool

#endif
