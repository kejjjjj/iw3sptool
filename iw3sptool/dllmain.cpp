#include "pch.hpp"
#include <cg/cg_init.hpp>
#include <thread>
#include <chrono>
#include <global_macros.hpp>

using namespace std::chrono_literals;

#define USE_CONSOLE 1

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, [[maybe_unused]]LPVOID lpReserved)
{
    std::thread thread;
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);

        thread = std::thread([&hModule]()
            {
                const auto game = GetModuleHandle(MODULE_NAME);

                if (!game) {
                    return 0;
                }

#if USE_CONSOLE
                FILE* _con = 0;

                AllocConsole();
                freopen_s(&_con, "CONOUT$", "w", stdout);

                puts("hello, world!");
#endif

                CG_Init();

                while (!!true) {
                    std::this_thread::sleep_for(1s);
                }

#if USE_CONSOLE
                if(_con)
                    fclose(_con);
#endif

                return 1;

            });
        thread.detach();


        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

