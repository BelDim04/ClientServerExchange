#pragma once

#include <common/registry/registry.h>
#include <server/core/algo/order/order.h>

#include <boost/intrusive/slist.hpp>
#include <vector>
#include <deque>

class OrderBook {
    struct OrderRepresentation : public Order, public boost::intrusive::slist_base_hook<> {
        using Order::Order;

        template<typename... Args>
        OrderRepresentation(Args &&... args)
            : Order(std::forward<Args>(args)...) {
        }
    };

    using PriceLevel =
        boost::intrusive::slist<OrderRepresentation,
            boost::intrusive::cache_last<true>>;

    struct const_iterator {
        const OrderBook *ob_;
        std::vector<PriceLevel>::const_iterator level_it_;
        PriceLevel::const_iterator order_it_;
        using iterator_category = std::forward_iterator_tag;

        const_iterator(std::vector<PriceLevel>::const_iterator l_it,
                       PriceLevel::const_iterator o_it, const OrderBook *ob)
            : ob_(ob),
              level_it_(l_it),
              order_it_(o_it) {};

        const_iterator(std::vector<PriceLevel>::const_iterator l_it,
                       const OrderBook *ob)
            : ob_(ob),
              level_it_(l_it) {
            if (level_it_ != ob->price_levels_.end() && !level_it_->empty())
                order_it_ = level_it_->cbegin();
        }

        const_iterator(const const_iterator &it) = default;

        const_iterator &operator=(const const_iterator &it) = default;

        const_iterator &operator++() {
            ++order_it_;
            if (order_it_ != level_it_->end())
                return *this;
            ++level_it_;
            while (level_it_ != ob_->price_levels_.end() && level_it_->empty()) {
                ++level_it_;
            }
            return *this = const_iterator(level_it_, ob_);
        };

        const_iterator operator++(int) {
            const_iterator temp = *this;
            ++(*this);
            return temp;
        };

        const Order &operator*() const {
            return *order_it_;
        };

        bool operator==(const const_iterator &other) {
            return (level_it_ == other.level_it_ &&
                    (level_it_ == ob_->price_levels_.end() ||
                     order_it_ == other.order_it_));
        }

        bool operator!=(const const_iterator &other) {
            return !(*this == other);
        }
    };

public:
    OrderBook();

    template<ESide side, typename F, typename... Args>
    size_t addOrder(F trade_handler, Args &&... args) {
        orders_arena_.emplace_back(std::forward<Args>(args)...);
        Order &order = orders_arena_.back();
        order.order_id_ = orders_arena_.size() - 1;

        auto isPriceSatisfying = [&]() {
            if constexpr (side == ESide::ESell) {
                return order.price_ <= bid_max_;
            }
            return order.price_ >= ask_min_;
        };

        while (isPriceSatisfying()) {
            auto level = price_levels_.begin();
            if constexpr (side == ESide::ESell)
                level += bid_max_;
            else
                level += ask_min_;

            auto it = level->begin();
            while (it != level->end()) {
                if (it->quantity_ < order.quantity_) {
                    trade_handler(order.order_id_, it->order_id_,
                                  it->quantity_);
                    order.quantity_ -= it->quantity_;
                    it->quantity_ = 0;
                } else {
                    trade_handler(order.order_id_, level->front().order_id_,
                                  order.quantity_);
                    it->quantity_ -= order.quantity_;
                    order.quantity_ = 0;
                    if (it->quantity_ == 0) {
                        ++it;
                    }
                    while (level->begin() != it) {
                        level->pop_front();
                    }
                    return orders_arena_.size() - 1;
                }
                ++it;
            }

            level->clear();
            if constexpr (side == ESide::ESell)
                --bid_max_;
            else
                ++ask_min_;
        }

        if (price_levels_.size() <= order.price_) {
            price_levels_.resize(order.price_ + 2);
        }
        price_levels_[order.price_].push_back(orders_arena_.back());

        if constexpr (side == ESide::ESell)
            ask_min_ = std::min(ask_min_, order.price_);
        else
            bid_max_ = std::max(bid_max_, order.price_);

        return orders_arena_.size() - 1;
    }

    void cancel_order(size_t orderId);

    Order &get_order_by_id(size_t id);

    const Order &get_order_by_id(size_t id) const;

    const_iterator begin() const;

    const_iterator end() const;

private:
    std::deque<OrderRepresentation> orders_arena_;
    std::vector<PriceLevel> price_levels_;
    size_t ask_min_;
    size_t bid_max_;
};
