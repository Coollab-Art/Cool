from pipe import *
from dataclasses import dataclass
from typing import List


def imgui_style_struct() -> str:
    import os
    from pathlib import Path

    path = os.path.join(
        Path(__file__).parent.parent.parent.parent,
        "lib/imgui/imgui.h",
    )

    with open(path, "r") as file:
        content = file.read()
    begin = """struct ImGuiStyle
{"""
    begin_idx = content.find(begin)
    end_idx = content.find("};", begin_idx)
    return content[begin_idx + len(begin) : end_idx]


@dataclass
class StyleElement:
    name: str
    type: str
    comment: str | None


def parse_style_elements() -> List[StyleElement]:
    def find_comment(line: str):
        index = line.find("//")
        if index == -1:
            return None
        return line[index + 2 :].strip()

    def parse_one_line(line: str):
        words = line.split("//")[0].split()
        return StyleElement(
            name=words[1].strip().removesuffix(";"),
            type=words[0].strip(),
            comment=find_comment(line),
        )

    def is_valid_line(line: str) -> bool:
        line_no_comment: str = line.split("//")[0].strip()

        if line_no_comment == "":
            return False
        if "[ImGuiCol_COUNT]" in line_no_comment:
            return False

        return True

    res = list(
        imgui_style_struct().split("\n")
        | where(lambda l: is_valid_line(l))
        | map(parse_one_line)
        | where(lambda x: x.type != "IMGUI_API")
        | where(lambda x: x.name != "IMGUI_API")
    )
    return res


def json(get_or_set: str):
    elems = parse_style_elements()
    res = ""
    for elem in elems:
        res += f'Cool::json_{get_or_set}(json, "{elem.name}", style.{elem.name});'
    return res


def json_get():
    return json("get")


def json_set():
    return json("set")


def main():
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
        folder="generated_style",
        files=[
            json_get,
            json_set,
        ],
    )


if __name__ == "__main__":
    main()
