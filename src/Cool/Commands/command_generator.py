from dataclasses import dataclass
from typing import List
from enum import Enum, auto


class FieldType(Enum):
    INT = auto()
    STRING = auto()


@dataclass
class Field:
    name: str
    type: FieldType


@dataclass
class Command:
    name: str
    fields: List[Field]


def commands() -> List[Command]:
    return [
        Command(
            name="Log",
            fields=[
                Field(name="Title", type=FieldType.STRING),
                Field(name="Content", type=FieldType.STRING),
            ],
        ),
    ]

def generate_commands():
    for command in commands():
