#include <print>
#include <string>
#include <memory>

#include <grpcpp/grpcpp.h>
#include "proto/pingpong.grpc.pb.h"

// Async callback-based server implementation
class PingPongServiceImpl final : public pingpong::PingPong::CallbackService {
public:
    grpc::ServerUnaryReactor* Ping(
        grpc::CallbackServerContext* context,
        const pingpong::PingRequest* request,
        pingpong::PongResponse* response) override {

        std::println("[Server] Received ping: \"{}\"", request->message());

        response->set_message("pong: " + request->message());

        std::println("[Server] Sending pong: \"{}\"", response->message());

        auto* reactor = context->DefaultReactor();
        reactor->Finish(grpc::Status::OK);
        return reactor;
    }
};

void RunServer() {
    std::string server_address("localhost:9090");
    PingPongServiceImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::println("[Server] Listening on {}", server_address);

    server->Wait();
}

int main() {
    RunServer();
    return 0;
}
