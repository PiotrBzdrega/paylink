#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h> // <-- fchmod
#include "paylink/paylink_c_api.h"

char *createConfiguration()
{
    char filename[] = "/tmp/paylinkXXXXXX.toml";

    /*
     * suffixlen = length of ".toml"
     * ".toml" = 5 characters
     */
    int fd = mkstemps(filename, 5);
    if (fd == -1)
    {
        perror("mkstemps");
        return NULL;
    }

    printf("File created: %s\n", filename);

    /*
     * 0644:
     * owner: rw-
     * group: r--
     * other: r--
     */
    if (fchmod(fd, 0644) == -1)
    {
        perror("fchmod");
        close(fd);
        return NULL;
    }

    FILE *fp = fdopen(fd, "w");
    if (!fp)
    {
        perror("fdopen");
        close(fd);
        return NULL;
    }

    fprintf(fp,
            "[logger]\n"
            "# TRACE, DEBUG, INFO, ERROR, NONE\n"
            "level = \"TRACE\"\n"
            "stdout = false\n"
            "# file = \"/tmp/paylink.log\"\n"
            "\n"
            "[module]\n"
            "# DISABLED, STUB, ENABLED\n"
            "pn532 = \"DISABLED\"\n"
            "acceptor = \"STUB\"\n");

    fclose(fp);

    return strdup(filename); // caller must free();
}

void log_callback(const char *msg)
{
    printf("Log: %s\n", msg);
}

int main()
{
    char *path = createConfiguration();
    if (path == NULL)
    {
        printf("Failed to create configuration\n");
        return -1;
    }

    printf("Hello, Paylink!\n");
    if (createPaylinkSystem(path, log_callback) != 0)
    {
        printf("Failed to create Paylink system\n");
        free(path);
        return 1;
    }

    free(path);

    int choice;

    const char *commands[] = {
        "getVersion()",
        "dispenseCoins(100)",
        "dispenseCoins(200)",
        "setMotor(true, 0)",
        "setMotor(false, 0)",
        "setMotor(true, 3000)",
        "setLED(1, true, 0)",
        "setLED(1, false, 0)",
        "setLED(1, true, 1000)",
        "levelOfCoins()",
        "currentCredit()",
        "getSensorsState()",
        "getButtonsState()",
        "Exit"};

    size_t command_size = sizeof(commands) / sizeof(commands[0]);
    int run{1};
    while (run)
    {
        printf("\n=== Menu ===\n");
        for (int i = 0; i < command_size; i++)
        {
            printf("%d - %s\n", i, commands[i]);
        }
        printf("Enter choice: ");

        while (scanf("%d", &choice) == 0)
        {
            printf("Invalid input\n");
            // while (getchar() != '\n'); // Clear the input buffer
        }

        switch (choice)
        {
        case 0:
        {
            printf("%s...\n", commands[choice]);
            const char* ver = getVersion();
            if(ver != NULL)
            {
                printf("Version: %s\n", ver);
            }
            else
            {
                printf("Invalid version:\n");
            }
        }
        break;
        case 1:
        {
            printf("%s...\n", commands[choice]);
            int result = dispenseCoins(100);
            if (result == -1)
            {
                printf("Failed to dispense coins, error code: %d\n", result);
            }
            else
            {
                printf(" %d Coins dispensed successfully!\n", result);
            }
        }
        break;
        case 2:
        {
            printf("%s...\n", commands[choice]);
            int result = dispenseCoins(200);
            if (result == -1)
            {
                printf("Failed to dispense coins, error code: %d\n", result);
            }
            else
            {
                printf(" %d Coins dispensed successfully!\n", result);
            }
        }
        break;
        case 3:
        {
            printf("%s...\n", commands[choice]);
            setMotor(true, 0);
        }
        break;
        case 4:
        {
            printf("%s...\n", commands[choice]);
            setMotor(false, 0);
        }
        break;
        case 5:
        {
            printf("%s...\n", commands[choice]);
            setMotor(true, 3000);
        }
        break;
        case 6:
        {
            printf("%s...\n", commands[choice]);
            setLED(1, true, 0);
        }
        break;
        case 7:
        {
            printf("%s...\n", commands[choice]);
            setLED(1, false, 0);
        }
        break;
        case 8:
        {
            printf("%s...\n", commands[choice]);
            setLED(1, true, 1000);
        }
        break;
        case 9:
        {
            printf("%s...\n", commands[choice]);
            int lvl = levelOfCoins();
            printf("Level of coins %d \n", lvl);
        }
        break;
        case 10:
        {
            printf("%s...\n", commands[choice]);
            int credit = currentCredit();
            printf("Current credit %d \n", credit);
        }
        break;
        case 11:
        {
            printf("%s...\n", commands[choice]);
            const char* sensors = getSensorsState();
            if(sensors != NULL)
            {
                printf("Sensors state: %s\n", sensors);
            }
            else
            {
                printf("Invalid Sensors state:\n");
            }
        }
        case 12:
        {
            printf("%s...\n", commands[choice]);
            uint16_t buttons = getButtonsState();
            printf("Buttons state: %s\n", buttons);
        }
        break;
        case 13:
        {
            printf("%s...\n", commands[choice]);
            run = 0;
        }
        break;
        }
    }

    sleep(2);

    destroyPaylinkSystem();
    return 0;
}