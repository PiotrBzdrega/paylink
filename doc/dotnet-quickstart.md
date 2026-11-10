# .NET quick start

`libpaylink` is a native C++ shared library with a C API. A .NET application
can call it on Linux using P/Invoke; no NuGet package is required. The
[`paylink_dotnet_example`](../paylink_dotnet_example) submodule contains a
Blazor example with bindings for more of the API.

## Where to find examples

- **C# / Blazor:** [`PaylinkBinding.cs`](../paylink_dotnet_example/Clients/PaylinkBinding.cs)
  declares the P/Invoke functions and callback delegates.
  [`Paylink.razor`](../paylink_dotnet_example/Components/Pages/Paylink.razor)
  shows initialization, callback registration, and UI controls that call the
  library. [`NativeStringHandle.cs`](../paylink_dotnet_example/Clients/NativeStringHandle.cs)
  demonstrates releasing native strings through `SafeHandle`.
- **Native C++:** [`example/main.cpp`](../example/main.cpp) creates a system,
  registers callbacks, and demonstrates the available operations in a terminal
  UI. It is the closest reference for the C API's behavior.
- **API declarations:** [`paylink_c_api.h`](../lib/include/paylink/paylink_c_api.h)
  lists exported functions, and
  [`typedefs_c_api.h`](../lib/include/paylink/typedefs_c_api.h) defines callback
  types and the `SystemInfo` structure.

The C# project is a practical reference, not a complete or authoritative API
specification. Match P/Invoke signatures to the C headers: for example,
`createPaylinkSystem` returns `int`, and the three button callback values are
`uint16_t` (use C# `ushort`), not `int`. The project currently pins a patch
version of the `.so`; the guide below uses the library SONAME instead.

## 1. Build and install the native library

Install the native build dependencies required by the project, then from the
repository root:

```sh
cmake -S . -B build
cmake --build build
sudo cmake --install build
sudo ldconfig
```

The install step places `libpaylink.so` and its versioned links in the system
library directory (typically `/usr/local/lib`). If .NET cannot find the
library, check that directory is included in the dynamic loader's search path
and run `sudo ldconfig` again.

## 2. Add a configuration file

Create a config file and pass its absolute path to the native initializer. For
example, start from
[`paylink-config.yaml`](../paylink_dotnet_example/paylink-config.yaml). Its
`STUB`/`DISABLED` modules are useful for trying the integration without
enabling hardware modules; configure the modules and hardware for your setup.

## 3. Declare the native functions

Add a class like this to a .NET console or ASP.NET Core project. The declarations
below show initialization, a logger callback, version retrieval, and cleanup.
The library exports C functions using the C calling convention.

```csharp
using System.Runtime.InteropServices;

internal static class Paylink
{
    private const string Library = "libpaylink.so.0";

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate void LoggerCallback(IntPtr message, IntPtr userData);

    [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
    internal static extern int createPaylinkSystem(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string configPath,
        LoggerCallback callback,
        IntPtr userData);

    [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
    internal static extern IntPtr getVersion();

    [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
    internal static extern void freeString(IntPtr value);

    [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
    internal static extern void destroyPaylinkSystem();

    [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
    internal static extern int dispenseCoins(uint amount);
}
```

Use the `.so.0` SONAME rather than pinning the import to a patch version such as
`.so.0.4.0`; this lets compatible library updates keep the same import name.

## 4. Initialize, use, and shut down

Keep the callback rooted for as long as native code may call it. Strings returned
by `getVersion()` are allocated by the library and must be released with
`freeString()`. Always destroy the system when the application is shutting down.

```csharp
using System.Runtime.InteropServices;

internal static class Program
{
    // A static delegate stays rooted for the lifetime of the process.
    private static readonly Paylink.LoggerCallback Log = OnLog;

    private static void OnLog(IntPtr message, IntPtr userData)
    {
        Console.WriteLine(Marshal.PtrToStringUTF8(message));
    }

    private static int Main(string[] args)
    {
        if (args.Length == 0)
        {
            Console.Error.WriteLine("Pass the path to paylink-config.yaml.");
            return 1;
        }

        string configPath = Path.GetFullPath(args[0]);
        int result = Paylink.createPaylinkSystem(configPath, Log, IntPtr.Zero);
        if (result != 0)
        {
            Console.Error.WriteLine("Could not initialize libpaylink.");
            return 1;
        }

        try
        {
            IntPtr version = Paylink.getVersion();
            if (version == IntPtr.Zero)
            {
                Console.Error.WriteLine("libpaylink did not return a version.");
                return 1;
            }

            try
            {
                Console.WriteLine($"libpaylink {Marshal.PtrToStringUTF8(version)}");
            }
            finally
            {
                Paylink.freeString(version);
            }

            // Only call hardware operations when the corresponding device is ready.
            // int dispenseResult = Paylink.dispenseCoins(100);
            return 0;
        }
        finally
        {
            Paylink.destroyPaylinkSystem();
        }
    }
}
```

Run the app with the config path as its first argument:

```sh
dotnet run -- /absolute/path/to/paylink-config.yaml
```

## Available functions

The functions below are declared in `paylink_c_api.h`. Call system operations
after successful initialization and before `destroyPaylinkSystem()`.

| Function | What it does |
|---|---|
| `createPaylinkSystem(config_path, logger, user_data)` | Creates the system using the given config path and optionally registers a logger callback. Returns `0` on success and `-1` on failure. |
| `destroyPaylinkSystem()` | Shuts down and releases the system. |
| `getVersion()` | Returns the library version as a UTF-8 string; release it with `freeString()`. |
| `setLoggerCallbackCtx(callback, user_data)` | Sets or replaces the logger callback after initialization. |
| `setnewBanknoteCallbackCtx(callback, user_data)` | Registers a callback for banknotes accepted. |
| `setCardDetectedCallbackCtx(callback, user_data)` | Registers a callback for detected cards; returns a status code. |
| `setButtonsStateChangeCallbackCtx(callback, user_data)` | Registers a callback for button state and open/close edges. |
| `setMainSensorChangeCallbackCtx(callback, user_data)` | Registers a callback for main sensor state changes. |
| `setDropSensorChangeCallbackCtx(callback, user_data)` | Registers a callback for drop sensor events. |
| `dispenseCoins(amount)` | Requests a payout; returns the operation result (`-1` indicates failure in the example). |
| `getButtonsState()` | Reads the current button state (`uint16_t`). |
| `getMainSensor()` | Reads the current main sensor state (`bool`). |
| `getLastDropSensor()` | Reads the last drop sensor value. |
| `setSensorLed(number, red, green, blue)` | Sets one sensor LED's RGB values (`uint16_t` components). |
| `setAllSensorLed(red, green, blue)` | Sets the RGB value for all sensor LEDs. |
| `setLED(number, on, interval_ms)` | Controls an LED, with an interval in milliseconds. |
| `setMotor(on, ms)` | Controls the motor for the specified duration in milliseconds. |
| `dispensedCoins()` | Reads the dispensed-coins value. |
| `levelOfCoins()` | Reads the coin level as a UTF-8 string; release it with `freeString()`. |
| `currentCredit()` | Reads the current credit value. |
| `systemInfo(info)` | Fills a `SystemInfo` structure. A `0` return means the request ran; check `info.result` for whether device information was obtained. Release its strings with `freeSystemInfo()`. |
| `freeString(value)` | Releases a string allocated by the library, including values from `getVersion()` and `levelOfCoins()`. |
| `freeSystemInfo(info)` | Releases strings in a populated `SystemInfo` structure. |

## Available callbacks

Callbacks are registered with a function pointer and an opaque `user_data`
pointer. Keep each .NET delegate alive until it has been replaced or the system
has been destroyed. Treat callbacks as potentially asynchronous: in a UI
application, dispatch UI updates to its synchronization context as
[`Paylink.razor`](../paylink_dotnet_example/Components/Pages/Paylink.razor)
does with `InvokeAsync`.

| Callback type | Arguments | Event |
|---|---|---|
| `LoggerCallback` | UTF-8 message, `user_data` | Library log message. Can be passed to `createPaylinkSystem` or registered later. |
| `BanknoteCallback` | overall value, banknote value, `user_data` | A banknote was accepted. |
| `CardDetectionCallback` | UTF-8 card UID, `user_data` | A card was detected. |
| `ButtonsChangeCallback` | state, open edge, close edge (`uint16_t` each), `user_data` | Button state or edge changed. |
| `MainSensorChangeCallback` | state (`bool`), `user_data` | Main sensor state changed. |
| `DropSensorChangeCallback` | sensor value (`int`), `user_data` | Drop sensor event occurred. |

`ErrorEventCallback` is typedef'd in `typedefs_c_api.h`, but there is currently
no exported function to register it. Do not confuse the typedef with a usable
registered callback.

Add further `[DllImport]` declarations by matching the C signatures in
[`paylink_c_api.h`](../lib/include/paylink/paylink_c_api.h) and callback
definitions in [`typedefs_c_api.h`](../lib/include/paylink/typedefs_c_api.h).
Use `CallingConvention.Cdecl` consistently. For native `bool` callback
parameters, explicitly specify one-byte marshalling (`[MarshalAs(UnmanagedType.I1)]`)
to match C++ `bool`.
