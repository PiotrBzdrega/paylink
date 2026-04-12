#include <condition_variable>
#include <mutex>
#include <thread>
#include <ranges>
#include <future>
#include <optional>
#include "task.h"

namespace com
{
    class scheduler
    {
    private:
        std::jthread thr_worker;
        std::vector<task> tasks;
        std::mutex mtx;
        std::condition_variable cv;
        std::uint16_t counter;
        struct task_in_sight_t
        {
            std::optional<std::size_t> index{};
            std::size_t id{};
            void reset()
            {
                index.reset();
                id = 0;
            }
            bool available() const { return index.has_value(); }
        } task_in_sight;
        void worker(std::stop_token st);
        bool remove_task_(std::size_t id_);

    public:
        std::size_t submit_periodic_task(task_t &&t_, std::chrono::milliseconds interval_, uint32_t repeat_ = 0, std::chrono::seconds delay_ = std::chrono::seconds(0));
        bool remove_task(std::size_t id_);
        std::size_t submit_task(task_t &&t_, std::chrono::seconds delay_ = std::chrono::seconds(0));
        void stop();
        scheduler();
        ~scheduler();
    };
}