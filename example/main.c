#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>   // <-- fchmod
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

    int result = dispenseCoins(100);
    if (result != 0)
    {
        printf("Failed to dispense coins, error code: %d\n", result);
    }
    else
    {
        printf("Coins dispensed successfully!\n");
    }
    sleep(5);

    destroyPaylinkSystem();
    return 0;
}