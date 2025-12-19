This is a simple Command-Line gRPC Messenger
The project consists of:
   - A gRPC server.
   - A console chat client.
   - Bidirectional streaming communication.

Project Structure:
.
├── CMakeLists.txt
├── proto/
│   └── messenger.proto
├── src/
│   ├── client.cpp
│   └── server.cpp
└── README.md

To build:
  - Create and enter a directory called build.
  - Pass the toolchain file:
    - I used vcpkg to install grpc on Windows, so I had to do this:
      - cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
    - Otherwise, "cmake .." may work.
  - Build the binaries:
    - Windows:        cmake --build . --config Release
    - Linux / macOS:  cmake --build .

To run the server:
  - Windows:
    - cd Release
    - ./grpc_server.exe
  - Linux/macOS:
    - ./grpc_server
  - You will be prompted for a localhost port (just enter the port, don't write "localhost:XXXXX", just "XXXXX")

To run a client:
  - Windows:
    - cd Release
    - ./grpc_client.exe
  - Linux/macOS:
    - ./grpc_client
  - You'll be prompted for a local host exactly like the server prompted; make sure to enter the same one.
  - You'll also be prompted for a username.

Run a 2nd or 3rd client in other terminals to test this.

    
    
