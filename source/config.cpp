#include "config.hpp"
#include "gui.hpp"
#include <dirent.h>
#include <unistd.h>

namespace GUI
{

#define CONFIG_LINES 17
#define BUF 128

/* Window settings */
int8_t last_window_size = 1;

/* Controls settings */
SDL_Scancode KEY_A[] = {SDL_SCANCODE_A, SDL_SCANCODE_ESCAPE};
SDL_Scancode KEY_B[] = {SDL_SCANCODE_S, SDL_SCANCODE_ESCAPE};
SDL_Scancode KEY_SELECT[] = {SDL_SCANCODE_SPACE, SDL_SCANCODE_ESCAPE};
SDL_Scancode KEY_START[] = {SDL_SCANCODE_RETURN, SDL_SCANCODE_ESCAPE};
SDL_Scancode KEY_UP[] = {SDL_SCANCODE_UP, SDL_SCANCODE_ESCAPE};
SDL_Scancode KEY_DOWN[] = {SDL_SCANCODE_DOWN, SDL_SCANCODE_ESCAPE};
SDL_Scancode KEY_LEFT[] = {SDL_SCANCODE_LEFT, SDL_SCANCODE_ESCAPE};
SDL_Scancode KEY_RIGHT[] = {SDL_SCANCODE_RIGHT, SDL_SCANCODE_ESCAPE};
int BTN_UP[] = {-1, -1};
int BTN_DOWN[] = {-1, -1};
int BTN_LEFT[] = {-1, -1};
int BTN_RIGHT[] = {-1, -1};
int BTN_A[] = {-1, -1};
int BTN_B[] = {-1, -1};
int BTN_SELECT[] = {-1, -1};
int BTN_START[] = {-1, -1};
bool useJoystick[] = {false, false};

std::string get_path()
{
    char cwd[512];
    std::string str(getcwd(cwd, 512));
    str += std::string(CONFIG_FALLBACK);
    return str;
}

int write_settings(int settings[CONFIG_LINES])
{
    FILE *config = NULL;
    int i = 0;

    config = fopen(get_path().c_str(), "w");

    if (config == NULL)
    {
        return -1;
    }

    std::string tempString("");
    for (int i = 0; i < CONFIG_LINES; i++)
    {
        tempString = std::to_string(settings[i]);
        tempString += "\n";
        fputs(tempString.c_str(), config);
    }

    fclose(config);
    return 0;
}

void get_config_data()
{
    FILE *config = NULL;
    int i = 0;
    int data[CONFIG_LINES];

    SDL_Texture * prompt;
    prompt = gen_text(get_path(), {255, 255, 255});
    render_texture(prompt, TEXT_CENTER, HEIGHT - FONT_SZ * 4);
    SDL_RenderPresent(renderer);
    svcSleepThread(1000000000);

    config = fopen(get_path().c_str(), "r");

    if (config == NULL)
    {
        int newSettings[CONFIG_LINES];
        set_size(1);
        for (int p = 0; p < 2; p++)
        {
            newSettings[0 + (p * 8)] = BTN_UP[p] = 17;
            newSettings[0 + (p * 8)] = BTN_DOWN[p] = 19;
            newSettings[0 + (p * 8)] = BTN_LEFT[p] = 16;
            newSettings[0 + (p * 8)] = BTN_RIGHT[p] = 18;
            newSettings[0 + (p * 8)] = BTN_A[p] = 0;
            newSettings[0 + (p * 8)] = BTN_B[p] = 1;
            newSettings[0 + (p * 8)] = BTN_SELECT[p] = 11;
            newSettings[0 + (p * 8)] = BTN_START[p] = 10;
            useJoystick[p] = true;
        }
        // Screen size
        newSettings[16] = 1;

        // write_settings(newSettings);

        return;
    }

    printf("Done.\n");

    char buf[128];
    while (fgets(buf, 128, config))
    {
        data[i] = std::stoi(std::string(buf));
        i++;
        if (i == CONFIG_LINES)
            break;
    }

    fclose(config);

    printf("Got data: ");
    for(int i = 0; i < CONFIG_LINES; ++i) {
        printf("%d ", data[i]);
    }
    
    printf("got file...");

    /* Screen settings */
    // TODO: Once OpenGL is available, use this instead
    // int screen_size = atoi(ini.GetValue("screen", "size", "1"));
    // if (screen_size < 1 || screen_size > 3)
    //     int screen_size = 1;

    //set_size(screen_size);

    set_size(1);

    useJoystick[0] = true;
    useJoystick[1] = true;

    printf("Setting up joycons...");
    for (int p = 0; p < 2; p++)
    {
        BTN_UP[p] = data[0 + (p * 8)];
        BTN_DOWN[p] = data[1 + (p * 8)];
        BTN_LEFT[p] = data[2 + (p * 8)];
        BTN_RIGHT[p] = data[3 + (p * 8)];
        BTN_A[p] = data[4 + (p * 8)];
        BTN_B[p] = data[5 + (p * 8)];
        BTN_SELECT[p] = data[6 + (p * 8)];
        BTN_START[p] = data[7 + (p * 8)];
    }
    printf("Done\n");
}

/* Load settings */
void load_settings()
{
    get_config_data();

    SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    SDL_JoystickEventState(SDL_ENABLE);
}

/* Save settings */
void save_settings()
{
    SDL_Texture * prompt;
    
    int settings[17];

    /* Control settings */
    for (int p = 0; p < 2; p++)
    {
        settings[0 + (p * 8)] = BTN_UP[p];
        settings[1 + (p * 8)] = BTN_DOWN[p];
        settings[2 + (p * 8)] = BTN_LEFT[p];
        settings[3 + (p * 8)] = BTN_RIGHT[p];
        settings[4 + (p * 8)] = BTN_A[p];
        settings[5 + (p * 8)] = BTN_B[p];
        settings[6 + (p * 8)] = BTN_SELECT[p];
        settings[7 + (p * 8)] = BTN_START[p];
    }
    settings[16] = 1;

    int saveResult = write_settings(settings);
    if (saveResult >= 0)
    {
        prompt = gen_text("Successfully saved config", {255, 255, 255});
        render_texture(prompt, TEXT_CENTER, HEIGHT - FONT_SZ * 4);
        SDL_RenderPresent(renderer);
        svcSleepThread(1000000000);
    }
    else
    {
        prompt = gen_text("Failed to save config!", {255, 255, 255});
        render_texture(prompt, TEXT_CENTER, HEIGHT - FONT_SZ * 4);
        SDL_RenderPresent(renderer);
        svcSleepThread(1000000000);
    }
}

} // namespace GUI
