#pragma once

#include <iostream>

template<typename T>
class GodObject {
public:
    static T *object_ptr_;

public:
    template<typename... Args>
    static void init(Args &&... args) {
        static T object(std::forward<Args>(args)...);
        object_ptr_ = &object;
    }

    static bool is_initialized() {
        return object_ptr_;
    }

    static T &get() {
        return *object_ptr_;
    }
};

template<typename T>
T *GodObject<T>::object_ptr_ = nullptr;
