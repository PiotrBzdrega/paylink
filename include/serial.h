#pragma once

#include <string>
#include <array>

namespace com
{

    class serial
    {
    private:
        int fd{-1};
        std::string path;
        int open();
        std::array<uint8_t,8192> buf;
    public:
        serial(std::string_view path_, read_validator);
        ~serial();
        int operator<<(std::string_view msg) const;
        int operator>>(std::string &container) const;
        static std::string list_ports();
    };
}