#include "pch.hpp"


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
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

                //FILE* _con = 0;

                //AllocConsole();
                //freopen_s(&_con, "CONOUT$", "w", stdout);

                //puts("hello, world!");

                while (!dx->device) {
                    std::this_thread::sleep_for(300ms);
                }

                CG_Init();

                while (!!true) {
                    std::this_thread::sleep_for(1s);
                }

                //fclose(_con);

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

