#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <algorithm>

#include <grpcpp/grpcpp.h>
#include "messenger.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReaderWriter;
using grpc::Status;

using messenger::ChatMessage;
using messenger::ChatService;

// ChatServiceImpl implements the ChatService defined in messenger.proto.
// It keeps track of all connected clients and broadcasts messages between them.
class ChatServiceImpl final : public ChatService::Service {
public:
    Status Chat(ServerContext* context, ServerReaderWriter<ChatMessage, ChatMessage>* stream) override {
        // Register this client's stream
        {
            std::lock_guard<std::mutex> lock(mu_);
            clients_.push_back(stream);
            std::cout << "[Server] New client connected. Total clients: "
                      << clients_.size() << std::endl;
        }

        ChatMessage msg;
        // Read messages from this client in a loop
        while (stream->Read(&msg)) {
            // Optional logging on the server
            std::cout << "[Server] Received from '" << msg.from()
                      << "': " << msg.text() << std::endl;

            // Broadcast to all OTHER connected clients
            std::lock_guard<std::mutex> lock(mu_);
            for (auto* client_stream : clients_) {
                if (client_stream == stream) {
                    continue; // don't echo back to the sender
                }
                // If Write fails, the client is probably gone, but for
                // simplicity we ignore errors here.
                client_stream->Write(msg);
            }
        }

        // Client disconnected (Read() returned false)
        {
            std::lock_guard<std::mutex> lock(mu_);
            clients_.erase(
                std::remove(clients_.begin(), clients_.end(), stream),
                clients_.end()
            );
            std::cout << "[Server] Client disconnected. Total clients: "
                      << clients_.size() << std::endl;
        }

        return Status::OK;
    }

private:
    std::mutex mu_;
    // List of all connected client streams
    std::vector<ServerReaderWriter<ChatMessage, ChatMessage>*> clients_;
};

void RunServer() {
    std::string server_address("localhost:50051");
    ChatServiceImpl service;

    ServerBuilder builder;
    // Listen on the given address without authentication
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register our service implementation
    builder.RegisterService(&service);

    // Build and start the server
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Chat server listening on " << server_address << std::endl;

    // Block until the server is shut down
    server->Wait();
}

int main(int argc, char** argv) {
    RunServer();
    return 0;
}