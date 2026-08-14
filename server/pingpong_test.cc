#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>

#include <grpcpp/grpcpp.h>
#include "proto/pingpong.grpc.pb.h"

// 서버 구현 (server.cc와 동일)
class TestPingPongService final : public pingpong::PingPong::CallbackService {
public:
    grpc::ServerUnaryReactor* Ping(
        grpc::CallbackServerContext* context,
        const pingpong::PingRequest* request,
        pingpong::PongResponse* response) override {

        response->set_message("pong: " + request->message());

        auto* reactor = context->DefaultReactor();
        reactor->Finish(grpc::Status::OK);
        return reactor;
    }
};

class PingPongTest : public ::testing::Test {
protected:
    void SetUp() override {
        service_ = std::make_unique<TestPingPongService>();

        grpc::ServerBuilder builder;
        // 포트 0 = OS가 빈 포트 할당
        builder.AddListeningPort("localhost:0",
                                 grpc::InsecureServerCredentials(),
                                 &port_);
        builder.RegisterService(service_.get());
        server_ = builder.BuildAndStart();
        ASSERT_NE(server_, nullptr) << "Server failed to start";
        ASSERT_GT(port_, 0) << "Failed to get assigned port";

        // 클라이언트 채널 생성
        auto channel = grpc::CreateChannel(
            "localhost:" + std::to_string(port_),
            grpc::InsecureChannelCredentials());
        stub_ = pingpong::PingPong::NewStub(channel);
    }

    void TearDown() override {
        server_->Shutdown();
    }

    int port_ = 0;
    std::unique_ptr<TestPingPongService> service_;
    std::unique_ptr<grpc::Server> server_;
    std::unique_ptr<pingpong::PingPong::Stub> stub_;
};

// 동기 호출 테스트: 기본 ping/pong 동작 확인
TEST_F(PingPongTest, SyncPingPong) {
    pingpong::PingRequest request;
    request.set_message("hello");

    pingpong::PongResponse response;
    grpc::ClientContext context;

    grpc::Status status = stub_->Ping(&context, request, &response);

    EXPECT_TRUE(status.ok()) << "RPC failed: " << status.error_message();
    EXPECT_EQ(response.message(), "pong: hello");
}

// 여러 요청 테스트
TEST_F(PingPongTest, MultiplePings) {
    for (int i = 0; i < 10; ++i) {
        pingpong::PingRequest request;
        request.set_message("msg_" + std::to_string(i));

        pingpong::PongResponse response;
        grpc::ClientContext context;

        grpc::Status status = stub_->Ping(&context, request, &response);

        EXPECT_TRUE(status.ok());
        EXPECT_EQ(response.message(), "pong: msg_" + std::to_string(i));
    }
}

// 빈 메시지 테스트
TEST_F(PingPongTest, EmptyMessage) {
    pingpong::PingRequest request;
    request.set_message("");

    pingpong::PongResponse response;
    grpc::ClientContext context;

    grpc::Status status = stub_->Ping(&context, request, &response);

    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.message(), "pong: ");
}

// Async callback 테스트: 비동기 요청 여러개 동시 전송
TEST_F(PingPongTest, AsyncMultiplePings) {
    constexpr int kNumRequests = 5;
    std::atomic<int> completed{0};
    std::mutex mu;
    std::condition_variable cv;
    std::vector<std::string> responses;

    for (int i = 0; i < kNumRequests; ++i) {
        auto* request = new pingpong::PingRequest();
        request->set_message("async_" + std::to_string(i));

        auto* response = new pingpong::PongResponse();
        auto* context = new grpc::ClientContext();

        stub_->async()->Ping(context, request, response,
            [&, request, response, context](grpc::Status status) {
                EXPECT_TRUE(status.ok());
                {
                    std::lock_guard<std::mutex> lock(mu);
                    responses.push_back(response->message());
                }
                completed.fetch_add(1);
                cv.notify_one();
                delete request;
                delete response;
                delete context;
            });
    }

    // 모든 비동기 응답 대기
    std::unique_lock<std::mutex> lock(mu);
    cv.wait_for(lock, std::chrono::seconds(5),
                [&] { return completed.load() == kNumRequests; });

    EXPECT_EQ(completed.load(), kNumRequests);
    EXPECT_EQ(responses.size(), kNumRequests);

    // 모든 응답이 "pong: async_N" 형태인지 확인
    for (const auto& resp : responses) {
        EXPECT_TRUE(resp.find("pong: async_") != std::string::npos);
    }
}
