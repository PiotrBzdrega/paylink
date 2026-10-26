#include <thread>
#include <utility>
#include <string>
#include <mutex>
#include <optional>
#include <future>
#include "BS_thread_pool.hpp"
#include "thread_safe_queue.h"
#include "callback_context.h"

namespace uc
{
    using RequestResult = std::variant<std::string, bool, int>;
    using RequestPromise = std::promise<RequestResult>;
    class stm
    {
    private:
        // TODO: signals state from stm should be stored, and compared during async from stm, execute callback if they are different for now use std::string, can be replace with std::array
        MainSensorChangeCallbackCtx main_sensor_state_change_cb_ctx;
        DropSensorChangeCallbackCtx drop_sensor_state_change_cb_ctx;
        std::string signals;
        // std::jthread irq_thr;
        std::jthread worker_thr;
        BS::thread_pool<> &pool;
        com::thread_safe_queue<std::pair<std::string, std::optional<RequestPromise>>> request_queue;
        int last_drop_sensor;
        // void sync_worker(std::stop_token stop_token, std::string_view channel_name);
        // void irq_worker(std::stop_token stop_token, std::string_view channel_name);
        void worker(std::stop_token stop_token, std::string_view channel_name);
        std::string create_request(std::string_view request);
        /**
         * @brief Create a request and add it to the front of the queue without waiting for a response. This is useful for high-priority requests that do not require a response.
         */
        void create_request_ff(std::string_view request);
        /**
         * @brief Synchronous channel waits for request on blocking queue call,
         * terminating request unlocks blocked status, recipient must handle it accordingly to be effective
         */
        void create_terminating_request();

        /*                 irq thread adds element to thread save queue and respond with status
                        sync thread waits for element, when appears -> calls it and execute callback if state has change
                        how to iject direct calls to stm with thread safe queue.
                        a) insert direct call to queue (in the front [higher priority]) and ask for future that you will be waiting at
                        if i received irq when in queue we have already same type request -> drop it
                        b) drop thread save queue and use just mutex for each call */
    public:
        stm(BS::thread_pool<> &pool_);
        ~stm();
        int get_last_drop_sensor_req();
        bool get_main_sensor_req();
        bool set_sensor_led(uint8_t number, uint16_t red, uint16_t green, uint16_t blue);
        void set_main_sensor_state_change_callback(MainSensorChangeCallbackCtx cb_ctx);
        void set_drop_sensor_state_change_callback(DropSensorChangeCallbackCtx cb_ctx);
        int run_communication();
    };

}