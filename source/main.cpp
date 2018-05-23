#include "gui.hpp"
#include "config.hpp"
#include <switch.h>

int main(int argc, char *argv[])
{
    GUI::load_settings();
    GUI::init();
    GUI::run();

    return 0;
}
