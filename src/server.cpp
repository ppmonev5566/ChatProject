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

class ChatServiceImpl final : public ChatService::Service {
    private:
        std::mutex mu;
        std::vector<ServerReaderWriter<ChatMessage, ChatMessage>*> clients;
    public:
        Status Chat(ServerContext* context, ServerReaderWriter<ChatMessage, ChatMessage>* stream) override {
            {
                std::lock_guard<std::mutex> lock(mu);
                clients.push_back(stream);
                std::cout << "New client connected. Total clients: " << clients.size() << std::endl;
            }

            ChatMessage msg;
            while (stream->Read(&msg)) {
                if(msg.text() == "") {
                    continue;
                }
                std::cout << "[Server] Received from '" << msg.from() << "': " << msg.text() << std::endl;
                
                std::lock_guard<std::mutex> lock(mu);
                for (auto* client_stream : clients) {
                    if (client_stream == stream) {
                        continue;
                    }
                    client_stream->Write(msg);
                }
            }

            {
                std::lock_guard<std::mutex> lock(mu);
                clients.erase(std::remove(clients.begin(), clients.end(), stream), clients.end());
                std::cout << "[Server] Client disconnected. Total clients: " << clients.size() << std::endl;
            }

            return Status::OK;
        }
};


void RunServer() {
    std::string local_port; //50051
    std::cout << "Enter localhost port number to host on: ";
    std::cin >> local_port;

    ChatServiceImpl service;
    ServerBuilder builder;
    
    builder.AddListeningPort("localhost:" + local_port, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Chat server listening on " << "localhost:" + local_port << std::endl;

    server->Wait();
}

int main(int argc, char** argv) {
    RunServer();
    return 0;
}