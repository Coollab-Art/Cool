import websocket
import json


def on_message(ws, message):
    print("Received:", message)


def on_open(ws):
    # Send a test JSON message
    msg = {
        "command": "ExportImage",
        "width": 2000,
        "height": 2000,
        "format": ".png",
    }
    ws.send(json.dumps(msg))


ws = websocket.WebSocketApp(
    "ws://localhost:12345", on_open=on_open, on_message=on_message
)

ws.run_forever()
