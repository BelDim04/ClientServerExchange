#include <common/registry/registry.h>

#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

namespace {
    template<typename T>
    std::pair<std::string, T> make_message_pair(EFields field, const T &t) {
        return {Registry::get().get_field_str(field), t};
    }
}

class Client {
public:
    Client(boost::asio::io_context &io_context,
           const tcp::resolver::results_type &endpoints)
        : socket_(io_context) {
        do_connect(endpoints);
    }

private:
    void close() {
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
        socket_.close();
    }

    void do_connect(const tcp::resolver::results_type &endpoints) {
        boost::asio::async_connect(
            socket_, endpoints,
            [this](boost::system::error_code ec, const tcp::endpoint &) {
                if (!ec) {
                    auth();
                }
            });
    }

    void do_read() {
        boost::asio::async_read_until(
            socket_, buffer_, '\0', [this](std::error_code ec, size_t) {
                if (!ec) {
                    std::istream is(&buffer_);
                    std::string line(std::istreambuf_iterator<char>(is), {});
                    auto j = nlohmann::json::parse(line);
                    std::cout << "Response: " << j.dump() << "\n";
                    do_read();
                }
            });
    }

    void auth() {
        auto &registry = Registry::get();
        std::cout << "Menu:\n";
        std::cout << int(ERequests::ERExit) << ") "
                  << registry.get_request_str(ERequests::ERExit) << '\n';
        std::cout << int(ERequests::ERAuth) << ") "
                  << registry.get_request_str(ERequests::ERAuth) << '\n';
        std::cout << int(ERequests::ERReg) << ") "
                  << registry.get_request_str(ERequests::ERReg) << '\n';

        int menu_option_num;
        std::cin >> menu_option_num;
        switch (ERequests(menu_option_num)) {
            case ERequests::ERReg: {
                do_read();
                do_write(make_message_pair(EFields::EFReqType,
                                           Registry::get().get_request_str(ERequests::ERReg)));
                break;
            }
            case ERequests::ERAuth: {
                size_t user_id;
                std::cout << "Enter UserId: ";
                std::cin >> user_id;
                auto req = make_req(
                    make_message_pair(EFields::EFReqType,
                                      registry.get_request_str(ERequests::ERAuth)),
                    make_message_pair(EFields::EFUserId, user_id));
                boost::asio::write(socket_, boost::asio::buffer(req, req.size()));
                boost::asio::read_until(socket_, buffer_, '\0');
                std::istream is(&buffer_);
                std::string line(std::istreambuf_iterator<char>(is), {});
                auto j = nlohmann::json::parse(line);
                std::cout << "Response: " << j.dump() << "\n";
                if (j == registry.error_response_) {
                    auth();
                } else {
                    do_read();
                    menu();
                }
                break;
            }
            case ERequests::ERExit: {
                close();
                break;
            }
            default: {
                std::cout << "Unknown menu option\n" << std::endl;
                auth();
            }
        }
    }

    void menu() {
        auto &registry = Registry::get();
        std::cout << "Menu:\n";
        for (const auto &[id, str]: registry.get_requests_str_map()) {
            if (id == ERequests::ERReg || id == ERequests::ERAuth)
                continue;
            std::cout << int(id) << ") " << str << '\n';
        }

        int menu_option_num;
        std::cin >> menu_option_num;
        switch (ERequests(menu_option_num)) {
            case ERequests::ERViewBook: {
                do_write(
                    make_message_pair(EFields::EFReqType,
                                      registry.get_request_str(ERequests::ERViewBook)));
                break;
            }
            case ERequests::ERViewUserInfo: {
                do_write(make_message_pair(
                    EFields::EFReqType,
                    registry.get_request_str(ERequests::ERViewUserInfo)));
                break;
            }
            case ERequests::ERMakeOrder: {
                std::cout << "Enter Side Quantity Price: ";
                std::string side;
                int q, p;
                std::cin >> side >> q >> p;
                if (q <= 0 || p <= 0 || !registry.is_valid_side_str(side)) {
                    std::cout << "Incorrect data\n";
                    menu();
                } else {
                    do_write(make_message_pair(
                                 EFields::EFReqType,
                                 registry.get_request_str(ERequests::ERMakeOrder)),
                             make_message_pair(EFields::EFSide, side),
                             make_message_pair(EFields::EFQuantity, q),
                             make_message_pair(EFields::EFPrice, p));
                }
                break;
            }
            case ERequests::ERCancelOrder: {
                std::cout << "Enter order_id: ";
                size_t order_id;
                std::cin >> order_id;
                do_write(make_message_pair(
                             EFields::EFReqType,
                             registry.get_request_str(ERequests::ERCancelOrder)),
                         make_message_pair(EFields::EFOrderId, order_id));
                break;
            }
            case ERequests::ERExit: {
                close();
                break;
            }
            default: {
                std::cout << "Unknown menu option\n" << std::endl;
                menu();
            }
        }
    }

    template<typename... Args>
    void do_write(const Args &... args) {
        auto request = make_req(args...);
        boost::asio::async_write(socket_,
                                 boost::asio::buffer(request, request.size()),
                                 [this](std::error_code ec, size_t) {
                                     if (!ec) {
                                         menu();
                                     }
                                 });
    }

    template<typename... Args>
    std::string make_req(const Args &... args) {
        nlohmann::json req;
        auto add = [&req](auto &pair) {
            req[pair.first] = pair.second;
        };

        [[maybe_unused]] int dummy[sizeof...(Args)] = {(add(args), 0)...};
        return req.dump();
    }

    tcp::socket socket_;
    boost::asio::streambuf buffer_;
};

int main(int argc, char *argv[]) {
    try {
        Registry::init();
        boost::asio::io_context io_context;

        tcp::resolver resolver(io_context);

        Client client(
            io_context,
            resolver.resolve(
                tcp::v4(), (argc > 1 ? argv[1] : "127.0.0.1"),
                (argc > 2 ? argv[2] : std::to_string(Registry::get().get_port()))));

        std::thread t([&]() {
            io_context.run();
        });
        io_context.run();

        t.join();
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}