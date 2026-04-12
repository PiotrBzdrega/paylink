#include <string>
#include <unordered_map>
#include <libudev.h>
#include <stdio.h>
#include "serial.h"
#include "logger.h"
#include "stm.h"
#include "Serial.h"

namespace uc
{
    namespace
    {
        /* MESSAGES */
        constexpr auto GET_SIGNALS_REQ{"GET_SIGNALS"};
        constexpr auto SET_SIGNAL_REQ{"SET_SIGNALS"};
        constexpr auto TEST_REQ{"SET_SIGNALS"};
        constexpr auto IRQ_MSG{"!"};
        constexpr auto EXIT_MSG{"EXIT"};

        /* SERIAL PORTS */
        void get_serial_ports(std::unordered_map<std::string, std::string> &serial_ports)
        {
            struct udev *udev = udev_new();
            struct udev_enumerate *enumerate = udev_enumerate_new(udev);

            udev_enumerate_add_match_subsystem(enumerate, "tty");
            udev_enumerate_scan_devices(enumerate);

            struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
            struct udev_list_entry *entry;

            udev_list_entry_foreach(entry, devices)
            {
                const char *path = udev_list_entry_get_name(entry);
                struct udev_device *dev = udev_device_new_from_syspath(udev, path);

                // Walk up to USB interface
                struct udev_device *usb_if =
                    udev_device_get_parent_with_subsystem_devtype(
                        dev, "usb", "usb_interface");

                if (usb_if)
                {
                    // const char *if_num =
                    //     udev_device_get_sysattr_value(usb_if, "bInterfaceNumber");

                    const char *if_str =
                        udev_device_get_sysattr_value(usb_if, "interface");

                    if (serial_ports.contains(if_str))
                    {
                        const char *devnode = udev_device_get_devnode(dev);
                        if (!devnode)
                        {
                            continue;
                        }
                        else
                        {
                            serial_ports[if_str] = devnode;
                        }
                    }
                }
                udev_device_unref(dev);
            }

            udev_enumerate_unref(enumerate);
            udev_unref(udev);
        }
    }
    void stm::sync_worker(std::stop_token stop_token, std::string_view channel_name)
    {
        /* Create serial port handler */
        // com::serial sync_serial{channel_name};
        com::Serial sync_serial(channel_name /* , 115200 */);
        /* To not close/open serial port explicitly, only in reconnect() */
        bool connected{};
        // TODO: make it more robust with std::optional
        struct sync_task_t
        {
            bool present{};
            std::pair<std::string, std::optional<std::promise<std::string>>> request{};
        } sync_task;

        /* Create stop callback to unlock endless read */
        std::stop_callback stop_cb(stop_token, [&sync_serial]()
                                   { sync_serial.interrupt_wait(); });

        /* Execute loop until stop requested */
        while (!stop_token.stop_requested())
        {
            if (auto res = sync_serial.reconnect(connected); !res)
            {
                connected = false;
                mik::logger::trace("Failed to connect to serial port in sync_worker worker: {}", res.error());
                std::this_thread::sleep_for(std::chrono::seconds(10));
                continue;
            }
            else
            {
                connected = true;
            }

            /* Check if request is not available from last run */
            if (!sync_task.present)
            {
                /* Wait for new request */
                request_queue.pop(sync_task.request);

                /* Make sure it is not request to terminate thread */
                if (sync_task.request.first == EXIT_MSG)
                {
                    connected = false;
                    continue;
                }
                sync_task.present = true;
            }

            /* Send message to serial port  */
            auto wr_res = sync_serial.write(sync_task.request.first, 1000);
            if (!wr_res)
            {
                mik::logger::error("Failed to write to serial port in sync worker : {}", wr_res.error());
                connected = false;
                continue;
            }

            /* Read response from serial port */
            auto rd_res = sync_serial.read([](std::string_view data)
                                           {
                                               mik::logger::info("Received data: {}", data);
                                               return true; // Stop reading after first data received
                                           });

            if (rd_res)
            {
                bool signals_changed{};

                /* Specific handling for Signals state */
                if (sync_task.request.first == GET_SIGNALS_REQ)
                {
                    /* States change detection */
                    if (rd_res.value() != last_states)
                    {
                        /* Update current states data */
                        last_states = rd_res.value();
                        signals_changed = true;
                    }
                }

                /* Direct calls contains promise that they are waiting for */
                if (sync_task.request.second.has_value())
                {
                    /* Pass value to the waiting instance */
                    sync_task.request.second.value().set_value(rd_res.value().data());
                }
                /* It is not direct call, but signals states has change */
                else if (signals_changed)
                {
                    if (signal_change_callback)
                    {
                        std::string msg{rd_res.value().data(), rd_res.value().size()};
                        // TODO: forward real message not stub
                        pool.detach_task([this, msg = std::move(msg)]()
                                         { signal_change_callback(msg.data()); });
                    }
                }
                /* Task completed */
                sync_task.present = false;
            }
            else
            {
                /* error response */

                /* DO NOT SEND EMPTY RESULT IF CONNECTION TO SERIAL PORT DIED -> REPEAT REQUEST */
                // TODO: if all tasks must be correct, then i need to validate them before sending
                // /* Direct calls contains promise that they are waiting for */
                // if (sync_task.request.second.has_value())
                // {
                //     sync_task.request.second.value().set_value("");
                // }
                connected = false;
                mik::logger::error("Failed to read response from serial port in sync worker : {}", rd_res.error());
            }
        }
    }
    void stm::irq_worker(std::stop_token stop_token, std::string_view channel_name)
    {
        com::Serial irq_serial{channel_name /* "/dev/ttyACM0" */};
        bool connected{false};

        /* Create stop callback to unlock endless read */
        std::stop_callback stop_cb(stop_token, [&irq_serial]()
                                   { irq_serial.interrupt_wait(); });

        /* Execute loop until stop requested */
        while (!stop_token.stop_requested())
        {
            if (auto res = irq_serial.reconnect(connected); !res)
            {
                connected = false;
                mik::logger::trace("Failed to connect to serial port in irq_worker worker: {}", res.error());
                std::this_thread::sleep_for(std::chrono::seconds(10));
                continue;
            }
            else
            {
                connected = true;
            }

            /* Read response from serial port */
            auto rd_res = irq_serial.read([](std::string_view data)
                                          {
                                              mik::logger::info("Received data: {}", data);
                                              return true; // Stop reading after first data received
                                          });

            if (rd_res)
            {
                if (rd_res.value() == IRQ_MSG)
                {
                    // TODO: consider obtain some counter from irq
                    /* add request to thread as less critical*/
                    request_queue.emplace_back(GET_SIGNALS_REQ, std::nullopt);
                }
            }
            else
            {
                connected = false;
                mik::logger::error("Failed to read response from serial port in irq worker : {}", rd_res.error());
            }
        }
    }

    std::string stm::create_request(std::string_view request)
    {
        auto prom = std::promise<std::string>{};
        auto fut = prom.get_future();
        request_queue.emplace_front(request.data(), std::make_optional(std::move(prom)));

        return fut.get();
    }
    void stm::create_terminating_request()
    {
        request_queue.emplace_front(EXIT_MSG, std::nullopt);
    }
    stm::stm(BS::thread_pool<> &pool_) : pool{pool_}
    {
    }
    stm::~stm()
    {
        mik::logger::trace("stm destructor start");
        create_terminating_request();
        if (sync_thr.joinable())
        {
            sync_thr.request_stop();
            sync_thr.join();
        }

        if (irq_thr.joinable())
        {
            irq_thr.request_stop();
            irq_thr.join();
        }
        mik::logger::trace("stm destructor end");
    }
    std::string stm::set_signal_req()
    {
        return create_request(SET_SIGNAL_REQ);
    }
    std::string stm::get_signals_req()
    {
        return create_request(GET_SIGNALS_REQ);
    }
    std::string stm::test_req()
    {
        return std::string(TEST_REQ);
    }
    void stm::set_sensors_state_change_callback(SignalChangeCallback func)
    {
        signal_change_callback = func;
    }
    int stm::run_communication()
    {
        if (sync_thr.joinable() || irq_thr.joinable()) // check if previous thread ended
        {
            mik::logger::error("Cannot run stm communication, previous thread still running");
            return -1; // previous thread still running
        }
        else
        {
            std::unordered_map<std::string, std::string> serial_ports{
                {"Pico SYNC", ""},
                {"Pico ASYNC", ""}};

            get_serial_ports(serial_ports);

            if (serial_ports["Pico SYNC"].empty() || serial_ports["Pico ASYNC"].empty())
            {
                mik::logger::error("Cannot find tty for Pico SYNC or Pico ASYNC");
                return -1;
            }

            sync_thr = std::jthread(std::bind_front(&stm::sync_worker, this), serial_ports["Pico SYNC"]);
            irq_thr = std::jthread(std::bind_front(&stm::irq_worker, this), serial_ports["Pico ASYNC"]);
            return 0;
        }
    };
}