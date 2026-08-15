#include <print>
#include <string>
#include <memory>

#include <grpcpp/grpcpp.h>
#include "server/pingpong_service.h"

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
