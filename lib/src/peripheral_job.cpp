#include "peripheral_job.h"

namespace uc
{
    void peripheral_job::run(std::stop_token stop_token)
    {
        while (!stop_token.stop_requested())
        {
            job_(mtx_,cv_,done_);

            {
                std::unique_lock<std::mutex> lock(mtx_);
                cv_.wait(lock, [this]
                         { return !done_; });
            }
        }
    }

    bool peripheral_job::resume()
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (!done_)
        {
            return false; // Job is still running
        }

        done_ = false;
        cv_.notify_one();
        return true;
    }

    bool peripheral_job::done()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        return done_;
    }

    void peripheral_job::wait_for_completion()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this]
                 { return done_; });
    }

    void peripheral_job::stop()
    {
        if (worker_.joinable())
        {
            worker_.request_stop();
            if (!resume()) // Ensure the worker is not waiting when we request stop
            {
                // If the worker is still running, we can call cancel_job_ to stop it
                cancel_job_();
                wait_for_completion();
            }

            worker_.join();
        }
    }

    peripheral_job::~peripheral_job()
    {
        stop();
    }
}
