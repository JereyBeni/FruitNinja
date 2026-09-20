// PS3 entry point (PSL1GHT).
// Scaffolding only -- real RSX / pad / audio / fixed-step loop still TODO.
// Pattern follows mainWii.cpp / mainSDL.cpp: init platform, then run the
// shared game fixed-step driver.

#include <stdio.h>
#include <unistd.h>

// PSL1GHT headers will go here once we start real init:
// #include <sys/process.h>
// #include <sysutil/sysutil.h>
// #include <io/pad.h>
// #include <rsx/rsx.h>
// etc.

// Game / engine entry points (shared across platforms).
// These will be wired once the rest of the PS3 backend exists.
// extern int GameMain(...);

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    printf("Fruit Ninja PS3 -- scaffolding build\n");
    printf("Real init (RSX, pad, audio, fixed-step loop) still TODO.\n");

    // TODO:
    // 1. sysModuleLoad / RSX init / display setup
    // 2. pad init
    // 3. filesystem (cellFs or hostfs for RPCS3)
    // 4. sound
    // 5. FixedStepDriver + game loop (same 60 Hz tick as other platforms)
    // 6. clean shutdown on quit / XMB exit

    // Keep the process alive briefly so RPCS3 / hardware shows something.
    sleep(3);

    return 0;
}
