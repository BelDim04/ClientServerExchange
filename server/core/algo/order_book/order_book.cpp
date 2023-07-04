#include "order_book.h"

OrderBook::OrderBook() {
    if (!Registry::is_initialized()) {
        Registry::init();
    }
    price_levels_.reserve(Registry::get().get_expected_max_price() + 2);
    // orders_arena_.reserve(Registry::get().get_expected_max_orders());
    ask_min_ = price_levels_.size() - 1;
    bid_max_ = 0;
}

void OrderBook::cancel_order(size_t order_id) {
    auto& order = get_order_by_id(order_id);
    order.quantity_ = 0;
    price_levels_[order.get_order_price()].erase(
        PriceLevel::iterator(static_cast<OrderRepresentation&>(order).this_ptr())
        );
}

Order &OrderBook::get_order_by_id(size_t id) {
    return orders_arena_[id];
}

const Order &OrderBook::get_order_by_id(size_t id) const {
    return orders_arena_[id];
}

OrderBook::const_iterator OrderBook::begin() const {
    auto it = price_levels_.cbegin();
    while (it != price_levels_.end() && it->empty()) {
        ++it;
    }
    return const_iterator(it, this);
}

OrderBook::const_iterator OrderBook::end() const {
    return const_iterator(price_levels_.end(), this);
}
