#pragma once

#include <string>
#include <array>
#include <functional>

namespace com
{
    using readAuxFunc = std::function<bool(uint8_t *, int)>;
    class serial
    {
    private:
        int fd{-1};
        std::string path;
        readAuxFunc read_validator;
        std::array<uint8_t,8192> buf;
        int open();
    public:
        serial(std::string_view path_, readAuxFunc read_validator_);
        ~serial();
        int operator<<(std::string_view msg) const;
        int operator>>(std::string &container);
        static std::string list_ports();
    };
}