#pragma once

namespace Cool {

/// Calling this at the beginning of the program can help handling some ASCII-extended characters like 'é' on Windows
/// Otherwise things like opening a file might fail if the path contains such characters
void set_utf8_locale();

} // namespace Cool