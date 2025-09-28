import json
from pathlib import Path


def first_codepoint():
    return 59648


def icomoon():
    codepoint = first_codepoint()
    range = f"#define BEGIN_RANGE_ICOMOON {codepoint}\n"
    res = ""
    icons_reference_path = Path(__file__).parent.joinpath("IcoMoon-Free.json")
    with open(icons_reference_path, "r") as file:
        data = json.loads(file.read())
        for entry in data:
            unicode_str = chr(codepoint)
            utf8_bytes = unicode_str.encode("utf-8")
            hex_str = utf8_bytes.hex()
            hex_str = f"\\x{hex_str[0:2]}\\x{hex_str[2:4]}\\x{hex_str[4:6]}"
            res += f'#define ICOMOON_{entry["name"].upper().replace("-", "_")} "{hex_str}"\n'
            if entry["name"].upper() == "INFO":
                res += f"#define INFO_RANGE_ICOMOON {codepoint}\n"
            codepoint += 1
    range += f"#define END_RANGE_ICOMOON {codepoint-1}\n"
    return range + res


def imgui_show_all_icons():
    codepoint = first_codepoint()
    res = ""
    icons_reference_path = Path(__file__).parent.joinpath("IcoMoon-Free.json")
    with open(icons_reference_path, "r") as file:
        data = json.loads(file.read())
        for entry in data:
            unicode_str = chr(codepoint)
            utf8_bytes = unicode_str.encode("utf-8")
            hex_str = utf8_bytes.hex()
            hex_str = f"\\x{hex_str[0:2]}\\x{hex_str[2:4]}\\x{hex_str[4:6]}"
            icon = f'ICOMOON_{entry["name"].upper().replace("-", "_")}'
            res += f"""
                if(ImGui::Button("  " {icon}))
                    ImGui::SetClipboardText("{icon}");
                ImGui::SetItemTooltip("%s", "{icon}");
                next_item();
            """
            codepoint += 1
    return res


if __name__ == "__main__":
    import os
    from pathlib import Path
    from importlib.machinery import SourceFileLoader

    generate_files = SourceFileLoader(
        "generate_files",
        os.path.join(
            Path(os.path.abspath(__file__)).parent.parent.parent.parent,
            "tooling",
            "generate_files.py",
        ),
    ).load_module()
    generate_files.generate(
        folder="generated_icons",
        files=[icomoon, imgui_show_all_icons],
    )
