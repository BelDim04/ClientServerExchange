#pragma once

#include <common/json/json.hpp>
#include <common/pre.h>

class Order {
    friend class OrderBook;

public:
    Order(ESide side, size_t quantity, size_t price, size_t user_id)
        : side_(side),
          initial_quantity_(quantity),
          quantity_(quantity),
          price_(price),
          user_id_(user_id) {};

    nlohmann::json as_json() const;

    size_t get_order_id() const;

    size_t get_quantity() const;

    size_t get_user_id() const;

    size_t get_order_price() const;

private:
    ESide side_;
    const size_t initial_quantity_;
    size_t quantity_;
    size_t price_;
    size_t user_id_;
    size_t order_id_;
};
