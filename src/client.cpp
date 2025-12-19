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

class ChatClient {
    private:
        std::unique_ptr<ChatService::Stub> stub_;
        std::string username;
    public:
        ChatClient(std::shared_ptr<Channel> channel, std::string username) : stub_(ChatService::NewStub(channel)), username(std::move(username)) {}
        void Run() {
            ClientContext context;
            std::shared_ptr<ClientReaderWriter<ChatMessage, ChatMessage>> stream(stub_->Chat(&context));
            
            if (!stream) {
                std::cerr << "Failed to open stream to server." << std::endl;
                return;
            }

            ChatMessage intro;
            intro.set_from(username);
            intro.set_text(""); 
            if (!stream->Write(intro)) {
                std::cerr << "Failed to send intro message." << std::endl;
                stream->WritesDone();
                return;
            }

            //thread to handle the incoming messages on the side
            std::thread reader([this, stream]() {
                ChatMessage incoming;
                while (stream->Read(&incoming)) {
                    if (incoming.from() == username && incoming.text().empty()) {
                        continue;
                    }
                    std::cout << std::endl << incoming.from() << ": " << incoming.text() << std::endl << "---> ";
                }
                std::cout << "Server closed the stream." << std::endl;
            });

            //loop to handle outbound messages
            std::cout << "Type messages and press Enter to send. Type /quit to exit." << std::endl;
            std::string line;
            while (true) {
                std::cout << "---> ";
                if (!std::getline(std::cin, line)) { 
                    break;
                }
                if (line == "/quit") {
                    break;
                }

                ChatMessage msg;
                msg.set_from(username);
                msg.set_text(line);

                if (!stream->Write(msg)) {
                    std::cerr << "Your message failed to send because the stream closed." << std::endl;
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
};

int main(int argc, char** argv) {
    //50051
    std::string local_port;
    std::string username;

    std::cout << "Enter localhost port number to connect to: ";
    std::getline(std::cin, local_port);

    std::cout << "Enter username: ";
    std::getline(std::cin, username);

    std::cout << "Connecting to " << "localhost:" << local_port << " as user [" << username << "]" << std::endl;

    ChatClient client(grpc::CreateChannel("localhost:" + local_port, grpc::InsecureChannelCredentials()), username);
    client.Run();

    return 0;
}
