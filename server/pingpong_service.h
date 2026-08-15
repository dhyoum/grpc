#ifndef SERVER_PINGPONG_SERVICE_H_
#define SERVER_PINGPONG_SERVICE_H_

#include <functional>
#include <mutex>
#include <vector>

#include <grpcpp/grpcpp.h>
#include "proto/pingpong.grpc.pb.h"

class PingPongServiceImpl final : public pingpong::PingPong::CallbackService {
public:
    grpc::ServerUnaryReactor* Ping(
        grpc::CallbackServerContext* context,
        const pingpong::PingRequest* request,
        pingpong::PongResponse* response) override;

    grpc::ServerWriteReactor<pingpong::StatusIndication>* Subscribe(
        grpc::CallbackServerContext* context,
        const pingpong::SubscribeRequest* request) override;

    // 외부에서 indication을 발행 — 모든 구독자에게 전달됨
    void Publish(pingpong::StatusIndication indication);

private:
    using WriterCallback = std::function<void(const pingpong::StatusIndication&)>;

    std::mutex subscribers_mu_;
    std::vector<WriterCallback> subscribers_;

    void AddSubscriber(WriterCallback cb);
    void RemoveSubscriber(const WriterCallback* cb);
};

#endif  // SERVER_PINGPONG_SERVICE_H_
