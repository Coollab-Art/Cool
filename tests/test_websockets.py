from typing import Callable
import websocket
import json
import threading


can_start = False


def on_open(ws):
    global can_start
    can_start = True
    print("Connected")


class Coollab:
    _ws: websocket.WebSocketApp
    _callback: Callable[[], None]

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
        self._ws.send(json.dumps(params))

    def _on_message(self, ws, message):
        print("Received:", message)
        d = json.loads(message)
        if d["event"] == "ImageExportFinished":
            self._callback()

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

    def on_image_export_finished(self, callback: Callable[[], None]) -> None:
        self._callback = callback


coollab = Coollab()

IMAGE_MAX = 10
image_count = 0

has_finished_exporting = False


def increase_image_count():
    global image_count
    global has_finished_exporting
    image_count += 1
    print(image_count)
    if image_count == IMAGE_MAX:
        coollab.close_app()
        has_finished_exporting = True


coollab.on_image_export_finished(increase_image_count)
for i in range(10):
    coollab.log(title="Script", content=f"This is {i}")
    coollab.export_image(2000, 2000)

# Need to keep the script running to listen to the responses from Coollab
while not has_finished_exporting:
    pass

# for i in range(IMAGE_MAX):
#     coollab.export_image(width=500, height=500)
#     coollab.wait_message()
#     # sleep(0.5)
# # for i in range(IMAGE_MAX):
# #     coollab.wait_message()
