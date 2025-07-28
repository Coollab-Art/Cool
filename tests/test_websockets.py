from time import sleep
from typing import Callable
import websocket
import json
import threading
import asyncio


can_start = False
max_retries = 10
retry_delay = 2


def on_open(ws):
    global can_start
    can_start = True
    print("[COOLLAB INFO] Connected")


class Coollab:
    _ws: websocket.WebSocketApp
    _callback: Callable[[], None]
    _next_id: int = 0
    _future: asyncio.Future

    def __init__(self, host: str = "127.0.0.1", port: int = 12345) -> None:
        global can_start
        can_start = False

        try_nb = 0
        while not can_start and try_nb < max_retries:
            if try_nb != 0:
                print(f"[COOLLAB WARN] Attempt {try_nb} failed, retrying...")
                self._ws.close()
            try_nb += 1
            self.start_websocket(host, port)
            waited = 0
            while not can_start and waited < retry_delay:
                sleep(0.1)
                waited += 0.1

        if not can_start:
            raise ConnectionError("[COOLLAB ERROR] Could not connect to server after multiple attempts.")

        
    def start_websocket(self, host: str, port: int):
        try:
            self._ws = websocket.WebSocketApp(
                f"ws://{host}:{port}", on_open=on_open, on_message=self._on_message
            )
            thread = threading.Thread(target=self._ws.run_forever)
            thread.daemon = True
            thread.start()

        except Exception as e:
            print(f"[COOLLAB ERROR] Exception during connection :",e)

    def _send_command(self, command: str, params: dict):
        params["command"] = command
        params["command_id"] = self._next_id
        self._next_id += 1
        self._ws.send(json.dumps(params))

    def _on_message(self, ws, message):
        print("[COOLLAB INFO] Received:", message)
        d = json.loads(message)
        if d["event"] == "ImageExportFinished":
            self._loop.call_soon_threadsafe(
                self._callback, d["path"]
            )
        elif d["event"] == "OpenedProject":
            self._loop.call_soon_threadsafe(
                self._future.set_result, None
            )  # TODO use command_id to know which future to set
        elif d["event"] == "ImageExportStarted":
            self._loop.call_soon_threadsafe(self._future.set_result, None)
        elif d["event"] == "GetVersionName":
            self._loop.call_soon_threadsafe(
                self._future.set_result, d["version_name"]
            )

    # Starts the export, it only only be finished a lot later, and then the callback on_image_export_finished() will be called
    async def start_image_export(
        self,
        width: int = 500,
        height: int = 500,
        folder: str | None = None,
        filename: str | None = None,
        extension: str | None = None,
        project_autosave: bool = False,
        export_file_overwrite: bool = False,
    ) -> None:
        print(f"Exporting image : {filename}")
        self._loop = asyncio.get_running_loop()
        self._future = self._loop.create_future()
        self._send_command(
            "ExportImage",
            {
                "width": width,
                "height": height,
                "folder": folder,
                "filename": filename,
                "extension": extension,
                "project_autosave": project_autosave,
                "export_file_overwrite": export_file_overwrite,
            },
        )
        await self._future

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

    async def get_version_name(self) -> None:
        self._loop = asyncio.get_running_loop()
        self._future = self._loop.create_future()
        self._send_command(
            "GetVersionName",
            {
                # "param1": False,
            },
        )
        return await self._future

    async def open_project(self, path: str) -> None:
        self._loop = asyncio.get_running_loop()
        self._future = self._loop.create_future()
        self._send_command(
            "OpenProject",
            {
                "path": path,
            },
        )
        await self._future

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
    from pathlib import Path

    mypath = "C:/Users/fouch/AppData/Roaming/Coollab Launcher/Projects/test"
    for filename in os.listdir(mypath):
        filepath = os.path.join(mypath, filename)
        if os.path.isfile(filepath):
            image_count += 1
            await coollab.open_project(filepath)
            await coollab.start_image_export(
                2000,
                2000,
                folder=mypath + "/img",
                filename=Path(filename).stem,
                extension="png",
                export_file_overwrite=True,
            )

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
