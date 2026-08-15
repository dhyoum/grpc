#include "server/pingpong_service.h"

#include <print>
#include <string>
#include <chrono>
#include <queue>
#include <condition_variable>

// --- Ping (unary) ---

grpc::ServerUnaryReactor* PingPongServiceImpl::Ping(
    grpc::CallbackServerContext* context,
    const pingpong::PingRequest* request,
    pingpong::PongResponse* response) {

    std::println("[Server] Received ping: \"{}\"", request->message());

    response->set_message("pong: " + request->message());

    std::println("[Server] Sending pong: \"{}\"", response->message());

    auto* reactor = context->DefaultReactor();
    reactor->Finish(grpc::Status::OK);
    return reactor;
}

// --- Subscribe (server streaming) ---

class SubscribeReactor : public grpc::ServerWriteReactor<pingpong::StatusIndication> {
public:
    SubscribeReactor(PingPongServiceImpl* service, std::string client_id)
        : service_(service), client_id_(std::move(client_id)) {

        std::println("[Server] Client '{}' subscribed", client_id_);
    }

    // 외부에서 호출: indication을 큐에 넣고 전송 시작
    void Send(const pingpong::StatusIndication& indication) {
        std::lock_guard<std::mutex> lock(mu_);
        queue_.push(indication);
        if (!writing_) {
            writing_ = true;
            NextWrite();
        }
    }

    void OnWriteDone(bool ok) override {
        std::lock_guard<std::mutex> lock(mu_);
        if (!ok || cancelled_) {
            writing_ = false;
            Finish(grpc::Status::OK);
            return;
        }
        if (!queue_.empty()) {
            NextWrite();
        } else {
            writing_ = false;
        }
    }

    void OnCancel() override {
        std::lock_guard<std::mutex> lock(mu_);
        cancelled_ = true;
        std::println("[Server] Client '{}' unsubscribed", client_id_);
        if (!writing_) {
            Finish(grpc::Status::CANCELLED);
        }
    }

    void OnDone() override {
        delete this;
    }

private:
    void NextWrite() {
        current_ = queue_.front();
        queue_.pop();
        StartWrite(&current_);
    }

    PingPongServiceImpl* service_;
    std::string client_id_;
    std::mutex mu_;
    std::queue<pingpong::StatusIndication> queue_;
    pingpong::StatusIndication current_;
    bool writing_ = false;
    bool cancelled_ = false;
};

grpc::ServerWriteReactor<pingpong::StatusIndication>* PingPongServiceImpl::Subscribe(
    grpc::CallbackServerContext* context,
    const pingpong::SubscribeRequest* request) {

    auto* reactor = new SubscribeReactor(this, request->client_id());

    // 구독자 등록 — weak reference로 reactor에 전달
    auto cb = [reactor](const pingpong::StatusIndication& ind) {
        reactor->Send(ind);
    };

    {
        std::lock_guard<std::mutex> lock(subscribers_mu_);
        subscribers_.push_back(cb);
    }

    return reactor;
}

// --- Publish ---

void PingPongServiceImpl::Publish(pingpong::StatusIndication indication) {
    auto now = std::chrono::system_clock::now();
    indication.set_timestamp_ms(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count());

    std::lock_guard<std::mutex> lock(subscribers_mu_);
    for (auto& cb : subscribers_) {
        cb(indication);
    }
}
