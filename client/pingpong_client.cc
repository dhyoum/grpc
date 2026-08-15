#include "client/pingpong_client.h"

#include <print>

PingPongClient::PingPongClient(std::shared_ptr<grpc::Channel> channel)
    : stub_(pingpong::PingPong::NewStub(std::move(channel))) {}

grpc::Status PingPongClient::Ping(const std::string& msg, std::string* response_msg) {
    pingpong::PingRequest request;
    request.set_message(msg);

    pingpong::PongResponse response;
    grpc::ClientContext context;

    grpc::Status status = stub_->Ping(&context, request, &response);
    if (status.ok()) {
        *response_msg = response.message();
    }
    return status;
}

void PingPongClient::AsyncPing(const std::string& msg,
                               std::function<void(grpc::Status, const std::string&)> callback) {
    auto* request = new pingpong::PingRequest();
    request->set_message(msg);

    auto* response = new pingpong::PongResponse();
    auto* context = new grpc::ClientContext();

    std::println("[Client] Sending ping: \"{}\"", msg);

    stub_->async()->Ping(context, request, response,
        [request, response, context, callback](grpc::Status s) {
            if (callback) {
                callback(s, response->message());
            }
            delete request;
            delete response;
            delete context;
        });
}
