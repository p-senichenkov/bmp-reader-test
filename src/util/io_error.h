#pragma once

#include <exception>
#include <string>

namespace bmp {
/// @brief Thrown when BMP cannot be read due to I/O error
class IOError : public std::exception {
private:
    std::string msg_;

public:
    IOError(std::string msg) : msg_("I/O error: " + std::move(msg)) {}

    virtual char const* what() const noexcept override {
        return msg_.c_str();
    }
};
}  // namespace bmp
