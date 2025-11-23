# headless-chrome-client

A small Qt-based GUI utility for interacting with a Headless Chrome / Chromium instance via the Chrome DevTools Protocol (CDP). The application launches or connects to a running Chrome instance, establishes a WebSocket debugging session, receives data, and displays it in a Qt `MainWindow`.

---

## Features

- Connects to Chrome/Chromium DevTools WebSocket endpoint.
- Uses Qt WebSockets (`QWebSocket`) to send/receive CDP messages.
- Displays received data inside a Qt GUI (`MainWindow`).
- Automatically queries the debugger URL if not provided.
- Provides window-size configuration and optional debug logging.

---

## Project Structure

```
headless-chrome-client
├── CMakeLists.txt
├── LICENSE
├── README.md (this file)
├── echoclient.h / echoclient.cpp   # WebSocket client for CDP
├── main.cpp                         # Application entry point
├── mainwindow.h / .cpp / .ui        # Qt GUI
```

---

## Build Instructions

This project uses **Qt** and **CMake**.

### Requirements
- Qt 5 or Qt 6 (with QtWebSockets and QtWidgets modules)
- CMake ≥ 3.5
- C++ compiler (GCC, Clang, MSVC)
- A Chrome or Chromium installation (if you plan to connect to local headless Chrome)

### Build Steps

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

Output binary will be placed in the `build` directory.

---

## Running

### 1. Ensure Chrome/Chromium is running with remote debugging enabled

```bash
chrome --headless --remote-debugging-port=9222
```

### 2. Run the application

```bash
./headless-chrome-client
```

If no WebSocket URL is specified, the application attempts to retrieve it automatically via:

```
http://localhost:9222/json
```

---

## Code Overview

### `EchoClient`
Located in `echoclient.h` / `echoclient.cpp`.

Responsible for:
- Opening a WebSocket connection to the DevTools endpoint
- Sending messages
- Emitting signals when data is received
- Handling simple mouse-click states and ID counters

Key members:
- `QWebSocket m_webSocket;`
- `QString m_url;`
- `int m_width, m_height;` (page size)

### `MainWindow`
Defines the GUI that displays incoming data.

Connected via:
```cpp
QObject::connect(&client, &EchoClient::dataReceived,
                 &w, &MainWindow::onDataReceived);
```

### `main.cpp`
- Initializes QApplication
- Resolves WebSocket debugging URL
- Creates `EchoClient` and `MainWindow`
- Connects signals and starts the event loop

---

## Debugger URL Retrieval

If a WebSocket URL isn't provided, Chrome's debugging info is fetched using Qt networking:

```cpp
QNetworkAccessManager
QNetworkRequest("http://localhost:9222/json")
```

The application extracts `webSocketDebuggerUrl` from the returned JSON.

---

## License

This project includes QtWebSockets BSD-licensed example code and is itself distributed under the included **LICENSE** file.

---

![demo](https://user-images.githubusercontent.com/11851670/186407215-7fa63995-1870-42b4-bf91-89d2653e15e5.png)
