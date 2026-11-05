#include "peripheral_job.h"

namespace uc
{
    void peripheral_job::run(std::stop_token stop_token)
    {
        while (!stop_token.stop_requested())
        {

            {
                // mik::logger::debug("Peripheral job started, locking mutex");
                std::lock_guard<std::mutex> lock(mtx_);
                state_ = JobState::RUNNING;
                cv_.notify_one();
                // mik::logger::debug("Peripheral job started, mutex unlocked");
            }

            job_(mtx_, cv_, state_);

            // TODO: create class so job will do fallback_done() in destructor if not done, so we will not have to call it here, same with running state
            fallback_done();

            {
                std::unique_lock<std::mutex> lock(mtx_);
                cv_.wait(lock, [this]
                         { return state_ == JobState::RESUMED; });
            }
        }
    }

    bool peripheral_job::resume()
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (state_ == JobState::RUNNING)
        {
            return false; // Job is still running
        }

        state_ = JobState::RESUMED;
        cv_.notify_all();
        return true;
    }

    bool peripheral_job::done()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        return state_ == JobState::DONE;
    }

    void peripheral_job::fallback_done()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        if (state_ == JobState::RUNNING)
        {
            state_ = JobState::DONE;
            cv_.notify_all();
        }
    }

    void peripheral_job::wait_for_completion()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this]
                 { return state_ == JobState::DONE; });
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

                while (!resume()) // Wait until the job is done
                {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            }

            worker_.join();
        }
    }

    peripheral_job::~peripheral_job()
    {
        stop();
    }
}
