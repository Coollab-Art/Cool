from time import sleep
from typing import Callable
import websocket
import json
import threading
import asyncio


can_start = False


def on_open(ws):
    global can_start
    can_start = True
    print("Connected")


class Coollab:
    _ws: websocket.WebSocketApp
    _callback: Callable[[], None]
    _next_id: int = 0
    _future: asyncio.Future

    def __init__(self, host: str = "127.0.0.1", port: int = 12345) -> None:
        self._ws = websocket.WebSocketApp(
            f"ws://{host}:{port}", on_open=on_open, on_message=self._on_message
        )
        thread = threading.Thread(target=self._ws.run_forever)
        thread.daemon = True
        thread.start()
        while not can_start:  # Wait until websocket connection is created
            pass

    def _send_command(self, command: str, params: dict):
        params["command"] = command
        params["command_id"] = self._next_id
        self._next_id += 1
        self._ws.send(json.dumps(params))

    def _on_message(self, ws, message):
        print("Received:", message)
        d = json.loads(message)
        if d["event"] == "ImageExportFinished":
            self._callback()
        elif d["event"] == "OpenedProject":
            self._loop.call_soon_threadsafe(self._future.set_result, None)

    def export_image(self, width: int = 500, height: int = 500) -> None:
        self._send_command(
            "ExportImage",
            {
                "width": width,
                "height": height,
                "format": ".png",
            },
        )

    def log(self, title: str, content: str) -> None:
        self._send_command(
            "Log",
            {
                "title": title,
                "content": content,
            },
        )

    def close_app(self) -> None:
        self._send_command(
            "CloseApp",
            {
                "force_kill_task_in_progress": False,
            },
        )

    def open_project(self, path: str) -> asyncio.Future:
        self._send_command(
            "OpenProject",
            {
                "path": path,
            },
        )
        self._loop = asyncio.get_running_loop()
        self._future = self._loop.create_future()
        return self._future

    def on_image_export_finished(self, callback: Callable[[], None]) -> None:
        self._callback = callback


async def main() -> None:
    coollab = Coollab()

    image_count = 0

    def decrease_image_count():
        nonlocal image_count
        image_count -= 1

    coollab.on_image_export_finished(decrease_image_count)
    import os

    mypath = "C:/Users/fouch/AppData/Roaming/Coollab Launcher/Projects/test"
    for filename in os.listdir(mypath):
        filepath = os.path.join(mypath, filename)
        if os.path.isfile(filepath):
            image_count += 1
            await coollab.open_project(filepath)
            coollab.export_image()

    # Need to keep the script running to listen to the responses from Coollab
    while image_count > 0:
        pass
    coollab.close_app()


asyncio.run(main())

# for i in range(IMAGE_MAX):
#     coollab.export_image(width=500, height=500)
#     coollab.wait_message()
#     # sleep(0.5)
# # for i in range(IMAGE_MAX):
# #     coollab.wait_message()
