#include "order.h"

nlohmann::json Order::as_json() const {
    nlohmann::json res;
    res["Side"] = (side_ == ESide::ESell ? "Sell" : "Buy");
    res["InitialQuantity"] = initial_quantity_;
    res["Quantity"] = quantity_;
    res["Price"] = price_;
    res["UserId"] = user_id_;
    res["OrderId"] = order_id_;
    return res;
}

size_t Order::get_order_id() const {
    return order_id_;
}

size_t Order::get_quantity() const {
    return quantity_;
}

size_t Order::get_user_id() const {
    return user_id_;
}

size_t Order::get_order_price() const {
    return price_;
}
