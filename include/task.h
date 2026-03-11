#include <thread>
#include <functional>
#include <chrono>
#include <optional>


namespace com
{
    using task_t = std::move_only_function<void()>;
    using repeat_t = std::optional<uint32_t>;
    using clock_ms = std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds>;

    class task
    {
    private:
        task_t t;
        std::chrono::milliseconds interval;
        std::size_t id;
        repeat_t repeat;
        clock_ms next_time_call{};
        void specify_future_call();

    public:
        explicit task(task_t &&t_, std::chrono::milliseconds interval_, std::size_t id_, repeat_t repeat_, std::chrono::seconds startup_delay_ = std::chrono::seconds(0)) : 
        t(std::move(t_)), interval(interval_), id(id_), repeat(repeat_), next_time_call(std::chrono::floor<std::chrono::milliseconds>(std::chrono::steady_clock::now()) - interval_ + startup_delay_) {}

        auto get_next_time_call() const { return next_time_call; }
        auto get_id() const { return id; }
        auto get_interval() const { return interval; }
        auto get_repeat() const { return repeat; }
        /**
         * @brief execute task, then override next future time call
         * @return if repeat counter reached zero
         */
        bool operator()();
    };
}