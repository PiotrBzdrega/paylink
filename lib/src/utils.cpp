#include "utils.h"
#include <thread>
#include "logger.h"
#include <chrono>
#include <cstdlib>
#include <stdlib.h>
#include <string>



// namespace
// {
// // TODO: fix c pointers -> smart pointers

//     int names_init(struct udev **udev_, struct udev_hwdb **hwdb_)
//     {
//         *udev_ = udev_new();
//         if (!*udev_)
//             return -1;

//         *hwdb_ = udev_hwdb_new(*udev_);
//         return hwdb_ ? 0 : -1;
//     }

//     void names_exit(struct udev **udev_, struct udev_hwdb **hwdb_)
//     {
//         *hwdb_ = udev_hwdb_unref(*hwdb_);
//         *udev_ = udev_unref(*udev_);
//     }

//     int read_sysfs_prop(char *buf, size_t size, const char *sysfs_name, const char *propname)
//     {
// #define PATH_MAX 4096 /* # chars in a path name including nul */
// #define SYSFS_DEV_ATTR_PATH "/sys/bus/usb/devices/%s/%s"

//         int n, fd;
//         char path[PATH_MAX];

//         buf[0] = '\0';
//         snprintf(path, sizeof(path), SYSFS_DEV_ATTR_PATH, sysfs_name, propname);
//         fd = open(path, O_RDONLY);

//         if (fd == -1)
//             return 0;

//         n = read(fd, buf, size);

//         if (n > 0)
//             buf[n - 1] = '\0'; // Turn newline into null terminator

//         close(fd);
//         return n;
//     }

// /*
//  * The documentation of libusb_get_port_numbers() says "As per the USB 3.0
//  * specs, the current maximum limit for the depth is 7."
//  */
// #define USB_MAX_DEPTH 7

//     int get_sysfs_name(char *buf, size_t size, libusb_device *dev)
//     {
//         int len = 0;
//         uint8_t bnum = libusb_get_bus_number(dev);
//         uint8_t pnums[USB_MAX_DEPTH];
//         int num_pnums;

//         buf[0] = '\0';

//         num_pnums = libusb_get_port_numbers(dev, pnums, sizeof(pnums));
//         if (num_pnums == LIBUSB_ERROR_OVERFLOW)
//         {
//             return -1;
//         }
//         else if (num_pnums == 0)
//         {
//             /* Special-case root devices */
//             return snprintf(buf, size, "usb%d", bnum);
//         }

//         len += snprintf(buf, size, "%d-", bnum);
//         for (int i = 0; i < num_pnums; i++)
//         {
//             int n = snprintf(buf + len, size - len, i ? ".%d" : "%d", pnums[i]);
//             if ((n < 0) || (n >= (int)(size - len)))
//                 break;
//             len += n;
//         }

//         return len;
//     }

//     static const char *hwdb_get(const char *modalias, const char *key, struct udev_hwdb *hwdb_)
//     {
//         struct udev_list_entry *entry;

//         udev_list_entry_foreach(entry, udev_hwdb_get_properties_list_entry(hwdb_, modalias, 0)) if (strcmp(udev_list_entry_get_name(entry), key) == 0) return udev_list_entry_get_value(entry);

//         return NULL;
//     }

//     const char *names_product(uint16_t vendorid, uint16_t productid, struct udev_hwdb *hwdb_)
//     {
//         char modalias[64];

//         sprintf(modalias, "usb:v%04Xp%04X*", vendorid, productid);
//         return hwdb_get(modalias, "ID_MODEL_FROM_DATABASE", hwdb_);
//     }

//     int get_product_string(char *buf, size_t size, uint16_t vid, uint16_t pid, struct udev_hwdb *hwdb_)
//     {
//         const char *cp;

//         if (size < 1)
//             return 0;
//         *buf = 0;
//         if (!(cp = names_product(vid, pid, hwdb_)))
//             return 0;
//         return snprintf(buf, size, "%s", cp);
//     }

//     const char *names_vendor(uint16_t vendorid, struct udev_hwdb *hwdb_)
//     {
//         char modalias[64];

//         sprintf(modalias, "usb:v%04X*", vendorid);
//         return hwdb_get(modalias, "ID_VENDOR_FROM_DATABASE", hwdb_);
//     }

//     int get_vendor_string(char *buf, size_t size, uint16_t vid, struct udev_hwdb *hwdb_)
//     {
//         const char *cp;

//         if (size < 1)
//             return 0;
//         *buf = 0;
//         if (!(cp = names_vendor(vid, hwdb_)))
//             return 0;
//         return snprintf(buf, size, "%s", cp);
//     }

//     /*
//      * Attempt to get friendly vendor and product names from the udev hwdb. If
//      * either or both are not present, instead populate those from the device's
//      * own string descriptors.
//      */
//     void get_vendor_product_with_fallback(char *vendor, int vendor_len,
//                                           char *product, int product_len,
//                                           libusb_device *dev, struct udev_hwdb *hwdb_)
//     {
//         struct libusb_device_descriptor desc;
//         char sysfs_name[4096];
//         bool have_vendor, have_product;

//         libusb_get_device_descriptor(dev, &desc);

//         /* set to "[unknown]" by default unless something below finds a string */
//         strncpy(vendor, "[unknown]", vendor_len);
//         strncpy(product, "[unknown]", product_len);

//         have_vendor = !!get_vendor_string(vendor, vendor_len, desc.idVendor, hwdb_);
//         have_product = !!get_product_string(product, product_len,
//                                             desc.idVendor, desc.idProduct, hwdb_);

//         if (have_vendor && have_product)
//             return;

//         if (get_sysfs_name(sysfs_name, sizeof(sysfs_name), dev) >= 0)
//         {
//             if (!have_vendor)
//                 read_sysfs_prop(vendor, vendor_len, sysfs_name, "manufacturer");
//             if (!have_product)
//                 read_sysfs_prop(product, product_len, sysfs_name, "product");
//         }
//     }

//     static void sort_device_list(libusb_device **list, ssize_t num_devs)
//     {
//         struct libusb_device *dev, *dev_next;
//         int bnum, bnum_next, dnum, dnum_next;
//         ssize_t i;
//         int sorted;
//         sorted = 0;
//         do
//         {
//             sorted = 1;
//             for (i = 0; i < num_devs - 1; ++i)
//             {
//                 dev = list[i];
//                 dev_next = list[i + 1];
//                 bnum = libusb_get_bus_number(dev);
//                 dnum = libusb_get_device_address(dev);
//                 bnum_next = libusb_get_bus_number(dev_next);
//                 dnum_next = libusb_get_device_address(dev_next);
//                 if ((bnum == bnum_next && dnum > dnum_next) || bnum > bnum_next)
//                 {
//                     list[i] = dev_next;
//                     list[i + 1] = dev;
//                     sorted = 0;
//                 }
//             }
//         } while (!sorted);
//     }

//     std::string list_devices(libusb_context *ctx, struct udev *udev_, struct udev_hwdb *hwdb_, int busnum, int devnum, int vendorid, int productid)
//     {
//         libusb_device **list;
//         struct libusb_device_descriptor desc;
//         char vendor[128], product[128];
//         ssize_t num_devs, i;
//         std::string serial_port{};

//         num_devs = libusb_get_device_list(ctx, &list);
//         if (num_devs < 0)
//             goto error;

//         sort_device_list(list, num_devs);
//         for (i = 0; i < num_devs; ++i)
//         {
//             libusb_device *dev = list[i];
//             uint8_t bnum = libusb_get_bus_number(dev);
//             uint8_t dnum = libusb_get_device_address(dev);

//             if ((busnum != -1 && busnum != bnum) ||
//                 (devnum != -1 && devnum != dnum))
//                 continue;
//             libusb_get_device_descriptor(dev, &desc);
//             if ((vendorid != -1 && vendorid != desc.idVendor) ||
//                 (productid != -1 && productid != desc.idProduct))
//                 continue;

//             get_vendor_product_with_fallback(vendor, sizeof(vendor),
//                                              product, sizeof(product), dev, hwdb_);

//             {
//                 struct udev_enumerate *enumerate = udev_enumerate_new(udev_);
//                 udev_enumerate_add_match_subsystem(enumerate, "tty");
//                 udev_enumerate_scan_devices(enumerate);

//                 struct udev_list_entry *devices =
//                     udev_enumerate_get_list_entry(enumerate);

//                 for (struct udev_list_entry *entry = devices;
//                      entry != nullptr;
//                      entry = udev_list_entry_get_next(entry))
//                 {
//                     const char *path =
//                         udev_list_entry_get_name(entry);

//                     struct udev_device *tty_dev =
//                         udev_device_new_from_syspath(udev_, path);

//                     struct udev_device *usb_parent =
//                         udev_device_get_parent_with_subsystem_devtype(
//                             tty_dev, "usb", "usb_device");

//                     if (usb_parent)
//                     {
//                         const char *busnum =
//                             udev_device_get_sysattr_value(usb_parent, "busnum");

//                         const char *devnum =
//                             udev_device_get_sysattr_value(usb_parent, "devnum");

//                         if (busnum && devnum)
//                         {
//                             if (std::stoi(busnum) == bnum &&
//                                 std::stoi(devnum) == dnum)
//                             {
//                                 serial_port =
//                                     udev_device_get_devnode(tty_dev);

//                                 // printf("%s\n", serial_port.data());
//                             }
//                         }
//                     }

//                     udev_device_unref(tty_dev);
//                 }

//                 udev_enumerate_unref(enumerate);
//             }

//             // printf("Bus %03u Device %03u: ID %04x:%04x %s %s\n",
//             //        bnum, dnum,
//             //        desc.idVendor,
//             //        desc.idProduct,
//             //        vendor, product);
//         }

//         libusb_free_device_list(list, 1);
//     error:
//         return serial_port;
//     }
// }

using namespace std::chrono_literals;

namespace paylink
{
    std::pair<int, std::string_view> utils::LastPayoutStatus()
    {
        auto value = ::LastPayStatus();
        std::string_view description;
        switch (value)
        {
        case PAY_ONGOING:
            description = "PAY_ONGOING: The interface is in the process of paying out";
            break;
        case PAY_FINISHED:
            description = "PAY_FINISHED: The payout process is up to date";
            break;
        case PAY_EMPTY:
            description = "PAY_EMPTY: The dispenser is empty";
            break;
        case PAY_JAMMED:
            description = "PAY_JAMMED: The dispenser is jammed";
            break;
        case PAY_US:
            description = "PAY_US: Dispenser non functional";
            break;
        case PAY_FRAUD:
            description = "PAY_FRAUD: Dispenser shut down due to fraud attempt";
            break;
        case PAY_FAILED_BLOCKED:
            description = "PAY_FAILED_BLOCKED: The dispenser is blocked";
            break;
        case PAY_NO_HOPPER:
            description = "PAY_NO_HOPPER: No Dispenser matches amount to be paid";
            break;
        case PAY_INHIBITED:
            description = "PAY_INHIBITED: The dispenser is inhibited";
            break;
        case PAY_SECURITY_FAIL:
            description = "PAY_SECURITY_FAIL: The internal self-checks failed";
            break;
        case PAY_HOPPER_RESET:
            description = "PAY_HOPPER_RESET: The hopper reset during a payout";
            break;
        case PAY_NOT_EXACT:
            description = "PAY_NOT_EXACT: The hopper cannot payout the exact amount";
            break;
        case PAY_GHOST:
            description = "PAY_GHOST: This hopper does not really exist";
            break;
        case PAY_NO_KEY:
            description = "PAY_NO_KEY: Waiting on a valid key exchange";
            break;
        default:
            description = " Unknown value";
            break;
        }
        return {value, description};
    }
    std::pair<int, std::string_view> utils::USBDriverStatus()
    {
        /*-- Determine Driver Status ----------------------------------------*/
        auto value = ::USBDriverStatus();
        std::string_view description;
        switch (value)
        {
        case NOT_USB:
            description = "NOT_USB: Interface is to a PCI card";
            break;
        case USB_IDLE:
            description = "USB_IDLE: No driver or other program running";
            break;
        case STANDARD_DRIVER:
            description = "STANDARD_DRIVER: The driver program is running normally";
            break;
        case FLASH_LOADER:
            description = "FLASH_LOADER: The flash re-programming tool is using the link";
            break;
        case MANUFACTURING_TEST:
            description = "MANUFACTURING_TEST: The manufacturing test tool is using the link";
            break;
        case DRIVER_RESTART:
            description = "DRIVER_RESTART: The standard driver is in the process of exiting / restarting";
            break;
        case USB_ERROR:
            description = "USB_ERROR: The driver has received an error from the low level driver";
            break;
        default:
            description = "Driver: Unknown Status";
            break;
        }
        return {value, description};
    }
    std::pair<int, std::string_view> utils::PlatformType()
    {
        auto value = ::PlatformType();
        std::string_view description;
        switch (value)
        {
        case GENOA_UNKNOWN:
            description = "GENOA_UNKNOWN: old versions of the firmware";
            break;
        case GENOA_H8:
            description = "GENOA_H8: The orginal 16 I/O Genoa unit";
            break;
        case GENOA_RX_HID:
            description = "GENOA_RX_HID: The new USB HUB Genoa unit, using the HID interface";
            break;
        case GENOA_RX_FTDI:
            description = "GENOA_RX_FTDI: The new 32 I/O + PWM + Battery Genoa unit, using the same FTDI interface as the H8";
            break;
        case GENOA_LITE_2:
            description = "GENOA_LITE_2: The 4 I/O Lite unit";
            break;
        case GENOA_USB_ONLY:
            description = "GENOA_USB_ONLY: A driver running with a donlge to drive USB units only";
            break;
        case GENOA_USB_MERGED:
            description = "GENOA_USB_MERGED: A driver running with a donlge to drive USB units only";
            break;
        default:
            break;
        }
        return {value, description};
    }
    std::pair<int, std::string_view> utils::OpenMHEVersion(int InterfaceVersion)
    {
        int value;
        std::string_view description;

        while (true)
        {
            bool retry{};
            bool exit{};
            value = ::OpenMHEVersion(InterfaceVersion);

            switch (value)
            {
            case SUCCESS:
                description = "SUCCESS: Money Handling Equipment opened with success";
                break;
            case ERROR_INVALID_DATA:
                description = "ERROR_INVALID_DATA: The DLL, application or device areat incompatible revision levels";
                exit = true;
                break;
            case ERROR_BAD_UNIT:
                description = "ERROR_BAD_UNIT: General system error";
                exit = true;
                break;
            case ERROR_NOT_READY:
                description = "ERROR_NOT_READY: Paylink has not yet started";
                retry = true;
                break;
            case ERROR_GEN_FAILURE:
                description = "ERROR_GEN_FAILURE: Driver program not running";
                retry = true;
                break;
            case ERROR_BUSY:
                description = "ERROR_BUSY: The USB link is in use";
                retry = true;
                break;
            case ERROR_DEVICE_NOT_CONNECTED:
                description = "ERROR_DEVICE_NOT_CONNECTED: No Paylink unit is connected";
                retry = true;
                break;
            default:
                break;
            }


            mik::logger::trace("IMHEI open state - [{}] {} \n{}", value, description,
                         exit ? "exit application" : retry ? "retry OpenMHE after 20s"
                                                           : "");
            if (exit)
            {
                // std::exit(EXIT_FAILURE);
            }
            else if (retry)
            {
                std::this_thread::sleep_for(20s);
            }
            else
            {
                /* sucessfully leave */
                break;
            }
        };

        return {value, description};
    }
    std::pair<int, std::string_view> utils::DESStatus()
    {
        int value;
        std::string_view description;
        value = ::DESStatus();

        switch (value)
        {
        case DES_UNLOCKED:
            description = "DES_UNLOCKED: The Paylink is unlocked";
            break;
        case DES_MATCH:
            description = "DES_MATCH: DES Key matched by Paylink and PC";
            break;
        case DES_NOT:
            description = "DES_NOT: Not a DES Paylink";
            break;
        case DES_WRONG:
            description = "DES_WRONG: Paylink wrong key";
            break;
        case DES_CHECKING:
            description = "DES_CHECKING: DES Key checking is still being performed.";
            break;
        case DES_APPLYING:
            description = "DES_APPLYING: DES Lock is being applied";
            break;
        default:
            break;
        }

        return {value, description};
    }
    // std::string utils::getSerialPortPath(int vendor, int product)
    // {
    //     struct udev *udev = nullptr;
    //     struct udev_hwdb *hwdb = nullptr;
    //     libusb_context *ctx;
    //     int err;
    //     int bus = -1, devnum = -1;
    //     /* by default, print names as well as numbers */
    //     if (names_init(&udev, &hwdb) < 0)
    //         fprintf(stderr, "unable to initialize usb spec");

    //     err = libusb_init(&ctx);
    //     if (err)
    //     {
    //         fprintf(stderr, "unable to initialize libusb: %i\n", err);
    //         return {};
    //     }
    //     std::string serial_port = list_devices(ctx, udev, hwdb, bus, devnum, vendor, product);

    //     names_exit(&udev, &hwdb);
    //     libusb_exit(ctx);
    //     return serial_port;
    // }
}