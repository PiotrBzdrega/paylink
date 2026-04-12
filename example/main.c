#include "stdio.h"
#include <unistd.h>
#include "paylink/paylink_c_api.h"

int main()
{
    printf("Hello, Paylink!\n");
    void *a = createPaylinkSystem("/home/dev/Sources/paylink-app/example/configuration.toml");
    int result = dispenseCoins(a, 100);
    if (result != 0) {
        printf("Failed to dispense coins, error code: %d\n", result);
    } else {
        printf("Coins dispensed successfully!\n");
    }
    sleep(5);

    destroyPaylinkSystem(a);
    return 0;
}