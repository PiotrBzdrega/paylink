#include <string>
#include <expected>
#include <array>
#include <functional>
#include <cstdint>

namespace com
{
    template <typename T>
    using expect = std::expected<T, std::string>;
    class Serial
    {
    private:
        std::string port_;
        int event_fd_{-1}; // eventfd for wakeup select
        int tty_fd_{-1};   // file descriptor for serial port
        std::array<char, 1024> r_buffer_;
        std::array<char, 1024> wr_buffer_;

    public:
        Serial(std::string_view port /* , int baudrate */);
        ~Serial();
        expect<void> open();
        expect<void> reconnect(bool keep_valid_fd = false);
        expect<void> configure();
        expect<std::string_view> read(std::move_only_function<bool(std::string_view)> pred, uint32_t timeout_ms = 500);
        expect<int> write(std::string_view data, uint32_t timeout_ms = 500);
        void interrupt_wait();
    };
}