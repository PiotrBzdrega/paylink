#include <string>
#include <unordered_map>
#include <semaphore>
#include <functional>
#include <libudev.h>
#include <stdio.h>
#include "serial.h"
#include "logger.h"
#include "stm.h"
#include "Serial.h"
#include "peripheral_job.h"

namespace uc
{
    namespace
    {
        /* MESSAGES */
        constexpr auto DCD_HIGH{"DCD: 1"};
        constexpr auto DCD_LOW{"DCD: 0"};
        constexpr auto LAST_DROP_SENSOR{"LAST_DROP_SENSOR"};
        constexpr auto MAIN_SENSOR{"MAIN_SENSOR"};
        constexpr auto SET_LED{"Set_Led"};
        constexpr auto EXIT_MSG{"EXIT"};

        constexpr auto STATUS{"ADC_SPIKE_STATUS\r\n"};
        constexpr auto RESET_STATUS{"ADC_RES\r\n"};
        constexpr auto SET_RI{"VCP_RI\r\n"};
        constexpr auto RESET_RI{"VCP_RI_OFF\r\n"};

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

                    if (if_str && serial_ports.contains(if_str))
                    {
                        const char *devnode = udev_device_get_devnode(dev);
                        if (devnode)
                        {
                            serial_ports[if_str] = devnode;
                        }
                    }
                }

                struct udev_device *usb_dev =
                    udev_device_get_parent_with_subsystem_devtype(
                        dev, "usb", "usb_device");
                if (usb_dev)
                {
                    const char *product = udev_device_get_sysattr_value(usb_dev, "product");
                    if (product && serial_ports.contains(product))
                    {
                        const char *devnode = udev_device_get_devnode(dev);
                        if (devnode)
                        {
                            serial_ports[product] = devnode;
                        }
                    }
                }
                udev_device_unref(dev);
            }

            udev_enumerate_unref(enumerate);
            udev_unref(udev);
        }

        std::optional<int> process_status(std::string_view s)
        {
            constexpr std::string_view prefix = "SPIKES_CH: ";

            if (!s.starts_with(prefix))
                return false;

            s.remove_prefix(prefix.size());

            if (s == "NONE")
                return {};

            if (s.empty() || s.back() != ',')
                return false;

            s.remove_suffix(1);

            int x{};
            const auto [ptr, ec] = std::from_chars(s.data(),
                                                   s.data() + s.size(), x);
            if (ec == std::errc{} && ptr == s.data() + s.size() && x >= 1 && x <= 15)
            {
                return x;
            }

            return {};
        }
    }
    // void stm::sync_worker(std::stop_token stop_token, std::string_view channel_name)
    // {
    //     /* Create serial port handler */
    //     // com::serial sync_serial{channel_name};
    //     com::Serial sync_serial(channel_name /* , 115200 */);
    //     /* To not close/open serial port explicitly, only in reconnect() */
    //     bool connected{};
    //     // TODO: make it more robust with std::optional
    //     struct sync_task_t
    //     {
    //         bool present{};
    //         std::pair<std::string, std::optional<std::promise<std::string>>> request{};
    //     } sync_task;

    //     /* Create stop callback to unlock endless read */
    //     std::stop_callback stop_cb(stop_token, [&sync_serial]()
    //                                { sync_serial.interrupt_wait(); });

    //     /* Execute loop until stop requested */
    //     while (!stop_token.stop_requested())
    //     {
    //         if (auto res = sync_serial.reconnect(connected); !res)
    //         {
    //             connected = false;
    //             mik::logger::trace("Failed to connect to serial port in sync_worker worker: {}", res.error());
    //             std::this_thread::sleep_for(std::chrono::seconds(10));
    //             continue;
    //         }
    //         else
    //         {
    //             connected = true;
    //         }

    //         /* Check if request is not available from last run */
    //         if (!sync_task.present)
    //         {
    //             /* Wait for new request */
    //             request_queue.pop(sync_task.request);

    //             /* Make sure it is not request to terminate thread */
    //             if (sync_task.request.first == EXIT_MSG)
    //             {
    //                 connected = false;
    //                 break;
    //             }
    //             sync_task.present = true;
    //         }

    //         /* Send message to serial port  */
    //         auto wr_res = sync_serial.write(sync_task.request.first, 1000);
    //         if (!wr_res)
    //         {
    //             mik::logger::error("Failed to write to serial port in sync worker : {}", wr_res.error());
    //             connected = false;
    //             continue;
    //         }

    //         /* Read response from serial port */
    //         auto rd_res = sync_serial.read([](std::string_view data)
    //                                        {
    //                                            mik::logger::info("Received data: {}", data);
    //                                            return true; // Stop reading after first data received
    //                                        });

    //         if (rd_res)
    //         {
    //             bool signals_changed{};

    //             /* Specific handling for Signals state */
    //             if (sync_task.request.first == GET_SIGNALS_REQ)
    //             {
    //                 /* States change detection */
    //                 if (rd_res.value() != last_states)
    //                 {
    //                     /* Update current states data */
    //                     last_states = rd_res.value();
    //                     signals_changed = true;
    //                 }
    //             }

    //             /* Direct calls contains promise that they are waiting for */
    //             if (sync_task.request.second.has_value())
    //             {
    //                 /* Pass value to the waiting instance */
    //                 sync_task.request.second.value().set_value(rd_res.value().data());
    //             }
    //             /* It is not direct call, but signals states has change */
    //             else if (signals_changed)
    //             {
    //                 if (drop_sensor_state_change_cb_ctx.callback)
    //                 {
    //                     std::string msg{rd_res.value().data(), rd_res.value().size()};
    //                     // TODO: forward real message not stub
    //                     pool.detach_task([this, msg = std::move(msg)]()
    //                                      { drop_sensor_state_change_cb_ctx.callback(msg.data(), drop_sensor_state_change_cb_ctx.user_data); });
    //                 }
    //             }
    //             /* Task completed */
    //             sync_task.present = false;
    //         }
    //         else
    //         {
    //             /* error response */

    //             /* DO NOT SEND EMPTY RESULT IF CONNECTION TO SERIAL PORT DIED -> REPEAT REQUEST */
    //             // TODO: if all tasks must be correct, then i need to validate them before sending
    //             // /* Direct calls contains promise that they are waiting for */
    //             // if (sync_task.request.second.has_value())
    //             // {
    //             //     sync_task.request.second.value().set_value("");
    //             // }
    //             connected = false;
    //             mik::logger::error("Failed to read response from serial port in sync worker : {}", rd_res.error());
    //         }
    //     }
    // }
    // void stm::irq_worker(std::stop_token stop_token, std::string_view channel_name)
    // {
    //     com::Serial irq_serial{channel_name /* "/dev/ttyACM0" */};
    //     bool connected{false};

    //     /* Create stop callback to unlock endless read */
    //     std::stop_callback stop_cb(stop_token, [&irq_serial]()
    //                                { irq_serial.interrupt_wait(); });

    //     /* Execute loop until stop requested */
    //     while (!stop_token.stop_requested())
    //     {
    //         if (auto res = irq_serial.reconnect(connected); !res)
    //         {
    //             connected = false;
    //             mik::logger::trace("Failed to connect to serial port in irq_worker worker: {}", res.error());
    //             std::this_thread::sleep_for(std::chrono::seconds(10));
    //             continue;
    //         }
    //         else
    //         {
    //             connected = true;
    //         }

    //         /* Read response from serial port */
    //         auto rd_res = irq_serial.read([](std::string_view data)
    //                                       {
    //                                           mik::logger::info("Received data: {}", data);
    //                                           return true; // Stop reading after first data received
    //                                       });

    //         if (rd_res)
    //         {
    //             if (rd_res.value() == IRQ_MSG)
    //             {
    //                 // TODO: consider obtain some counter from irq
    //                 /* add request to thread as less critical*/
    //                 request_queue.emplace_back(GET_SIGNALS_REQ, std::nullopt);
    //             }
    //         }
    //         else
    //         {
    //             connected = false;
    //             mik::logger::error("Failed to read response from serial port in irq worker : {}", rd_res.error());
    //         }
    //     }
    // }

    void stm::worker(std::stop_token stop_token, std::string_view channel_name)
    {
        com::Serial virt_port{channel_name};
        bool connected{false};

        struct sync_task_t
        {
            bool present{};
            std::pair<std::string, std::optional<RequestPromise>> request{};
        } sync_task;

        peripheral_job periph_job{[&virt_port, this](std::mutex &mtx,
                                                     std::condition_variable &cv, bool &done)
                                  {
                                      // TODO: what if wait_for_notification crash but sertial still operateable -> reconnect
                                      mik::logger::debug("Peripheral job started, waiting for notification");
                                      auto notification = virt_port.wait_for_notification();
                                      {
                                          mik::logger::debug("Peripheral job finished, notification received, start locking mutex");
                                          std::unique_lock<std::mutex> lock(mtx);
                                          done = true;
                                          cv.notify_one();
                                          mik::logger::debug("Peripheral job finished, notification received, mutex unlocked");
                                      }

                                      if (notification.has_value())
                                      {
                                          auto notify = notification.value();
                                          mik::logger::info("Notification type: {}", static_cast<int>(notify));
                                          if (notify == com::NotificationType::DSR)
                                          {
                                              this->create_request_ff(STATUS);
                                          }
                                          else if (notify == com::NotificationType::DCD)
                                          {
                                              if (auto state = virt_port.get_dcd(); state.has_value())
                                              {
                                                  this->create_request_ff(state.value() ? DCD_HIGH : DCD_LOW);
                                              }
                                          }
                                          else if (notify == com::NotificationType::RI)
                                          {
                                              /* restore low ring indicator state */
                                              this->create_request_ff(RESET_RI);
                                          }
                                      }
                                      else
                                      {
                                          mik::logger::error("Failed to wait for notification in peripheral job: {}", notification.error());
                                      }
                                  },
                                  [&virt_port]()
                                  {
                                      virt_port.unlock_wait_notification(SET_RI);
                                  }};

        /* Create stop callback to unlock endless read */
        std::stop_callback stop_cb(stop_token, [&virt_port]()
                                   { virt_port.interrupt_wait(); });

        /* Execute loop until stop requested */
        while (!stop_token.stop_requested())
        {
            if (auto res = virt_port.reconnect(connected); !res)
            {
                connected = false;
                mik::logger::trace("Failed to connect to serial port in worker: {}", res.error());
                mik::logger::trace("Try to stop peripheral job");
                periph_job.stop();
                std::this_thread::sleep_for(std::chrono::seconds(10));
                continue;
            }
            else if (res.value() == com::ReconnectResult::AlreadyOpen)
            {
                mik::logger::trace("Serial port already open, skipping reconnect");
                if (periph_job.done())
                {
                    mik::logger::trace("Resuming peripheral job after carried out job");
                    periph_job.resume();
                }
            }
            else
            {
                mik::logger::debug("Serial port reconnected successfull");
                connected = true;
                periph_job.start();
                if (periph_job.done())
                {
                    mik::logger::trace("Resuming peripheral job after carried out job");
                    periph_job.resume();
                }
            }

            /* Check if request is not available from last run */
            if (!sync_task.present)
            {
                /* Wait for new request */
                request_queue.pop(sync_task.request);
                mik::logger::debug("New message received in worker: {}", sync_task.request.first);

                /* Make sure it is not request to terminate thread */
                if (sync_task.request.first == EXIT_MSG)
                {
                    connected = false;
                    break;
                }
                sync_task.present = true;
            }

            /* possible requests:
                STATUS (event)
                DCD_HIGH (event)
                DCD_LOW (event)
                MAIN_SENSOR (direct call)
                LAST_DROP_SENSOR (direct call)
            */
            if (sync_task.request.first == STATUS)
            {
                /* Send message to serial port  */
                mik::logger::debug("Sending request to serial port: {}", sync_task.request.first);
                auto wr_res = virt_port.write(sync_task.request.first, 1000);
                if (!wr_res)
                {
                    mik::logger::error("Failed to write to serial port in sync worker : {}", wr_res.error());
                    connected = false;
                    continue;
                }

                // TODO: read only if request has callback or promise, otherwise just send it and forget it
                // or read always, but if no callback or promise, just drop it
                /* Read response from serial port */
                auto rd_res = virt_port.read([](std::string_view data) // TODO: put here status validate
                                             {
                                                 mik::logger::info("Received data: {}", data);
                                                 return true; // Stop reading after first data received
                                             });

                if (rd_res)
                {
                    mik::logger::debug("Response to request {}: {}", sync_task.request.first, rd_res.value().data());
                    auto sensor_id = process_status(rd_res.value().data());
                    mik::logger::debug("Processed status result: {}", sensor_id.has_value() ? std::to_string(sensor_id.value()) : "NONE");
                    // /* Direct calls contain promise that they are waiting for */
                    // if (sync_task.request.second.has_value())
                    // {
                    //     /* Pass value to the waiting instance */
                    //     sync_task.request.second.value().set_value(rd_res.value().data());
                    // }

                    if (sensor_id.has_value())
                    {
                        last_drop_sensor = sensor_id.value();
                    }

                    mik::logger::debug("Execute callback for drop sensor change");
                    if (drop_sensor_state_change_cb_ctx.callback)
                    {
                        pool.detach_task([this, sensor_id]()
                                         { drop_sensor_state_change_cb_ctx.callback(sensor_id.value_or(-1), drop_sensor_state_change_cb_ctx.user_data); });
                    }
                    /* Task completed */
                    sync_task.present = false;

                    mik::logger::debug("Send reset status request to serial port");
                    /* Send message to serial port to reset detected status */
                    auto wr_res = virt_port.write(RESET_STATUS, 1000);
                    if (!wr_res)
                    {
                        mik::logger::error("Failed to write to serial port in sync worker : {}", wr_res.error());
                        connected = false;
                        continue;
                    }
                    mik::logger::debug("Wait for low DSR state to be sure that status reset is done");
                    virt_port.wait_for_low_dsr(500, 2000);
                }
            }
            else if (sync_task.request.first == DCD_HIGH || sync_task.request.first == DCD_LOW)
            {
                auto state = DCD_HIGH ? true : false;
                mik::logger::debug("Response to request {}: {}", sync_task.request.first, state);

                if (main_sensor_state_change_cb_ctx.callback)
                {
                    pool.detach_task([this, state]()
                                     { main_sensor_state_change_cb_ctx.callback(state, main_sensor_state_change_cb_ctx.user_data); });
                }
                /* Task completed */
                sync_task.present = false;
            }
            else if (sync_task.request.first == LAST_DROP_SENSOR)
            {
                mik::logger::debug("Response to request {}: {}", sync_task.request.first, last_drop_sensor);
                /* Direct calls contain promise that they are waiting for */
                if (sync_task.request.second.has_value())
                {
                    /* Pass value to the waiting instance */
                    sync_task.request.second.value().set_value(last_drop_sensor);
                }
                /* Task completed */
                sync_task.present = false;
            }
            else if (sync_task.request.first == MAIN_SENSOR)
            {
                mik::logger::debug("Sending request to serial port to get DCD state");
                auto state = virt_port.get_dcd();
                if (!state.has_value())
                {
                    mik::logger::error("Failed to get DCD state from serial port in sync worker");
                    connected = false;
                    continue;
                }

                /* Direct calls contain promise that they are waiting for */
                if (sync_task.request.second.has_value())
                {
                    /* Pass value to the waiting instance */

                    sync_task.request.second.value().set_value(state.value());
                }
                /* Task completed */
                sync_task.present = false;
            }
            else if (sync_task.request.first.contains(SET_LED) ||
                     sync_task.request.first == RESET_RI)
            {
                mik::logger::debug("Sending fire-and-forget message to serial port: {}", sync_task.request.first);
                auto wr_res = virt_port.write(sync_task.request.first, 1000);
                if (!wr_res)
                {
                    mik::logger::error("Failed to write to serial port in sync worker : {}", wr_res.error());
                    connected = false;
                    continue;
                }
                /* Task completed */
                sync_task.present = false;
                /* Write and omit readback from serial port, no need to wait for response, just send it and forget it */
            }
        }

        // NOTE: peripheral job will be stopped in destructor, so no need to stop it here
    }

    std::string stm::create_request(std::string_view request)
    {
        auto prom = RequestPromise{};
        auto fut = prom.get_future();
        request_queue.emplace_front(request.data(), std::make_optional(std::move(prom)));

        auto result = fut.get();
        if (std::holds_alternative<std::string>(result))
        {
            return std::get<std::string>(result);
        }

        mik::logger::error("Not valid promise type for create_request");
        throw std::runtime_error("Not valid promise type for create_request");
    }
    void stm::create_request_ff(std::string_view request)
    {
        request_queue.emplace_front(request, std::nullopt);
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
        if (worker_thr.joinable())
        {
            worker_thr.request_stop();
            worker_thr.join();
        }

        mik::logger::trace("stm destructor end");
    }

    int stm::get_last_drop_sensor_req()
    {
        auto prom = RequestPromise{};
        auto fut = prom.get_future();
        request_queue.emplace_front(LAST_DROP_SENSOR, std::make_optional(std::move(prom)));

        auto result = fut.get();
        if (std::holds_alternative<int>(result))
        {
            return std::get<int>(result);
        }
        mik::logger::error("Not valid promise type for get_last_drop_sensor_req");
        throw std::runtime_error("Not valid promise type for get_last_drop_sensor_req");
    }
    bool stm::get_main_sensor_req()
    {
        auto prom = RequestPromise{};
        auto fut = prom.get_future();
        request_queue.emplace_front(MAIN_SENSOR, std::make_optional(std::move(prom)));

        auto result = fut.get();
        if (std::holds_alternative<bool>(result))
        {
            return std::get<bool>(result);
        }
        mik::logger::error("Not valid promise type for get_main_sensor_req");
        throw std::runtime_error("Not valid promise type for get_main_sensor_req");
    }
    bool stm::set_sensor_led(uint8_t number, uint16_t red, uint16_t green, uint16_t blue)
    {
        // Validate LED number
        if (number > 15)
            return false;

        // Validate 12-bit RGB values
        constexpr std::uint16_t max_color = 4095;

        if (red > max_color ||
            green > max_color ||
            blue > max_color)
        {
            return false;
        }

        std::string request = std::format("Set_Led={},{},{},{}\r\n", number, red, green, blue);

        create_request_ff(request);

        return true;
    }
    void stm::set_main_sensor_state_change_callback(MainSensorChangeCallbackCtx cb_ctx)
    {
        main_sensor_state_change_cb_ctx = cb_ctx;
    }
    void stm::set_drop_sensor_state_change_callback(DropSensorChangeCallbackCtx cb_ctx)
    {
        drop_sensor_state_change_cb_ctx = cb_ctx;
    }
    int stm::run_communication()
    {
        if (worker_thr.joinable()) // check if previous thread ended
        {
            mik::logger::error("Cannot run stm communication, previous thread still running");
            return -1; // previous thread still running
        }
        else
        {
            std::unordered_map<std::string, std::string> serial_ports{
                {"Pico SYNC", ""},
                {"Pico ASYNC", ""},
                {"STM32 CDC INTERFACE", ""}};

            get_serial_ports(serial_ports);

            if (serial_ports["STM32 CDC INTERFACE"].empty())
            {
                mik::logger::error("Cannot find tty for STM32 CDC INTERFACE");
                return -1;
            }

            worker_thr = std::jthread(std::bind_front(&stm::worker, this), serial_ports["STM32 CDC INTERFACE"]);
            return 0;
        }
    };
}