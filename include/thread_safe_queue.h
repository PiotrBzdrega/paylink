#pragma once
#include <deque>
#include <mutex>
#include <condition_variable>
#include <memory>

namespace com
{
    template<typename T>
    class thread_safe_queue
    {
        /***
         * @brief push_back with normal priority, push_front element that must be received quicker, pop - pops to begin with elements inserted at front
         */
    private:
        mutable std::mutex mtx; /*Since locking a mutex is a mutating operation, the mutex object must be marked mutable to change its state even in const functions*/
        //TODO: limit amount of events, *and rise Notification if buffer overflow
        std::deque<T> que;
        std::condition_variable cv;
    public:
        thread_safe_queue() {};
        ~thread_safe_queue() {};
        void push_back(const T &new_value)
        {
            /* lock function scope */
            std::lock_guard<std::mutex> lck(mtx); 

            /* push new value at the end of que if exclusive access available */
            que.push_back(new_value);

            /* let know other thread that smth new appears*/
            cv.notify_one();
        }
        void push_front(const T &new_value)
        {
            /* lock function scope */
            std::lock_guard<std::mutex> lck(mtx); 

            /* push new value at the end of que if exclusive access available */
            que.push_front(new_value);

            /* let know other thread that smth new appears*/
            cv.notify_one();
        }
        std::shared_ptr<T> pop()
        {   
            /* lock function scope */
            std::unique_lock<std::mutex> lck(mtx);

            /* block till queue has new data */
            cv.wait(lck,[this]{return !que.empty();});

            /* obtain last element (added as first) from queue */
            std::shared_ptr<T> res(std::make_shared<T>(que.front()));

            /* remove last element (added as first) from queue */
            que.pop_front();

            return res;
        }
        template<typename F>
        void pop_many_drop_internal(std::vector<T>& container, F&& filter_out)
        {   
            /* lock function scope */
            std::unique_lock<std::mutex> lck(mtx);

            /* block till queue has new data */
            cv.wait(lck,[this]{return !que.empty();});

            /* Retrieve elements till queue runs out or defined amount of elements pushed */
            while (!que.empty())
            {
                if (std::forward<F>(filter_out)(que.front()))
                {
                    /* remove last element (added as first) from queue */
                    que.pop_front();
                    continue;                    
                }
                /* obtain last element (added as first) from queue */
                container.push_back(std::move(que.front()));

                /* remove last element (added as first) from queue */
                que.pop_front();
            }
        }
        /**
         * @return one position behind last element
         */
        template<std::size_t N, typename F>
        std::pair<bool,std::size_t> pop_many_break_on_internal(std::array<T,N>& arr, std::size_t position, F&& filter_out)
        {   
            bool filtered_out{};
            /* lock function scope */
            std::unique_lock<std::mutex> lck(mtx);

            /* block till queue has new data */
            cv.wait(lck,[this]{return !que.empty();});

            auto new_position{position};
            /* Retrieve elements till queue runs out or array fully filled */
            while (!que.empty() && new_position < arr.size())
            {
                if (std::forward<F>(filter_out)(que.front()))
                {
                    /* Leave element and break */
                    filtered_out = true;
                    break;                   
                }
                
                /* obtain last element (added as first) from queue */
                arr[new_position++]=std::move(que.front());

                /* remove last element (added as first) from queue */
                que.pop_front();
            }
            return {filtered_out,new_position};
        }
        bool pop(T& value)
        {   
            /* lock function scope */
            std::unique_lock<std::mutex> lck(mtx);

            /* block till queue has new data */
            cv.wait(lck,[this]{return !que.empty();});

            /* obtain last element (added as first) from queue */
            value = std::move(que.front());

            /* remove last element (added as first) from queue */
            que.pop_front();

            return true;
        }
        bool try_pop(T& value)
        {   
            /* lock function scope */
            std::lock_guard<std::mutex> lck(mtx);             

            /* check if data available */
            if(que.empty())
            {
                return false;
            }

            /* obtain last element (added as first) from queue */
            value = std::move(que.front());

            /* remove last element (added as first) from queue */
            que.pop_front();

            return true;
        }
        template<typename Rep, typename Period>
        bool try_wait_pop(T& value, std::chrono::duration<Rep, Period> timeout)
        {   
            /* lock function scope */
            std::unique_lock<std::mutex> lck(mtx);

            /* block till queue has new data or timeout */
            if(cv.wait_for(lck,timeout,[this]{return !que.empty();}) == false)
            {
                // timeout happened, no data available
                return false;
            }
            /* obtain last element (added as first) from queue */
            value = std::move(que.front());

            /* remove last element (added as first) from queue */
            que.pop_front();

            return true;
        }
        bool try_wait_until_pop(T& value, std::chrono::system_clock::time_point timeout)
        {   
            /* lock function scope */
            std::unique_lock<std::mutex> lck(mtx);

            /* block till queue has new data or timeout */
            if(cv.wait_until(lck,timeout,[this]{return !que.empty();}) == false)
            {
                // timeout happened, no data available
                return false;
            }
            /* obtain last element (added as first) from queue */
            value = std::move(que.front());

            /* remove last element (added as first) from queue */
            que.pop_front();

            return true;
        }
        bool empty() const
        {
            std::lock_guard<std::mutex> lock(mtx);
            return que.empty();
        }
        void clear()
        {
            /* lock function scope */
            std::lock_guard<std::mutex> lck(mtx); 

            //Erases all elements from the container.
            que.clear();
        }
    };
}