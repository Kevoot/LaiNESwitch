#include "gui.hpp"
#include "config.hpp"
#include <switch.h>
#include <sys/errno.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    // gfxInitDefault();
    // consoleInit(NULL);
    // Initialise sockets
    // socketInitializeDefault();

    // nxlinkStdio();
    // printf("nxlink started\n");

    // gfxFlushBuffers();
    // gfxSwapBuffers();
    // gfxWaitForVsync();
    // gfxExit();

    GUI::load_settings();
    GUI::init();
    GUI::run();

    return 0;
}
