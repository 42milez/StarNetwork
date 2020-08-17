# P2P Tech Demo
![CircleCI](https://img.shields.io/circleci/build/github/42milez/p2p-techdemo?token=d96746bb95c952ba079e569f683d11478f419ebb) ![Codecov](https://img.shields.io/codecov/c/github/42milez/p2p-techdemo)

## Overview
P2P Tech Demo demonstrates peer-to-peer networking (implemented as [star network](https://en.wikipedia.org/wiki/Star_network)). One peer becomes host, and other peers can send message through the host (broadcast/unicast).

## Demo
The under pain is host. The left and right upper pains are guest.

![demo](./docs/demo.gif)

By following the instruction below, you can run the demo (tmux and docker-compose are both required).

```
1. create a tmux session (tmux new -s demo)
2. create a new terminal window, and run demo.bash in the window
```

## Directory Structure
```
p2p-techdemo/ ................ Top src dir
├── app/ ..................... Application code
├── cmake/ ................... Helpers for CMake
│   ├── cable/  .............. https://github.com/ethereum/cable
│   ├── templates/ ........... Generate build infomation
│   └── toolchains/ .......... Toolchains for this project
├── demo.bash ................ Run demonstration
├── docker/ .................. Dockerfile and scripts
├── docker-compose.dev.yml ... Service definitions for development
├── docker-compose.release.yml Service definitions for release build
├── docker-compose.test.yml .. Service definitions for feature testing and unit testing
├── docs/ .................... Documentation for this project
├── lib/ ..................... Libraries for this project
│   ├── core/ ................ Core library
│   ├── rudp/ ................ Reliable UDP library
│   └── test/ ................ Test library
└── test/ .................... Test code
    ├── feature/ ............. Feature testing
    └── unit/ ................ Unit testing
```

## Quick Start
##### Start dev server:

```
docker-compose -f docker-compose.dev.yml up -d dev_server
```

##### Run all tests:

```
docker-compose -f docker-compose.test.yml run --rm all_tests
```

##### Build for release:

```
docker-compose -f docker-compose.release.yml run --rm build
```

## Development
If you use CLion, remote debugging is available: [Remote Debugging with CLion](https://github.com/42milez/p2p-techdemo/wiki/Remote-Debugging-with-CLion)

## Supported OS
- Linux

## Technologies used
- Reliable UDP (inspired by [ENet](https://github.com/lsalzman/enet))
  - Flow Control
    - Delayed Acknowledgement
    - Variable Bandwidth

## Road map
- Traffic Encryption
- Data Compression

## Acknowledgments
I've been influenced by:

- [ENet](https://github.com/lsalzman/enet) : Reliable UDP networking library
- [Godot Engine](https://github.com/godotengine/godot) : Multi-platform 2D and 3D game engine
