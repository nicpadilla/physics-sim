#define SDL_MAIN_HANDLED

#include <physics_sim/application.hpp>
#include <physics_sim/lab_application.hpp>
#include <physics_sim/mode_switch.hpp>
#include <physics_sim/version.hpp>

#include <cstdio>
#include <cstdlib>
#include <string_view>

namespace
{
bool version_requested(int argc, char *argv[])
{
    for (int index = 1; index < argc; ++index)
    {
        const std::string_view argument = argv[index];
        if (argument == "--version" || argument == "-v")
        {
            return true;
        }
    }
    return false;
}

bool lab_mode_requested(int argc, char *argv[])
{
    for (int index = 1; index < argc; ++index)
    {
        const std::string_view argument = argv[index];
        if ((argument == "--mode" && index + 1 < argc && std::string_view{argv[index + 1]} == "lab") || argument == "--mode=lab")
        {
            return true;
        }
    }
    return false;
}

int run_selected_mode(int argc, char *argv[])
{
    if (version_requested(argc, argv))
    {
        std::printf("physics-sim %.*s\n", static_cast<int>(physics_sim::version.size()), physics_sim::version.data());
        return 0;
    }

    bool lab_mode = lab_mode_requested(argc, argv);
    for (;;)
    {
        const int result = lab_mode ? physics_sim::lab::run_lab_application(argc, argv) : physics_sim::app::run_application(argc, argv);
        if (result == physics_sim::switch_to_lab_exit_code)
        {
            lab_mode = true;
            continue;
        }
        if (result == physics_sim::switch_to_sandbox_exit_code)
        {
            lab_mode = false;
            continue;
        }
        return result;
    }
}
} // namespace

#if defined(_WIN32)
#include <Windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    return run_selected_mode(__argc, __argv);
}
#else
int main(int argc, char *argv[])
{
    return run_selected_mode(argc, argv);
}
#endif
