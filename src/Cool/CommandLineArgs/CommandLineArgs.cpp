#include "CommandLineArgs.h"
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace Cool {

static auto convert_to_utf8(std::string const& str) -> std::string
{
#if defined(_WIN32)
    if (str.empty())
        return "";

    auto const wstr = [&]() { // IIFE
        int const size = MultiByteToWideChar(CP_ACP, 0, str.data(), static_cast<int>(str.size()), nullptr, 0);
        auto      wstr = std::wstring(static_cast<size_t>(size), 0);
        MultiByteToWideChar(CP_ACP, 0, str.data(), static_cast<int>(str.size()), wstr.data(), size);
        return wstr;
    }();
    if (wstr.empty())
        return "";

    int const size = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);
    auto      res  = std::string(static_cast<size_t>(size), 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), res.data(), size, nullptr, nullptr);
    return res;
#else
    return str;
#endif
}

void CommandLineArgs::init(int argc, char** argv)
{
    // Skip the first argument, since we don't care about the path of the executable.
    // NB: On Windows command-line arguments are not utf-8 encoded, so we need to do that manually
    for (int i = 1; i < argc; ++i)
        _args.emplace_back(convert_to_utf8(argv[i])); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
}

} // namespace Cool