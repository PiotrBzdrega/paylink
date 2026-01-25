* Are function calls thread-safe ? If yes, then i can simultaneusly run many threads to handle readAcceptor, readDispenser ... , otherwise I must run some functionality to add to queue some elements with time 
* Should i store some persistent data on Eprom ? Or on fs ? 
* LinuxDemo creates many callbacks using g_timeout_add
* PayOut should be blocking, or response should be returned via callback
* `CurrentUpdates` returns new value almost every second, it is not good event to detect changes in system 
```cpp
int new_update{};
int old_update{};
for (;;)
{
    std::this_thread::sleep_for(1s);
    new_update = CurrentUpdates();
    if (old_update != new_update)
    {
        std::println("new update {}", new_update);
        old_update=new_update;
    }
    
}
```
