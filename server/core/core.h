#pragma once

#include <common/god_object/god_object.hpp>
#include <common/json/json.hpp>
#include <server/core/user_info/user_info.h>
#include <server/core/algo/order/order.h>
#include <server/core/algo/order_book/order_book.h>

#include <unordered_set>

class Core : public GodObject<Core> {
public:
    template<ESide side, typename... Args>
    std::unordered_set<size_t> &make_order(Args &&... args) {
        to_notify_.clear();
        auto trade_handler = [this](size_t our_order_id, size_t other_order_id,
                                    size_t quantity) {
            const Order &our_order = order_book_.get_order_by_id(our_order_id);
            const Order &other_order = order_book_.get_order_by_id(other_order_id);
            to_send_[other_order.get_user_id()].push_back(other_order.as_json());
            if (our_order.get_user_id() != other_order.get_user_id()) {
                to_notify_.insert(other_order.get_user_id());
                if constexpr (side == ESide::ESell) {
                    users_[other_order.get_user_id()].add_usd(quantity);
                    users_[other_order.get_user_id()].add_rub(
                        -quantity * other_order.get_order_price());
                    users_[our_order.get_user_id()].add_usd(-quantity);
                    users_[our_order.get_user_id()].add_rub(quantity *
                                                            other_order.get_order_price());
                } else {
                    users_[other_order.get_user_id()].add_usd(-quantity);
                    users_[other_order.get_user_id()].add_rub(
                        quantity * other_order.get_order_price());
                    users_[our_order.get_user_id()].add_usd(quantity);
                    users_[our_order.get_user_id()].add_rub(-quantity *
                                                            other_order.get_order_price());
                }
            }
        };
        size_t order_id = order_book_.addOrder<side>(trade_handler, side,
                                                     std::forward<Args>(args)...);
        const Order &order = order_book_.get_order_by_id(order_id);
        users_[order.get_user_id()].add_order(order_id);
        to_send_[order.get_user_id()].push_back(order.as_json());
        return to_notify_;
    }

    void cancel_order(size_t user_id, size_t order_id);

    void view_book(size_t user_id);

    void user_info(size_t user_id);

    std::vector<nlohmann::json> &get_to_send(size_t user_id);

    size_t reg();

private:
    OrderBook order_book_;
    std::unordered_map<size_t, UserInfo> users_;
    std::unordered_map<size_t, std::vector<nlohmann::json>> to_send_;
    size_t last_user_id_ = 0;
    std::unordered_set<size_t> to_notify_;
};
