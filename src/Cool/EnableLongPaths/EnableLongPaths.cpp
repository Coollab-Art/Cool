#if defined(_WIN32)
#include "EnableLongPaths.hpp"
#include "Cool/Path/Path.h"
//
#include "windows.h"
//
#include "shellapi.h"

namespace Cool {

auto has_long_paths_enabled() -> bool
{
    HKEY key{};
    LONG result = RegOpenKeyExA(HKEY_LOCAL_MACHINE, R"(SYSTEM\CurrentControlSet\Control\FileSystem)", 0, KEY_READ, &key);
    if (result != ERROR_SUCCESS)
        return true;

    DWORD value_type{};
    DWORD value_data{};
    DWORD value_size{sizeof(DWORD)};
    LONG  query_result = RegQueryValueExA(key, "LongPathsEnabled", nullptr, &value_type, reinterpret_cast<LPBYTE>(&value_data), &value_size); // NOLINT(*reinterpret-cast)
    RegCloseKey(key);

    if (query_result != ERROR_SUCCESS || value_type != REG_DWORD)
        return false;

    return value_data;
}

void enable_long_paths_by_asking_user_permission()
{
    auto const path = std::filesystem::weakly_canonical(Path::cool_res() / "LongPathsEnabled.reg").wstring();

    SHELLEXECUTEINFOW sei = {};
    sei.cbSize            = sizeof(SHELLEXECUTEINFOW);
    sei.fMask             = 0;
    sei.hwnd              = nullptr;
    sei.lpDirectory       = nullptr;
    sei.hInstApp          = nullptr;
    sei.lpVerb            = L"runas";       // Request elevated privileges
    sei.lpFile            = L"regedit.exe"; // Registry Editor application
    sei.lpParameters      = path.c_str();   // Path to the .reg file
    sei.nShow             = SW_SHOWNORMAL;

    ShellExecuteExW(&sei);
}

} // namespace Cool

#endif