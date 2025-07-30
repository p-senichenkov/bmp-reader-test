#pragma once

#include <exception>
#include <string>

namespace bmp {
/// @brief This error is thrown when BMP cannot be read due to invalid data, mainly in headers
class InvalidBMPError : public std::exception {
private:
    std::string msg_;

public:
    InvalidBMPError(std::string&& msg) : msg_("Invalid BMP: " + std::move(msg)) {}

    virtual char const* what() const noexcept override {
        return msg_.c_str();
    }

    /// @brief Check condition @c cond, throw @c InvalidBMPError if it's not @c true
    static void Assert(bool cond, std::string&& msg) {
        if (!cond) {
            throw InvalidBMPError(std::move(msg));
        }
    }
};
}  // namespace bmp
