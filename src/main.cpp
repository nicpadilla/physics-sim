#define SDL_MAIN_HANDLED

#include <physics_sim/application.hpp>

#include <cstdlib>

#if defined(_WIN32)
#include <Windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    return physics_sim::app::run_application(__argc, __argv);
}
#else
int main(int argc, char* argv[])
{
    return physics_sim::app::run_application(argc, argv);
}
#endif
