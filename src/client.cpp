#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <grpcpp/grpcpp.h>

#include "messenger.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReaderWriter;
using grpc::Status;

using messenger::ChatMessage;
using messenger::ChatService;

// A simple chat client that connects to ChatService::Chat (bidirectional stream)
class ChatClient {
public:
    ChatClient(std::shared_ptr<Channel> channel, std::string username)
        : stub_(ChatService::NewStub(channel)), username_(std::move(username)) {}

    void Run() {
        ClientContext context;

        std::shared_ptr<ClientReaderWriter<ChatMessage, ChatMessage>> stream(
            stub_->Chat(&context));

        if (!stream) {
            std::cerr << "Failed to open stream to server." << std::endl;
            return;
        }

        ChatMessage intro;
        intro.set_from(username_);
        intro.set_text(""); 
        if (!stream->Write(intro)) {
            std::cerr << "Failed to send intro message." << std::endl;
            stream->WritesDone();
            return;
        }

        std::thread reader([this, stream]() {
            ChatMessage incoming;
            while (stream->Read(&incoming)) {
                if (incoming.from() == username_ && incoming.text().empty()) {
                    continue;
                }

                std::cout << incoming.from() << ": " << incoming.text() << std::endl;
            }

            std::cout << "Server closed the stream." << std::endl;
        });

        std::string line;
        std::cout << "Type messages and press Enter to send. Type /quit to exit." << std::endl;

        while (true) {
            if (!std::getline(std::cin, line)) {
                break;
            }

            if (line == "/quit") {
                break;
            }

            ChatMessage msg;
            msg.set_from(username_);
            msg.set_text(line);

            if (!stream->Write(msg)) {
                std::cerr << "Write failed (stream closed)." << std::endl;
                break;
            }
        }

        stream->WritesDone();

        reader.join();

        Status status = stream->Finish();
        if (!status.ok()) {
            std::cerr << "Chat RPC failed: " << status.error_code() << " - " << status.error_message() << std::endl;
        }
    }

private:
    std::unique_ptr<ChatService::Stub> stub_;
    std::string username_;
};

int main(int argc, char** argv) {
    std::string server_address = "localhost:50051";
    std::string username;
    std::cout << "Enter username: ";
    std::cin >> username;


    std::cout << "Connecting to " << server_address
              << " as user [" << username << "]" << std::endl;

    ChatClient client(
        grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()),
        username);

    client.Run();

    return 0;
}
