#include <common/json/json.hpp>
#include <server/core/algo/order_book/order_book.h>
#include <server/core/core.h>

#include <boost/asio.hpp>
#include <memory>
#include <unordered_map>
#include <list>

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket,
            std::unordered_map<size_t, std::weak_ptr<Session>> &sessions)
        : socket_(std::move(socket)),
          sessions_(sessions) {
    }

    void start() {
        auth();
    }

private:
    void auth() {
        auto self(shared_from_this());
        socket_.async_read_some(
            boost::asio::buffer(data_, max_length),
            [this, self](std::error_code ec, size_t length) {
                if (!ec) {
                    data_[length] = '\0';
                    auto j = nlohmann::json::parse(data_);
                    auto &reg = Registry::get();
                    auto req_type = Registry::get().get_request_type(
                        j[reg.get_field_str(EFields::EFReqType)]);
                    auto &core = Core::get();

                    switch (req_type) {
                        case ERequests::ERAuth:
                            user_id_ = j[reg.get_field_str(EFields::EFUserId)];
                            if (sessions_.find(user_id_) == sessions_.end() ||
                                !sessions_[user_id_].expired()) {
                                boost::asio::async_write(
                                    socket_,
                                    boost::asio::buffer(reg.error_response_.dump() + '\0'),
                                    [this, self](std::error_code ec, size_t /*length*/) {
                                        if (!ec) {
                                            auth();
                                        }
                                    });
                            } else {
                                sessions_[user_id_] = weak_from_this();
                                boost::asio::async_write(
                                    socket_,
                                    boost::asio::buffer(reg.ok_response_.dump() + '\0'),
                                    [this, self](std::error_code ec, size_t /*length*/) {
                                        if (!ec) {
                                            do_write();
                                        }
                                    });
                            }
                            break;
                        case ERequests::ERReg:
                            user_id_ = core.reg();
                            sessions_[user_id_] = weak_from_this();
                            do_write();
                            break;
                        default:
                            assert(false);
                    }
                }
            });
    }

    void do_read() {
        auto self(shared_from_this());
        socket_.async_read_some(
            boost::asio::buffer(data_, max_length),
            [this, self](std::error_code ec, size_t length) {
                if (!ec) {
                    data_[length] = '\0';
                    auto j = nlohmann::json::parse(data_);

                    auto &reg = Registry::get();
                    auto req_type = Registry::get().get_request_type(
                        j[reg.get_field_str(EFields::EFReqType)]);
                    auto &core = Core::get();
                    switch (req_type) {
                        case ERequests::ERViewBook:
                            core.view_book(user_id_);
                            break;
                        case ERequests::ERMakeOrder: {
                            auto &to_notify =
                                (j[reg.get_field_str(EFields::EFSide)] ==
                                 reg.get_side_str(ESide::ESell)
                                 ? core.make_order<ESide::ESell>(
                                        j[reg.get_field_str(EFields::EFQuantity)]
                                            .get<size_t>(),
                                        j[reg.get_field_str(EFields::EFPrice)]
                                            .get<size_t>(),
                                        user_id_)
                                 : core.make_order<ESide::EBuy>(
                                        j[reg.get_field_str(EFields::EFQuantity)]
                                            .get<size_t>(),
                                        j[reg.get_field_str(EFields::EFPrice)]
                                            .get<size_t>(),
                                        user_id_));
                            for (size_t id: to_notify) {
                                do_notify(id);
                            }
                            break;
                        }
                        case ERequests::ERCancelOrder:
                            core.cancel_order(user_id_,
                                              j[reg.get_field_str(EFields::EFOrderId)]);
                            break;
                        case ERequests::ERViewUserInfo:
                            core.user_info(user_id_);
                            break;
                        default:
                            assert(false);
                    }

                    do_write();
                }
            });
    }

    void do_write() {
        auto self(shared_from_this());
        auto response = make_response(user_id_);
        auto str = response.dump();
        boost::asio::async_write(
            socket_, boost::asio::buffer(std::move(str) + '\0'),
            [this, self, response = std::move(response)](std::error_code ec,
                                                         size_t /*length*/) {
                if (!ec) {
                    do_read();
                } else {
                    Core::get().get_to_send(user_id_).push_back(response);
                }
            });
    }

    void do_notify(size_t user_id) {
        if (sessions_[user_id].expired()) {
            return;
        }
        auto session = sessions_[user_id].lock();
        auto response = make_response(user_id);
        auto str = response.dump();
        boost::asio::async_write(
            session->socket_, boost::asio::buffer(std::move(str) + '\0'),
            [this, session, response = std::move(response), user_id](
                std::error_code ec, size_t /*length*/) {
                if (ec) {
                    Core::get().get_to_send(user_id).push_back(response);
                    do_notify(user_id);
                }
            });
    }

    nlohmann::json make_response(size_t user_id) {
        nlohmann::json response;
        for (auto &&j: Core::get().get_to_send(user_id)) {
            response += j;
        }
        Core::get().get_to_send(user_id).resize(0);
        return response;
    }

    tcp::socket socket_;
    size_t user_id_;
    enum {
        max_length = 1024
    };
    char data_[max_length];
    std::unordered_map<size_t, std::weak_ptr<Session>> &sessions_;
};

class Server {
public:
    Server(boost::asio::io_context &io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
          socket_(io_context) {
        std::cout << "Server started! Listen " << port << " port" << std::endl;

        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept(socket_, [this](std::error_code ec) {
            if (!ec) {
                std::make_shared<Session>(std::move(socket_), sessions_)->start();
            }

            do_accept();
        });
    }

    tcp::acceptor acceptor_;
    tcp::socket socket_;
    std::unordered_map<size_t, std::weak_ptr<Session>> sessions_;
};

int main(int argc, char *argv[]) {
    try {
        Registry::init();
        Core::init();
        boost::asio::io_context io_context;

        Server s(io_context,
                 (argc > 1 ? std::stoi(argv[1]) : Registry::get().get_port()));

        io_context.run();
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}