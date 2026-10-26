#pragma once

#include <thread>
#include <condition_variable>
#include <mutex>
#include <utility>
#include <functional>

namespace uc
{
    class peripheral_job
    {
    private:
        std::move_only_function<void(std::mutex &mtx,
                                                     std::condition_variable &cv, bool &done)> job_;
        std::move_only_function<void()> cancel_job_;
        std::mutex mtx_;
        std::condition_variable cv_;
        std::jthread worker_;
        bool done_{false};
        void run(std::stop_token stop_token);

    public:
        peripheral_job(std::move_only_function<void(std::mutex &mtx,
                                                     std::condition_variable &cv, bool &done)> &&job , std::move_only_function<void()> &&cancel_job) : job_(std::move(job)) , cancel_job_(std::move(cancel_job) ) {}
        bool resume();
        bool done();
        void wait_for_completion();
        void start()
        {
            if (!worker_.joinable())
            {
                worker_ = std::jthread(std::bind_front(&peripheral_job::run, this));
            }
        }
        void stop();
        ~peripheral_job();
    };
}
