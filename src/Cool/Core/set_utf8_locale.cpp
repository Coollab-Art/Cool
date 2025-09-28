#include "set_utf8_locale.hpp"

namespace Cool {

void set_utf8_locale()
{
#ifdef _WIN32
    if (!std::setlocale(LC_ALL, ".65001" /*utf-8*/))
        assert(false);
#endif
}

} // namespace Cool