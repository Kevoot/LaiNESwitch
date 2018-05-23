#pragma once
#include <cerrno>
#include <sys/stat.h>
#include <SDL2/SDL.h>

#define CONFIG_DIR_DEFAULT_MODE      S_IRWXU | S_IRGRP |  S_IXGRP | S_IROTH | S_IXOTH
#define USE_CONFIG_DIR true
#define CONFIG_DIR_NAME "LaiNES" 
#define CONFIG_FALLBACK ".laines-settings"
/* PATH_MAX is a portability nightmare. */
#define CONFIG_PATH_MAX 1024 

#define JOY_A     0
#define JOY_B     1
#define JOY_X     2
#define JOY_Y     3
#define JOY_L     6
#define JOY_R     7
#define JOY_ZL    8
#define JOY_ZR    9
#define JOY_PLUS  10
#define JOY_MINUS 11
#define JOY_LEFT  12
#define JOY_UP    13
#define JOY_RIGHT 14
#define JOY_DOWN  15
#define JOY_LSTICK_LEFT 16
#define JOY_LSTICK_UP 17
#define JOY_LSTICK_RIGHT 18
#define JOY_LSTICK_DOWN 19

namespace GUI {

/* Loading and saving */
void load_settings();
void save_settings();
const char* get_config_path(char * buf, int buflen);

extern int last_window_size;
extern SDL_Scancode KEY_A     [];
extern SDL_Scancode KEY_B     [];
extern SDL_Scancode KEY_SELECT[];
extern SDL_Scancode KEY_START [];
extern SDL_Scancode KEY_UP    [];
extern SDL_Scancode KEY_DOWN  [];
extern SDL_Scancode KEY_LEFT  [];
extern SDL_Scancode KEY_RIGHT [];
extern int BTN_UP     [];
extern int BTN_DUP    [];
extern int BTN_DOWN   [];
extern int BTN_DDOWN  [];
extern int BTN_LEFT   [];
extern int BTN_DLEFT  [];
extern int BTN_RIGHT  [];
extern int BTN_DRIGHT [];
extern int BTN_A      [];
extern int BTN_B      [];
extern int BTN_SELECT [];
extern int BTN_START  [];
extern bool useJoystick[];

}
