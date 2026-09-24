#pragma once

namespace Cool {

/// Writes the GPU and the OpenGL limits that we depend on to the log file.
/// Must be called once an OpenGL context is current and glad has been loaded.
/// This is the only place where we can report that information: the log file's header
/// (see gen_dump_string()) is written before any OpenGL context exists, so it can't contain it.
void log_opengl_info();

} // namespace Cool
