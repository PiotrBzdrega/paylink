#pragma once
#include <print>
#include <source_location>
#include <mutex>
#include <chrono>
#include <fstream>
#include <format>

// Custom formatter for std::vector
// Concept for container types
template <typename T>
concept Container = requires(T container) {
    typename T::value_type;
    container.begin();
    container.end();
    container.size();
};

template <Container C>
struct std::formatter<C>
{
    constexpr auto parse(std::format_parse_context &ctx)
    {
        return ctx.begin(); // No format spec needed
    }

    auto format(const C &vec, std::format_context &ctx) const
    {
        auto out = ctx.out();
        *out++ = '[';
        for (size_t i = 0; i < vec.size(); ++i)
        {
            out = std::format_to(out, "{}", vec[i]);
            if (i != vec.size() - 1)
            {
                out = std::format_to(out, ", ");
            }
        }
        *out++ = ']';
        return out;
    }
};

namespace impl
{
    template <class... Args>
    struct format_string_with_location
    {
        template <class T>
            requires std::constructible_from<std::format_string<Args...>, T const &>
        consteval format_string_with_location(T const &fmt, std::source_location loc = std::source_location::current())
            : fmt{fmt}, loc{loc}
        {
        }

        std::format_string<Args...> fmt;
        std::source_location loc;
    };
}

template <class... Args>
using format_string_with_location = impl::format_string_with_location<std::type_identity_t<Args>...>;

namespace mik
{
    enum class LogLevel
    {
        TRACE,
        DEBUG,
        INFO,
        ERROR,
        NONE
    };

    class logger
    {
    private:
        static inline LogLevel level = LogLevel::NONE; // Default level;
        static inline std::mutex s_mutex;              // Thread safety
        static inline std::ofstream *outstream{nullptr};
        static inline bool standard_output{};
        template <class... Args>
        static void log(LogLevel lvl, format_string_with_location<Args...> fmt, Args &&...args)
        {
            if (static_cast<int>(lvl) < static_cast<int>(level))
            {
                return;
            }

            const char *level_str = [&]
            {
                switch (lvl)
                {
                case LogLevel::TRACE:
                    return "TRACE";
                case LogLevel::DEBUG:
                    return "DEBUG";
                case LogLevel::INFO:
                    return "INFO";
                case LogLevel::ERROR:
                    return "ERROR";
                case LogLevel::NONE:
                    return "NONE";
                default:
                    return "UNKNOWN";
                }
            }();

            {
                std::scoped_lock lock{s_mutex};
                if (outstream)
                {
                    std::print(*outstream, "[{:%Y-%m-%d %H:%M:%S}] [{}] [{}:{}]: ", std::chrono::zoned_time{std::chrono::current_zone(), std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now())}, level_str, fmt.loc.file_name(), fmt.loc.line());
                    std::println(*outstream, fmt.fmt, std::forward<Args>(args)...);
                }

                if (standard_output)
                {
                    std::print("[{:%Y-%m-%d %H:%M:%S}] [{}] [{}:{}]: ", std::chrono::zoned_time{std::chrono::current_zone(), std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now())}, level_str, fmt.loc.file_name(), fmt.loc.line());
                    std::println(fmt.fmt, std::forward<Args>(args)...);
                }
            }
        }

    public:
        template <class... Args>
        static void trace(format_string_with_location<Args...> fmt, Args &&...args)
        {
            log(mik::LogLevel::TRACE, fmt, std::forward<Args>(args)...);
        }

        template <class... Args>
        static void debug(format_string_with_location<Args...> fmt, Args &&...args)
        {
            log(mik::LogLevel::DEBUG, fmt, std::forward<Args>(args)...);
        }

        template <class... Args>
        static void info(format_string_with_location<Args...> fmt, Args &&...args)
        {
            log(mik::LogLevel::INFO, fmt, std::forward<Args>(args)...);
        }

        template <class... Args>
        static void error(format_string_with_location<Args...> fmt, Args &&...args)
        {
            log(mik::LogLevel::ERROR, fmt, std::forward<Args>(args)...);
        }

        static void setup(bool standard_output = true, std::ofstream *stream = nullptr, LogLevel lvl = level)
        {
            set_stdout(standard_output);
            set_stream(stream);
            set_level(lvl);
        }
        /* Set global log level */
        static void set_level(LogLevel lvl)
        {
            level = lvl;
        }
        static void set_stream(std::ofstream *stream)
        {
            std::scoped_lock lock{s_mutex};
            outstream = stream;
        }
        static void set_stdout(bool standard_output)
        {
            std::scoped_lock lock{s_mutex};
            standard_output = standard_output;
        }
    };
}
