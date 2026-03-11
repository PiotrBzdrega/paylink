#include "scheduler.h"

namespace com
{
    void scheduler::worker(std::stop_token st)
    {
        while (!st.stop_requested())
        {

            // TODO: do not search for a new periodic task if is old still ready, just compare if container size is the same like last
            /*
            if (!task_in_sight)
            {
            }
            */

            // TODO: assure task cancelation, for graceful class shutdown (maybe each periodic task should have cancel() function, or wrap given function to kill it anytime (if possible))
            {
                std::unique_lock<std::mutex> lck(mtx);

                /* Search for closest future task to execute */
                for (const auto &[task_i, task_ref] : std::views::enumerate(tasks))
                {
                    /* take first task in container */
                    if (!task_in_sight.available())
                    {
                        task_in_sight = {static_cast<std::size_t>(task_i), task_ref.get_id()};
                    }
                    else
                    {
                        /* task should be executed earlier than task_in_sight */
                        if (task_ref.get_next_time_call() < tasks[task_in_sight.index.value()].get_next_time_call())
                        {
                            task_in_sight = {static_cast<std::size_t>(task_i), task_ref.get_id()};
                        }
                    }
                }
                /* Store current task size to detect if new appeared in container */
                auto periodic_tasks_size = tasks.size();

                /* Wait until stop token requested, periodic task appeared or timeout that imply need to execute next periodic task */
                cv.wait_until(lck, task_in_sight.available() ? tasks[task_in_sight.index.value()].get_next_time_call() : std::chrono::steady_clock::time_point::max(), [&]
                              { return st.stop_requested() || periodic_tasks_size != tasks.size(); });
            }

            // TODO: detect system time change, how much did it change, correct next time call for all tasks, if it is not possible -> store last finished execution and future call ,
            // wait difference between those two, if current time is not between make adjustments to all periodic tasks interval

            if (st.stop_requested())
            {
                break;
            }

            {
                /* lock function scope */
                std::lock_guard<std::mutex> lck(mtx);

                if (task_in_sight.available() && tasks.size() > 0)
                {
                    if (tasks.size() - 1 >= task_in_sight.index.value() &&
                        task_in_sight.id == tasks[task_in_sight.index.value()].get_id())
                    {
                        auto &t = tasks[task_in_sight.index.value()];
                        auto now = std::chrono::floor<std::chrono::milliseconds>(std::chrono::steady_clock::now());

                        if (t.get_next_time_call() <= now)
                        {
                            // TODO: make sure that we need to have lock during task execution
                            /* Execute task */
                            if (t())
                            {
                                remove_task_(task_in_sight.id);
                            }

                            /* Drop task */
                            task_in_sight.reset();
                        }
                    }
                    else
                    {
                        /* Task available but not in the same place as before, probably removed or relocated */
                        /* Drop task */
                        task_in_sight.reset();
                    }
                }
            }
        }
    }

    bool scheduler::remove_task_(std::size_t id_)
    {
        for (const auto &[task_i, task_ref] : std::views::enumerate(tasks))
        {
            if (task_ref.get_id() == id_)
            {
                // std::println("removed task {}", id_);
                tasks.erase(tasks.begin() + task_i);
                return true;
            }
        }
        return false;
    }

    std::size_t scheduler::submit_periodic_task(task_t &&t_, std::chrono::milliseconds interval_, uint32_t repeat_)
    {
        repeat_t r = repeat_ > 0 ? repeat_t{repeat_ - 1} : std::nullopt;
        auto id = std::hash<std::string_view>{}(std::to_string(counter++));
        {
            // std::println("new task {}", id);
            std::lock_guard<std::mutex> lck(mtx);
            tasks.emplace_back(std::move(t_), interval_, id, r);
        }
        cv.notify_one();
        return id;
    }

    bool scheduler::remove_task(std::size_t id_)
    {
        bool res{};
        {
            std::lock_guard<std::mutex> lck(mtx);
            res = remove_task_(id_);
        }
        // TODO: not sure if i should notify condition variable if tasks container is untouched
        cv.notify_one();
        return res;
    }

    std::size_t scheduler::submit_task(task_t &&t_)
    {
        return submit_periodic_task(std::move(t_), std::chrono::milliseconds(0), 1);
    }

    void scheduler::stop()
    {
        if (thr_worker.joinable())
        {
            thr_worker.request_stop();
            thr_worker.join();
        }
    }

    scheduler::scheduler()
    {
        thr_worker = std::jthread(std::bind_front(&scheduler::worker, this));
    }

    scheduler::~scheduler()
    {
        stop();
    }
}
