#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h> // <-- fchmod
#include "paylink/paylink_c_api.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

template <typename T>
class AdaptiveMenu
{
private:
    int selected;
    bool autoScroll{true};
    std::vector<T> container;
    MenuOption menuOption;
    Component menu;
    const int visibleElements;
    void updateSelected()
    {
        if (autoScroll)
        {
            selected = container.size() - 1;
        }
    }

public:
    AdaptiveMenu() = delete;
    AdaptiveMenu(int visibleElements) : visibleElements(visibleElements)
    {
        menuOption.on_change = [this]()
        {
            autoScroll = container.empty() || selected == container.size() - 1;
        };
        menu = Menu(&container, &selected, menuOption);
    }

    void addElement(const T &value)
    {
        container.push_back(value);
        updateSelected();
    }

    void clear()
    {
        container.clear();
        selected = {};
        autoScroll = true;
    }

    Component getComponent()
    {
        return menu;
    }

    Element getElement()
    {
        return (menu->Render() | vscroll_indicator | frame | size(HEIGHT, EQUAL, visibleElements));
    }
};

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

void log_callback(const char *msg, void *user_data)
{
    printf("Log: %s\n", msg);
}

void banknote_callback(int overall_val, int banknote_val, void *user_data)
{
    printf("Banknote inserted: overall value = %d, banknote value = %d\n", overall_val, banknote_val);
}

void card_detected_callback(const char *uid, void *user_data)
{
    printf("Card detected: card UID = %s\n", uid);
}

void action(ScreenInteractive &screen, std::vector<std::string> &commmandName, int selectedCommand)
{

    if (commmandName[selectedCommand] == "getVersion()")
    {
        const char *ver = getVersion();
        if (ver != NULL)
        {
            printf("Version: %s\n", ver);
        }
        else
        {
            printf("Invalid version:\n");
        }
    }
    else if (commmandName[selectedCommand] == "dispenseCoins(100)")
    {
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
    else if (commmandName[selectedCommand] == "dispenseCoins(200)")
    {
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
    else if (commmandName[selectedCommand] == "setMotor(true, 0)")
    {
        setMotor(true, 0);
    }
    else if (commmandName[selectedCommand] == "setMotor(false, 0)")
    {
        setMotor(false, 0);
    }
    else if (commmandName[selectedCommand] == "setMotor(true, 3000)")
    {
        setMotor(true, 3000);
    }
    else if (commmandName[selectedCommand] == "setLED(1, true, 0)")
    {
        setLED(1, true, 0);
    }
    else if (commmandName[selectedCommand] == "setLED(1, false, 0)")
    {
        setLED(1, false, 0);
    }
    else if (commmandName[selectedCommand] == "setLED(1, true, 1000)")
    {
        setLED(1, true, 1000);
    }
    else if (commmandName[selectedCommand] == "levelOfCoins()")
    {
        int lvl = levelOfCoins();
        printf("Level of coins %d \n", lvl);
    }
    else if (commmandName[selectedCommand] == "currentCredit()")
    {
        int credit = currentCredit();
        printf("Current credit %d \n", credit);
    }
    else if (commmandName[selectedCommand] == "getSensorsState()")
    {
        const char *sensors = getSensorsState();
        if (sensors != NULL)
        {
            printf("Sensors state: %s\n", sensors);
        }
        else
        {
            printf("Invalid Sensors state:\n");
        }
    }
    else if (commmandName[selectedCommand] == "getButtonsState()")
    {
        uint16_t buttons = getButtonsState();
        printf("Buttons state: %d\n", buttons);
    }
    else if (commmandName[selectedCommand] == "Exit")
    {
        screen.Exit();
    }
}

int main()
{
    auto callbackElements = AdaptiveMenu<std::string>(3);
    auto log_menu = AdaptiveMenu<std::string>(10);

    char *path = createConfiguration();
    if (path == NULL)
    {
        printf("Failed to create configuration\n");
        return -1;
    }

    printf("Hello, Paylink!\n");
    if (createPaylinkSystem(path, log_callback) != 0) pass here user_datat for log_callback
    {
        printf("Failed to create Paylink system\n");
        free(path);
        return 1;
    }

    setnewBanknoteCallbackCtx(banknote_callback,nullptr); // Set banknote callback
    setCardDetectedCallbackCtx(card_detected_callback,nullptr); // Set card detection callback

    free(path);

    auto screen = ScreenInteractive::TerminalOutput();
    std::vector<std::string> commmandName =
        {
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
    int selectedCommand{};
    auto dropDownCommands = Dropdown(&commmandName, &selectedCommand);

    auto clear_button = Button("Clear Logs", [&]
                               { log_menu.clear(); });
    auto submit_button = Button("Submit Command", [&]
                                { action(screen, commmandName, selectedCommand); }, ButtonOption::Animated(Color::Red));

    // ---- MAIN LAYOUT ----
    auto layout = Container::Vertical({
        dropDownCommands,
        submit_button,
        Container::Horizontal({
            clear_button,
        }),
        // log_renderer,
        callbackElements.getComponent(),
        log_menu.getComponent(),
    });

    auto renderer = Renderer(layout, [&]
                             { return vbox({
                                          text("FTXUI Dashboard") | bold | center,
                                          separator(),
                                          vbox({
                                              dropDownCommands->Render() | frame |
                                                  size(HEIGHT, EQUAL, commmandName.size()),
                                          }),
                                          separator(),
                                          hbox({
                                              submit_button->Render(),
                                              text(" "),
                                              clear_button->Render(),
                                          }),
                                          separator(),
                                          vbox({

                                              hcenter(bold(text("Callbacks log:"))),
                                              separator(),
                                              callbackElements.getElement(),
                                          }),
                                          separator(),
                                          vbox({
                                              hcenter(bold(text("Logs:"))),
                                              separator(),
                                              log_menu.getElement(),
                                          }),

                                      }) |
                                      border; });

    screen.Loop(renderer);

    destroyPaylinkSystem();
    return 0;
}