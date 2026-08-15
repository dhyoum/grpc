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
#include "server/pingpong_service.h"

class PingPongTest : public ::testing::Test {
protected:
    void SetUp() override {
        service_ = std::make_unique<PingPongServiceImpl>();

        grpc::ServerBuilder builder;
        builder.AddListeningPort("0.0.0.0:0",
                                 grpc::InsecureServerCredentials(),
                                 &port_);
        builder.RegisterService(service_.get());
        server_ = builder.BuildAndStart();
        ASSERT_NE(server_, nullptr) << "Server failed to start";
        ASSERT_GT(port_, 0) << "Failed to get assigned port";

        auto channel = grpc::CreateChannel(
            "localhost:" + std::to_string(port_),
            grpc::InsecureChannelCredentials());
        stub_ = pingpong::PingPong::NewStub(channel);
    }

    void TearDown() override {
        if (server_) {
            server_->Shutdown();
        }
    }

    int port_ = 0;
    std::unique_ptr<PingPongServiceImpl> service_;
    std::unique_ptr<grpc::Server> server_;
    std::unique_ptr<pingpong::PingPong::Stub> stub_;
};

// --- Ping tests ---

TEST_F(PingPongTest, SyncPingPong) {
    pingpong::PingRequest request;
    request.set_message("hello");

    pingpong::PongResponse response;
    grpc::ClientContext context;

    grpc::Status status = stub_->Ping(&context, request, &response);

    EXPECT_TRUE(status.ok()) << "RPC failed: " << status.error_message();
    EXPECT_EQ(response.message(), "pong: hello");
}

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

TEST_F(PingPongTest, EmptyMessage) {
    pingpong::PingRequest request;
    request.set_message("");

    pingpong::PongResponse response;
    grpc::ClientContext context;

    grpc::Status status = stub_->Ping(&context, request, &response);

    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.message(), "pong: ");
}

// --- Subscribe/Indication tests ---

TEST_F(PingPongTest, SubscribeReceivesIndications) {
    // 구독 시작 (별도 스레드에서 blocking read)
    std::vector<pingpong::StatusIndication> received;
    std::mutex mu;
    std::condition_variable cv;

    grpc::ClientContext stream_context;

    auto reader_thread = std::thread([&] {
        pingpong::SubscribeRequest req;
        req.set_client_id("test-client");

        auto reader = stub_->Subscribe(&stream_context, req);

        pingpong::StatusIndication indication;
        while (reader->Read(&indication)) {
            std::lock_guard<std::mutex> lock(mu);
            received.push_back(indication);
            cv.notify_one();
        }
    });

    // 서버가 구독을 처리할 시간
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // 서버에서 indication 발행
    pingpong::StatusIndication ind1;
    ind1.set_level(pingpong::StatusIndication::INFO);
    ind1.set_source("system");
    ind1.set_message("service started");
    service_->Publish(ind1);

    pingpong::StatusIndication ind2;
    ind2.set_level(pingpong::StatusIndication::WARNING);
    ind2.set_source("memory");
    ind2.set_message("usage above 80%");
    service_->Publish(ind2);

    // 수신 대기
    {
        std::unique_lock<std::mutex> lock(mu);
        cv.wait_for(lock, std::chrono::seconds(3),
                    [&] { return received.size() >= 2; });
    }

    // 클라이언트 쪽에서 스트림 취소
    stream_context.TryCancel();
    reader_thread.join();

    ASSERT_EQ(received.size(), 2);
    EXPECT_EQ(received[0].level(), pingpong::StatusIndication::INFO);
    EXPECT_EQ(received[0].source(), "system");
    EXPECT_EQ(received[0].message(), "service started");
    EXPECT_GT(received[0].timestamp_ms(), 0);

    EXPECT_EQ(received[1].level(), pingpong::StatusIndication::WARNING);
    EXPECT_EQ(received[1].source(), "memory");
    EXPECT_EQ(received[1].message(), "usage above 80%");
}

TEST_F(PingPongTest, MultipleSubscribersReceiveIndications) {
    // 두 클라이언트 구독
    std::vector<pingpong::StatusIndication> received1;
    std::vector<pingpong::StatusIndication> received2;
    std::mutex mu1, mu2;
    std::condition_variable cv1, cv2;

    grpc::ClientContext ctx1, ctx2;

    auto reader1 = std::thread([&] {
        pingpong::SubscribeRequest req;
        req.set_client_id("client-1");
        auto reader = stub_->Subscribe(&ctx1, req);

        pingpong::StatusIndication ind;
        while (reader->Read(&ind)) {
            std::lock_guard<std::mutex> lock(mu1);
            received1.push_back(ind);
            cv1.notify_one();
        }
    });

    auto reader2 = std::thread([&] {
        pingpong::SubscribeRequest req;
        req.set_client_id("client-2");
        auto reader = stub_->Subscribe(&ctx2, req);

        pingpong::StatusIndication ind;
        while (reader->Read(&ind)) {
            std::lock_guard<std::mutex> lock(mu2);
            received2.push_back(ind);
            cv2.notify_one();
        }
    });

    // 두 구독 모두 등록될 시간
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // indication 발행
    pingpong::StatusIndication ind;
    ind.set_level(pingpong::StatusIndication::ERROR);
    ind.set_source("disk");
    ind.set_message("disk full");
    service_->Publish(ind);

    // 두 클라이언트 모두 수신 대기
    {
        std::unique_lock<std::mutex> lock(mu1);
        cv1.wait_for(lock, std::chrono::seconds(3),
                     [&] { return received1.size() >= 1; });
    }
    {
        std::unique_lock<std::mutex> lock(mu2);
        cv2.wait_for(lock, std::chrono::seconds(3),
                     [&] { return received2.size() >= 1; });
    }

    // 스트림 종료
    ctx1.TryCancel();
    ctx2.TryCancel();
    reader1.join();
    reader2.join();

    // 검증: 둘 다 같은 indication을 받았는지
    ASSERT_EQ(received1.size(), 1);
    ASSERT_EQ(received2.size(), 1);

    EXPECT_EQ(received1[0].level(), pingpong::StatusIndication::ERROR);
    EXPECT_EQ(received1[0].source(), "disk");
    EXPECT_EQ(received1[0].message(), "disk full");

    EXPECT_EQ(received2[0].level(), pingpong::StatusIndication::ERROR);
    EXPECT_EQ(received2[0].source(), "disk");
    EXPECT_EQ(received2[0].message(), "disk full");
}
