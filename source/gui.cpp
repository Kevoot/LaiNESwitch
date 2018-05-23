#include <csignal>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>
#include "Sound_Queue.h"
#include "apu.hpp"
#include "cartridge.hpp"
#include "cpu.hpp"
#include "menu.hpp"
#include "gui.hpp"
#include "config.hpp"
#include <dirent.h>
#include <switch.h>

namespace GUI {

// Screen size:
const unsigned WIDTH  = 256;
// const unsigned WIDTH = 128;
const unsigned HEIGHT = 240;
// const unsigned HEIGHT = 120;

// SDL structures:
SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* gameTexture;
SDL_Texture* background;
TTF_Font* font;
u8 const* keys;
Sound_Queue* soundQueue;
SDL_Joystick* joystick[] = { nullptr, nullptr };

// Menus:
Menu* menu;
Menu* mainMenu;
Menu* settingsMenu;
Menu* videoMenu;
Menu* keyboardMenu[2];
Menu* joystickMenu[2];
FileMenu* fileMenu;

bool pause = true;
int currentRenderQuality = 0;
bool exitFlag = false;

/*void set_render_quality(int quality) {
    switch(quality) {
        case 0:
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
            currentRenderQuality = 0;
            break;
        case 1:
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
            currentRenderQuality = 1;
            break;
        case 2:
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
            currentRenderQuality = 2;
            break;
        default:
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
            currentRenderQuality = 1;
            break;
    }
}*/

/* Set the window size multiplier */
void set_size(int mul)
{
    last_window_size = mul;
    SDL_SetWindowSize(window, WIDTH * mul, HEIGHT * mul);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

void updateVideoMenu() {
    /*std::string quality("Render Quality: ");

    switch(currentRenderQuality) {
        case 0: 
            quality += "Fastest";
            break;
        case 1:
            quality += "Medium";
            break;
        case 2:
            quality += "Nicest";
            break;
        default:
            quality += "Fastest";
            break;
    }*/

    videoMenu = new Menu;
    videoMenu->add(new Entry("<",       []{ menu = settingsMenu; }));
    videoMenu->add(new Entry("Size 1x", []{ set_size(1); }));
    videoMenu->add(new Entry("Size 2x", []{ set_size(2); }));
    videoMenu->add(new Entry("Size 3x", []{ set_size(3); }));
    /*videoMenu->add(new Entry((quality), []{ set_render_quality((currentRenderQuality + 1) % 3);
                                             updateVideoMenu();
                                             menu = videoMenu;
                                             }));
    */
}

/* Initialize GUI */
void init()
{
    // Initialize graphics system:
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {
        printf("Failed to init video, audio, and joystick\n");
        return;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

    if(TTF_Init() < 0) {
        printf("Failed to init TTF\n");
        return;
    }

    // for (int i = 0; i < SDL_NumJoysticks(); i++)
    printf("Attempting to open joystick 0...");
    joystick[0] = SDL_JoystickOpen(0);
    // joystick[1] = SDL_JoystickOpen(1);
    if(joystick[0] == nullptr) {
        printf("Failure!\n");
        return;
    }
    printf("Done.\n");

    printf("Initializing APU...");
    APU::init();
    printf("Done.\n");

    soundQueue = new Sound_Queue;
    soundQueue->init(96000);

    // Initialize graphics structures:
    window = SDL_CreateWindow(NULL, 
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                              WIDTH * last_window_size, HEIGHT * last_window_size, SDL_WINDOW_FULLSCREEN);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    SDL_RenderSetLogicalSize(renderer, WIDTH, HEIGHT);

    gameTexture = SDL_CreateTexture(renderer,
                                    SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                    WIDTH, HEIGHT);

    font = TTF_OpenFont("res/font.ttf", FONT_SZ);

    // keys = SDL_GetKeyboardState(0);

    // Initial background:
    if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        exit(1);
    }

    SDL_Surface* backSurface  = IMG_Load("res/init.png");

    background = SDL_CreateTextureFromSurface(renderer, backSurface);

    SDL_SetTextureColorMod(background, 60, 60, 60);
    SDL_FreeSurface(backSurface);

    // Menus:
    mainMenu = new Menu;
    mainMenu->add(new Entry("Load ROM", []{ menu = fileMenu; }));
    mainMenu->add(new Entry("Settings", []{ menu = settingsMenu; }));
    mainMenu->add(new Entry("Exit",     []{ exitFlag = true; }));

    settingsMenu = new Menu;
    settingsMenu->add(new Entry("<",            []{ menu = mainMenu; }));
    // TODO: Add this back and enable substituting the render quality during runtime
    // settingsMenu->add(new Entry("Video",        []{ menu = videoMenu; }));
    // settingsMenu->add(new Entry("Controller 1", []{ menu = joystickMenu[0]; }));
    
    // updateVideoMenu();

    // TODO: Add multiplayer support
    // settingsMenu->add(new Entry("Controller 2", []{ menu = joystickMenu[1]; }));
    settingsMenu->add(new Entry("Save Settings", []{ save_settings(); menu = mainMenu; }));

    // for (int i = 0; i < 2; i++)
    // {
        // if (joystick[i] != nullptr)
        // {
            int i = 0;
            joystickMenu[i] = new Menu;
            joystickMenu[i]->add(new Entry("<", []{ menu = settingsMenu; }));
            joystickMenu[i]->add(new ControlEntry("Up",     &BTN_UP[i]));
            joystickMenu[i]->add(new ControlEntry("Down",   &BTN_DOWN[i]));
            joystickMenu[i]->add(new ControlEntry("Left",   &BTN_LEFT[i]));
            joystickMenu[i]->add(new ControlEntry("Right",  &BTN_RIGHT[i]));
            joystickMenu[i]->add(new ControlEntry("A",      &BTN_A[i]));
            joystickMenu[i]->add(new ControlEntry("B",      &BTN_B[i]));
            joystickMenu[i]->add(new ControlEntry("Start",  &BTN_START[i]));
            joystickMenu[i]->add(new ControlEntry("Select", &BTN_SELECT[i]));
        // }
    // }

    fileMenu = new FileMenu;

    menu = mainMenu;
}

//* Render a texture on screen */
void render_texture(SDL_Texture* texture, int x, int y)
{
    int w, h;
    SDL_Rect dest;

    SDL_QueryTexture(texture, NULL, NULL, &dest.w, &dest.h);
    if (x == TEXT_CENTER)
        dest.x = WIDTH/2 - dest.w/2;
    else if (x == TEXT_RIGHT)
        dest.x = WIDTH - dest.w - 10;
    else
        dest.x = x + 10;
    dest.y = y + 5;

    SDL_RenderCopy(renderer, texture, NULL, &dest);
}

/* Generate a texture from text */
SDL_Texture* gen_text(std::string text, SDL_Color color)
{
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_FreeSurface(surface);
    return texture;
}

/* Get the joypad state from SDL */
u8 get_joypad_state(int n)
{
    if(n == 1) return 0;

    const int DEAD_ZONE = 8000;

    u8 j = 0;
    j |= (SDL_JoystickGetButton(joystick[n], BTN_A[n]))          << 0;  // A.
    j |= (SDL_JoystickGetButton(joystick[n], BTN_B[n]))          << 1;  // B.
    j |= (SDL_JoystickGetButton(joystick[n], BTN_SELECT[n]))     << 2;  // Select.
    j |= (SDL_JoystickGetButton(joystick[n], BTN_START[n]))      << 3;  // Start.

    j |= (SDL_JoystickGetButton(joystick[n], BTN_DUP[n]))        << 4; // Up dpad
    j |= (SDL_JoystickGetButton(joystick[n], BTN_UP[n]))         << 4; // Up joy
    j |= (SDL_JoystickGetAxis(joystick[n], 1) < -DEAD_ZONE)      << 4;

    j |= (SDL_JoystickGetButton(joystick[n], BTN_DDOWN[n]))      << 5; // Down dpad
    j |= (SDL_JoystickGetButton(joystick[n], BTN_DOWN[n]))       << 5; // Down joy
    j |= (SDL_JoystickGetAxis(joystick[n], 1) > DEAD_ZONE)       << 5; 


    j |= (SDL_JoystickGetButton(joystick[n], BTN_DLEFT[n]))      << 6; // Left dpad
    j |= (SDL_JoystickGetButton(joystick[n], BTN_LEFT[n]))       << 6; // Left joy
    j |= (SDL_JoystickGetAxis(joystick[n], 0) < -DEAD_ZONE)      << 6;

    j |= (SDL_JoystickGetButton(joystick[n], BTN_DRIGHT[n]))     << 7; // Right dpad
    j |= (SDL_JoystickGetButton(joystick[n], BTN_RIGHT[n]))      << 7; // Right joy
    j |= (SDL_JoystickGetAxis(joystick[n], 0) > DEAD_ZONE)       << 7;
    return j;
}

/* Send the rendered frame to the GUI */
void new_frame(u32* pixels)
{
    SDL_UpdateTexture(gameTexture, NULL, pixels, WIDTH * sizeof(u32));
}

void new_samples(const blip_sample_t* samples, size_t count)
{
    soundQueue->write(samples, count);
}

/* Render the screen */
void render()
{
    SDL_RenderClear(renderer);

    // Draw the NES screen:
    if (Cartridge::loaded())
        SDL_RenderCopy(renderer, gameTexture, NULL, NULL);
    else
        SDL_RenderCopy(renderer, background, NULL, NULL);

    // Draw the menu:
    if (pause) menu->render();

    SDL_RenderPresent(renderer);
}

/* Play/stop the game */
void toggle_pause()
{
    printf("Toggling pause\n");
    pause = !pause;
    menu = mainMenu;

    if (pause) {
        printf("Paused, setting gameTexture\n");
        SDL_SetTextureColorMod(gameTexture,  60,  60,  60);
    }
    else {
        printf("Not paused, setting gameTexture\n");
        SDL_SetTextureColorMod(gameTexture, 255, 255, 255);
    }
}

/* Prompt for a key, return the scancode */
SDL_Scancode query_key()
{
    SDL_Texture* prompt = gen_text("Press a key...", { 255, 255, 255 });
    render_texture(prompt, TEXT_CENTER, HEIGHT - FONT_SZ*4);
    SDL_RenderPresent(renderer);

    SDL_Event e;
    while (true)
    {
        SDL_PollEvent(&e);
        if (e.type == SDL_KEYDOWN)
            return e.key.keysym.scancode;
    }
}

int query_button()
{
    SDL_Texture* prompt = gen_text("Press a button...", { 255, 255, 255 });
    render_texture(prompt, TEXT_CENTER, HEIGHT - FONT_SZ*4);
    SDL_RenderPresent(renderer);

    SDL_Event e;
    while (true)
    {
        SDL_PollEvent(&e);
        if (e.type == SDL_JOYBUTTONDOWN) {
            printf("Button pressed: %d", e.jbutton.button);
            return e.jbutton.button;
        }
    }
}

/* Run the emulator */
void run()
{
    SDL_Event e;
    // Framerate control:
    u32 frameStart, frameTime;
    const int FPS   = 60;
    const int DELAY = 20.0f / FPS;

    while (true)
    {
        frameStart = SDL_GetTicks();

        // Handle events:
        while (SDL_PollEvent(&e)) {
            switch (e.type)
            {
                case SDL_QUIT: return;
                case SDL_JOYBUTTONDOWN:
                    if ((e.jbutton.button == JOY_R) and Cartridge::loaded())
                        toggle_pause();
                    else if (pause)
                        menu->update(e.jbutton.button);
            }
        }

        if (not pause) { 
            CPU::run_frame();
        }
        render();

        if(exitFlag) {
            SDL_Quit();
            return;
        }

        // Wait to mantain framerate:
        frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < DELAY)
            SDL_Delay((int)(DELAY - frameTime));
    }
}


}
