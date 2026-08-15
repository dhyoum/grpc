#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <mutex>
#include <condition_variable>
#include <vector>

#include <grpcpp/grpcpp.h>
#include "client/pingpong_client.h"
#include "server/pingpong_service.h"

class PingPongClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        service_ = std::make_unique<PingPongServiceImpl>();

        grpc::ServerBuilder builder;
        builder.AddListeningPort("0.0.0.0:0",
                                 grpc::InsecureServerCredentials(),
                                 &port_);
        builder.RegisterService(service_.get());
        server_ = builder.BuildAndStart();
        ASSERT_NE(server_, nullptr);
        ASSERT_GT(port_, 0);

        auto channel = grpc::CreateChannel(
            "localhost:" + std::to_string(port_),
            grpc::InsecureChannelCredentials());
        client_ = std::make_unique<PingPongClient>(channel);
    }

    void TearDown() override {
        server_->Shutdown();
    }

    int port_ = 0;
    std::unique_ptr<PingPongServiceImpl> service_;
    std::unique_ptr<grpc::Server> server_;
    std::unique_ptr<PingPongClient> client_;
};

TEST_F(PingPongClientTest, SyncPing) {
    std::string response;
    grpc::Status status = client_->Ping("hello", &response);

    EXPECT_TRUE(status.ok()) << status.error_message();
    EXPECT_EQ(response, "pong: hello");
}

TEST_F(PingPongClientTest, SyncPingEmpty) {
    std::string response;
    grpc::Status status = client_->Ping("", &response);

    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response, "pong: ");
}

TEST_F(PingPongClientTest, AsyncPing) {
    constexpr int kNumRequests = 5;
    std::atomic<int> completed{0};
    std::mutex mu;
    std::condition_variable cv;
    std::vector<std::string> responses;

    for (int i = 0; i < kNumRequests; ++i) {
        client_->AsyncPing("async_" + std::to_string(i),
            [&](grpc::Status status, const std::string& response) {
                EXPECT_TRUE(status.ok());
                {
                    std::lock_guard<std::mutex> lock(mu);
                    responses.push_back(response);
                }
                completed.fetch_add(1);
                cv.notify_one();
            });
    }

    std::unique_lock<std::mutex> lock(mu);
    cv.wait_for(lock, std::chrono::seconds(5),
                [&] { return completed.load() == kNumRequests; });

    EXPECT_EQ(completed.load(), kNumRequests);
    for (const auto& resp : responses) {
        EXPECT_TRUE(resp.find("pong: async_") != std::string::npos);
    }
}
