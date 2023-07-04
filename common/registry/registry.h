#pragma once

#include <common/pre.h>
#include <common/god_object/god_object.hpp>
#include <common/json/json.hpp>

#include <unordered_map>
#include <map>

class Registry : public GodObject<Registry> {
public:
    Registry();

    short get_port();

    size_t get_expected_max_price();

    size_t get_expected_max_orders();

    ERequests get_request_type(const std::string &str);

    std::map<ERequests, std::string> &get_requests_str_map();

    const std::string &get_field_str(EFields field);

    const std::string &get_request_str(ERequests req);

    ESide get_side(const std::string &str);

    const std::string &get_side_str(ESide side);

    bool is_valid_side_str(const std::string &str);

    Registry(size_t max_price, size_t max_orders);

    std::string error_ = "Error";
    std::string ok_ = "Ok";
    nlohmann::json error_response_;
    nlohmann::json ok_response_;

private:
    std::unordered_map<std::string, ERequests> make_requests_map();

    std::map<ERequests, std::string> make_requests_str_map();

    std::unordered_map<EFields, std::string> make_fields_str_map();

    std::unordered_map<std::string, EFields> make_fields_map();

    std::unordered_map<ESide, std::string> make_side_str_map();

    std::unordered_map<std::string, ESide> make_side_map();

    short port_ = 5555;
    size_t expected_max_price_ = 1000000;
    size_t expected_max_orders_ = 1000000;
    std::unordered_map<std::string, ERequests> requests_map_;
    std::map<ERequests, std::string> requests_str_map_;
    std::unordered_map<EFields, std::string> fields_str_map_;
    std::unordered_map<std::string, EFields> fields_map_;
    std::unordered_map<ESide, std::string> side_str_map_;
    std::unordered_map<std::string, ESide> side_map_;
};
