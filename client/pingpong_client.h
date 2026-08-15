#ifndef CLIENT_PINGPONG_CLIENT_H_
#define CLIENT_PINGPONG_CLIENT_H_

#include <string>
#include <functional>
#include <memory>

#include <grpcpp/grpcpp.h>
#include "proto/pingpong.grpc.pb.h"

class PingPongClient {
public:
    explicit PingPongClient(std::shared_ptr<grpc::Channel> channel);

    // 동기 호출 — 테스트 용이
    grpc::Status Ping(const std::string& msg, std::string* response_msg);

    // 비동기 호출
    void AsyncPing(const std::string& msg,
                   std::function<void(grpc::Status, const std::string&)> callback);

private:
    std::unique_ptr<pingpong::PingPong::Stub> stub_;
};

#endif  // CLIENT_PINGPONG_CLIENT_H_
