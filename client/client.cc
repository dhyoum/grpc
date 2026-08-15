#include <print>
#include <string>
#include <thread>
#include <chrono>

#include <grpcpp/grpcpp.h>
#include "client/pingpong_client.h"

int main() {
    auto channel = grpc::CreateChannel("localhost:9090",
                                       grpc::InsecureChannelCredentials());
    PingPongClient client(channel);

    // 비동기로 여러 ping 전송
    for (int i = 1; i <= 5; ++i) {
        client.AsyncPing("hello #" + std::to_string(i),
            [](grpc::Status status, const std::string& response) {
                if (status.ok()) {
                    std::println("[Client] Received pong: \"{}\"", response);
                } else {
                    std::println("[Client] RPC failed: {}", status.error_message());
                }
            });
    }

    // 비동기 콜백이 완료될 시간 대기
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::println("[Client] Done.");
    return 0;
}
