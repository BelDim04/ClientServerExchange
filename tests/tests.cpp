#include <server/core/core.h>

#include <gtest/gtest.h>

namespace {
    void init_limits(size_t max_price, size_t max_orders) {
        Registry::init(Registry(max_price, max_orders));
    }
}  // namespace

TEST(OrderBook, Basics) {
    init_limits(3, 1);
    OrderBook order_book;

    Order orders[] = {Order(ESide::ESell, 10, 50, 1),
                      Order(ESide::ESell, 20, 60, 2)};
    order_book.addOrder<ESide::ESell>([](size_t, size_t, size_t) {}, orders[1]);
    order_book.addOrder<ESide::ESell>([](size_t, size_t, size_t) {}, orders[0]);

    int count = 0;
    for (const auto &order: order_book) {
        ASSERT_EQ(order.get_order_price(), orders[count].get_order_price());
        ++count;
    }
    ASSERT_EQ(count, 2);

    Order order3(ESide::EBuy, 20, 55, 3);
    order_book.addOrder<ESide::EBuy>([](size_t, size_t, size_t) {}, order3);

    count = 0;
    size_t expected_prices[] = {55, 60};
    size_t expected_quantities[] = {10, 20};
    for (const auto &order: order_book) {
        ASSERT_EQ(order.get_order_price(), expected_prices[count]);
        ASSERT_EQ(order.get_quantity(), expected_quantities[count]);
        ++count;
    }
    ASSERT_EQ(count, 2);
    ASSERT_EQ(order_book.get_order_by_id(1).get_quantity(), 0);

    order_book.addOrder<ESide::EBuy>([](size_t, size_t, size_t) {}, order3);
    Order order4(ESide::ESell, 18, 45, 3);
    order_book.addOrder<ESide::ESell>([](size_t, size_t, size_t) {}, order4);

    count = 0;
    size_t more_expected_prices[] = {55, 60};
    size_t more_expected_quantities[] = {12, 20};
    for (const auto &order: order_book) {
        ASSERT_EQ(order.get_order_price(), more_expected_prices[count]);
        ASSERT_EQ(order.get_quantity(), more_expected_quantities[count]);
        ++count;
    }
    ASSERT_EQ(count, 2);
    ASSERT_EQ(order_book.get_order_by_id(4).get_quantity(), 0);
}

TEST(Core, CoreBasics) {
    if (!Core::is_initialized()) {
        Core::init();
    }
    if (!Registry::is_initialized()) {
        Registry::init();
    }

    OrderBook order_book;
    Core::get().make_order<ESide::EBuy>(10, 60, 1);
    ASSERT_EQ(Core::get().get_to_send(1).size(), 1u);
    Core::get().make_order<ESide::ESell>(20, 50, 2);
    ASSERT_EQ(Core::get().get_to_send(1).size(), 2u);
    ASSERT_EQ(Core::get().get_to_send(2).size(), 1u);
    Core::get().view_book(1);
    ASSERT_EQ(Core::get().get_to_send(1).size(), 3u);
    ASSERT_EQ(Core::get().get_to_send(1)[2].size(), 1u);

    Core::get().user_info(1);
    ASSERT_EQ(Core::get().get_to_send(1)[3]["USD"].get<int>(), 10);
    ASSERT_EQ(Core::get().get_to_send(1)[3]["RUB"].get<int>(), -600);
    Core::get().user_info(2);
    ASSERT_EQ(Core::get().get_to_send(2)[1]["USD"].get<int>(), -10);
    ASSERT_EQ(Core::get().get_to_send(2)[1]["RUB"].get<int>(), 600);
}
