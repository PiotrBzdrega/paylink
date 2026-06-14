// #define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <format>
#include <mutex>
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
    ScreenInteractive &screen;
    const int visibleElements;
    std::mutex mtx;

    void updateSelected()
    {
        if (autoScroll)
        {
            selected = container.size() - 1;
        }
    }

public:
    AdaptiveMenu() = delete;
    AdaptiveMenu(ScreenInteractive &screen, int visibleElements) : screen(screen), visibleElements(visibleElements)
    {
        menuOption.on_change = [this]()
        {
            autoScroll = container.empty() || selected == container.size() - 1;
        };
        menu = Menu(&container, &selected, menuOption);
    }

    void addElement(const T &value)
    {
        const std::lock_guard<std::mutex> lock(mtx);
        container.push_back(value);
        updateSelected();
        screen.PostEvent(Event::Custom);
    }

    void clear()
    {
        const std::lock_guard<std::mutex> lock(mtx);
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

    // printf("File created: %s\n", filename);

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
    auto *component = static_cast<AdaptiveMenu<std::string> *>(user_data);
    component->addElement(msg);
}

void banknote_callback(int overall_val, int banknote_val, void *user_data)
{
    auto *component = static_cast<AdaptiveMenu<std::string> *>(user_data);
    component->addElement(std::format("Banknote inserted: overall value = {}, banknote value = {}", overall_val, banknote_val));
}

void card_detected_callback(const char *uid, void *user_data)
{
    auto *component = static_cast<AdaptiveMenu<std::string> *>(user_data);
    component->addElement(std::format("Card detected: card UID = {}", uid));
}

void action(ScreenInteractive &screen, std::vector<std::string> &commmandName, int selectedCommand, AdaptiveMenu<std::string> &log_menu)
{

    if (commmandName[selectedCommand] == "getVersion()")
    {
        const char *ver = getVersion();
        if (ver != NULL)
        {
            log_menu.addElement(std::format("Version: {}", ver));
        }
        else
        {
            log_menu.addElement("Invalid app version:");
        }
    }
    else if (commmandName[selectedCommand] == "dispenseCoins(100)")
    {
        int result = dispenseCoins(100);
        if (result == -1)
        {
            log_menu.addElement(std::format("Failed to dispense coins, error code: {}", result));
        }
        else
        {
            log_menu.addElement(std::format(" {} Coins dispensed successfully!", result));
        }
    }
    else if (commmandName[selectedCommand] == "dispenseCoins(200)")
    {
        int result = dispenseCoins(200);
        if (result == -1)
        {
            log_menu.addElement(std::format("Failed to dispense coins, error code: {}", result));
        }
        else
        {
            log_menu.addElement(std::format(" {} Coins dispensed successfully!", result));
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
        log_menu.addElement(std::format("Level of coins: {}", lvl));
    }
    else if (commmandName[selectedCommand] == "currentCredit()")
    {
        int credit = currentCredit();
        log_menu.addElement(std::format("Current credit: {}", credit));
    }
    else if (commmandName[selectedCommand] == "getSensorsState()")
    {
        const char *sensors = getSensorsState();
        if (sensors != NULL)
        {
            log_menu.addElement(std::format("Sensors state: {}", sensors));
        }
        else
        {
            log_menu.addElement("Invalid Sensors state:");
        }
        freeString(sensors);
    }
    else if (commmandName[selectedCommand] == "getButtonsState()")
    {
        uint16_t buttons = getButtonsState();
        log_menu.addElement(std::format("Buttons state: {}", buttons));
    }
    else if (commmandName[selectedCommand] == "Exit")
    {
        destroyPaylinkSystem();
        destroy menus, to omit posting to screen after exiting
         (since callbacks may still be called after exiting, but we want to avoid posting to screen
        screen.Exit();
    }
}

int main()
{
    auto screen = ScreenInteractive::TerminalOutput();

    auto callbackElements = AdaptiveMenu<std::string>(screen, 3);
    auto log_menu = AdaptiveMenu<std::string>(screen, 10);

    char *path = createConfiguration();
    if (path == NULL)
    {
        log_menu.addElement("[EXAMPLE_APP] Failed to create configuration");
        return -1;
    }

    log_menu.addElement("[EXAMPLE_APP] Hello, Paylink!");
    if (createPaylinkSystem(path, log_callback, static_cast<void *>(&log_menu)))
    {
        log_menu.addElement("[EXAMPLE_APP] Failed to create Paylink system");
        free(path);
        return 1;
    }

    setnewBanknoteCallbackCtx(banknote_callback, static_cast<void *>(&callbackElements));       // Set banknote callback
    setCardDetectedCallbackCtx(card_detected_callback, static_cast<void *>(&callbackElements)); // Set card detection callback
    setButtonsStateChangeCallbackCtx([](const uint16_t state, const uint16_t open_edge, const uint16_t close_edge, void *user_data)
                                     {
                                            auto *component = static_cast<AdaptiveMenu<std::string> *>(user_data);
                                            component->addElement(std::format("Buttons state changed: state = {}, open_edge = {}, close_edge = {}", state, open_edge, close_edge)); },
                                     static_cast<void *>(&callbackElements)); // Set buttons state change callback
    setSensorsStateChangeCallbackCtx([](const char *view, void *user_data)
                                     {
                                              auto *component = static_cast<AdaptiveMenu<std::string> *>(user_data);
                                              component->addElement(std::format("Sensors state changed: {}", view)); },
                                     static_cast<void *>(&callbackElements)); // Set sensors state change callback

    free(path);

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
                                { action(screen, commmandName, selectedCommand, log_menu); }, ButtonOption::Animated(Color::Red));

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


    // printf("\033[0m");       // reset colors
    // printf("\033[?25h");     // show cursor
    // printf("\033[?1049l");   // exit alternate screen (fullscreen mode)
    // printf("\033[2J\033[H"); // clear screen
    // fflush(stdout);
    return 0;
}