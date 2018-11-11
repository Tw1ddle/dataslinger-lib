[![Dataslinger Logo](https://github.com/Tw1ddle/dataslinger-lib/blob/master/screenshots/logo.png?raw=true "Dataslinger logo")](https://github.com/Tw1ddle/dataslinger-lib)

[![License](https://img.shields.io/badge/License-GPL%20v3-blue.svg?style=flat-square)](https://github.com/Tw1ddle/dataslinger/lib/dataslinger/blob/master/LICENSE)

Data slinger is a C++17 library for asynchronously passing data between processes on the same computer or over a network.

See the [demo](https://github.com/Tw1ddle/dataslinger) repository for examples.

## Features

 * WebSocket client and server support built on with Boost Beast.
 * Pipe-based interprocess communication (work in progress).
 * Shared memory interprocess communication using Boost Interprocess (work in progress).

## Usage

Create Slinger objects, passing in callbacks and the ConnectionOptions for the backend you want to use, then start sending byte data and polling the slingers.

## Building

Requires C++17 support, depends on boost 1.68 and [concurrentqueue](https://github.com/cameron314/concurrentqueue).

## Screenshots

[![WebSocket implementation client-server](https://github.com/Tw1ddle/dataslinger-lib/blob/master/screenshots/websocket_simple_client_server.png?raw=true "WebSocket simple client-server example")](https://github.com/Tw1ddle/dataslinger)

## Notes
 * Got an idea or suggestion? Open an issue on GitHub, or send Sam a message on [Twitter](https://twitter.com/Sam_Twidale).