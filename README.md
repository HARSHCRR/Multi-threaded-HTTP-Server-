# Multithreaded HTTP Server

A from-scratch HTTP/1.1 server written in C++17, built to understand how web servers actually work — TCP sockets, thread pools, HTTP parsing, and static file serving — without any external libraries.

## Features

- TCP socket server using POSIX sockets
- Thread pool (configurable size) — no one-thread-per-connection waste
- HTTP/1.1 request parsing (method, path, headers, body)
- HTTP response generation with proper status codes
- Static file serving with MIME type detection
- GET and POST support
- Path traversal protection

## Architecture

```
Client Request
      │
      ▼
  TCP Socket (accept)
      │
      ▼
  Thread Pool ──► Worker Thread
                      │
                      ▼
               Parse HTTP Request
                      │
             ┌────────┴────────┐
             ▼                 ▼
            GET               POST
             │                 │
     Serve static file    Echo JSON body
             │                 │
             └────────┬────────┘
                      ▼
             Send HTTP Response
```

## Project Structure

```
multithreaded-http-server/
├── Makefile
├── public/                  # Static files served by the server
│   ├── index.html
│   └── style.css
└── src/
    ├── main.cpp             # Entry point, argument parsing
    ├── server.h / .cpp      # TCP accept loop, dispatches to thread pool
    ├── thread_pool.h / .cpp # Fixed-size worker thread pool
    ├── http_parser.h / .cpp # Raw request string → HttpRequest struct
    ├── http_response.h/.cpp # HttpResponse struct → raw response string
    └── file_handler.h / .cpp# Resolve URL path → read file → HttpResponse
```

## Build & Run

**Requirements:** g++ with C++17 support (`g++ --version` should be ≥ 7)

```bash
# Build
make

# Run with defaults (port 8080, 4 threads, ./public)
make run

# Or with custom args: ./http_server <port> <threads> <static-root>
./http_server 8080 8 public
```

Then open `http://localhost:8080` in your browser.

## Usage

### GET — Static Files

Any file in the `public/` directory is served automatically.

```
GET /          → public/index.html
GET /style.css → public/style.css
GET /about.html→ public/about.html
```

### POST — Echo Endpoint

POST to any path — the server echoes your body back as JSON.

```bash
curl -X POST http://localhost:8080/echo \
     -H "Content-Type: text/plain" \
     -d "hello world"

# Response:
# {"method":"POST","path":"/echo","body":"hello world"}
```

### Load testing (optional)

```bash
# Install wrk, then:
wrk -t4 -c100 -d10s http://localhost:8080/
```

## How It Works — Key Concepts

### Thread Pool

Instead of spawning a new thread per connection (which breaks under load), we pre-create N worker threads that sit idle waiting on a condition variable. When a client connects, `accept()` returns a `client_fd` that gets pushed onto a task queue. A worker wakes up, processes the request, and goes back to sleep.

```
main thread:  accept() → push client_fd to queue → back to accept()
worker N:     wait → wake → pop client_fd → handle → wait
```

### HTTP Parsing

HTTP/1.1 requests are plain text:

```
GET /index.html HTTP/1.1\r\n
Host: localhost:8080\r\n
\r\n
```

We split on `\r\n\r\n` to separate headers from body, then parse the request line and each `Key: Value` header into a struct.

### Static File Serving

URL path → strip query string → prepend `public/` → check for path traversal → `std::ifstream` → read → set MIME type → send.

## Supported MIME Types

| Extension | MIME Type |
|-----------|-----------|
| `.html`   | text/html |
| `.css`    | text/css  |
| `.js`     | application/javascript |
| `.json`   | application/json |
| `.png`    | image/png |
| `.jpg`    | image/jpeg |
| `.svg`    | image/svg+xml |
| `.txt`    | text/plain |

## What's Next (Steps 2–10)

- Step 2: Persistent connections (`Connection: keep-alive`)
- Step 3: Chunked transfer encoding
- Step 4: Router — map paths to handler functions
- Step 5: Logging middleware
- Step 6: Basic authentication
- Step 7: HTTPS via OpenSSL
- Step 8: WebSocket upgrade
- Step 9: Config file (JSON/TOML)
- Step 10: Benchmarking and tuning
