#include <print>
#include <string>
#include <memory>
#include <functional>
#include <thread>
#include <chrono>

#include <grpcpp/grpcpp.h>
#include "proto/pingpong.grpc.pb.h"

class PingPongClient {
public:
    explicit PingPongClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(pingpong::PingPong::NewStub(channel)) {}

    // Async unary call using callback API
    void AsyncPing(const std::string& msg) {
        auto* request = new pingpong::PingRequest();
        request->set_message(msg);

        auto* response = new pingpong::PongResponse();
        auto* context = new grpc::ClientContext();
        auto* status = new grpc::Status();

        std::println("[Client] Sending ping: \"{}\"", msg);

        stub_->async()->Ping(context, request, response,
            [request, response, context, status](grpc::Status s) {
                if (s.ok()) {
                    std::println("[Client] Received pong: \"{}\"", response->message());
                } else {
                    std::println("[Client] RPC failed: {}", s.error_message());
                }
                delete request;
                delete response;
                delete context;
                delete status;
            });
    }

private:
    std::unique_ptr<pingpong::PingPong::Stub> stub_;
};

int main() {
    auto channel = grpc::CreateChannel("localhost:9090",
                                       grpc::InsecureChannelCredentials());
    PingPongClient client(channel);

    // 비동기로 여러 ping 전송
    for (int i = 1; i <= 5; ++i) {
        client.AsyncPing("hello #" + std::to_string(i));
    }

    // 비동기 콜백이 완료될 시간 대기
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::println("[Client] Done.");
    return 0;
}
