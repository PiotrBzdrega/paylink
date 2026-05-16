#include "stdio.h"
#include <unistd.h>
#include "paylink/paylink_c_api.h"

int main()
{
    printf("Hello, Paylink!\n");
    if(createPaylinkSystem("/home/dev/Sources/paylink-app/example/configuration.toml") != 0) {
        printf("Failed to create Paylink system\n");
        return 1;
    }
    int result = dispenseCoins(100);
    if (result != 0) {
        printf("Failed to dispense coins, error code: %d\n", result);
    } else {
        printf("Coins dispensed successfully!\n");
    }
    sleep(5);

    destroyPaylinkSystem();
    return 0;
}