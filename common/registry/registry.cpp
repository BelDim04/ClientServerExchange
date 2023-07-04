#include "registry.h"

Registry::Registry() {
    requests_map_ = make_requests_map();
    requests_str_map_ = make_requests_str_map();
    fields_str_map_ = make_fields_str_map();
    fields_map_ = make_fields_map();
    side_str_map_ = make_side_str_map();
    side_map_ = make_side_map();
    error_response_[get_field_str(EFields::EFMessage)] = error_;
    ok_response_[get_field_str(EFields::EFMessage)] = ok_;
}

short Registry::get_port() {
    return port_;
}

size_t Registry::get_expected_max_price() {
    return expected_max_price_;
}

size_t Registry::get_expected_max_orders() {
    return expected_max_orders_;
}

ERequests Registry::get_request_type(const std::string &str) {
    return requests_map_[str];
}

std::map<ERequests, std::string> &Registry::get_requests_str_map() {
    return requests_str_map_;
}

std::unordered_map<std::string, ERequests> Registry::make_requests_map() {
    return {
        {"Book",   ERequests::ERViewBook},
        {"Order",  ERequests::ERMakeOrder},
        {"Cancel", ERequests::ERCancelOrder},
        {"Info",   ERequests::ERViewUserInfo},
        {"Auth",   ERequests::ERAuth},
        {"Reg",    ERequests::ERReg},
        {"Exit",   ERequests::ERExit}};
}

std::map<ERequests, std::string> Registry::make_requests_str_map() {
    std::map<ERequests, std::string> res;
    for (const auto &[str, req]: requests_map_) {
        res[req] = str;
    }
    return res;
}

std::unordered_map<EFields, std::string> Registry::make_fields_str_map() {
    return {{EFields::EFOrderId,  "OrderId"},
            {EFields::EFUserId,   "UserId"},
            {EFields::EFSide,     "Side"},
            {EFields::EFQuantity, "Quantity"},
            {EFields::EFPrice,    "Price"},
            {EFields::EFReqType,  "ReqType"},
            {EFields::EFMessage,  "Message"}};
}

std::unordered_map<std::string, EFields> Registry::make_fields_map() {
    std::unordered_map<std::string, EFields> res;
    for (const auto &[field, str]: fields_str_map_) {
        res[str] = field;
    }
    return res;
}

const std::string &Registry::get_field_str(EFields field) {
    return fields_str_map_[field];
}

const std::string &Registry::get_request_str(ERequests req) {
    return requests_str_map_[req];
}

std::unordered_map<ESide, std::string> Registry::make_side_str_map() {
    return {{ESide::ESell, "Sell"},
            {ESide::EBuy,  "Buy"}};
}

std::unordered_map<std::string, ESide> Registry::make_side_map() {
    std::unordered_map<std::string, ESide> res;
    for (const auto &[side, str]: side_str_map_) {
        res[str] = side;
    }
    return res;
}

ESide Registry::get_side(const std::string &str) {
    return side_map_[str];
}

const std::string &Registry::get_side_str(ESide side) {
    return side_str_map_[side];
}

bool Registry::is_valid_side_str(const std::string &str) {
    return side_map_.find(str) != side_map_.end();
}

Registry::Registry(size_t max_price, size_t max_orders)
    : Registry() {
    expected_max_price_ = max_price;
    expected_max_orders_ = max_orders;
}
