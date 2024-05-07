#include "cmd.hpp"
#include <com/com_channel.hpp>

cmd_function_s* Cmd_FindCommand(const char* name)
{
    static const DWORD fnc = 0x530EF0;
    __asm
    {
        mov esi, name;
        call fnc;
    }
}

cmd_function_s cmds[24];

cmd_function_s* Cmd_AddCommand(const char* cmdname, void(__cdecl* function)())
{
    cmd_function_s* cmd = Cmd_FindCommand(cmdname);

    if (cmd)
        return cmd;

    static int num_cmds = 0;

    Com_Printf(CON_CHANNEL_CONSOLEONLY, "adding a new func command: %s\n", cmdname);

    auto* _cmd = &cmds[num_cmds++];

    __asm {
        push function;
        mov edi, _cmd;
        mov eax, cmdname;
        mov esi, 0x530F40;
        call esi;
        add esp, 4;
    }

    return 0;
}
cmd_function_s* Cmd_RemoveCommand(const char* cmdname)
{
    __asm
    {
        push cmdname;
        mov esi, 0x4F99E0;
        call esi;
        add esp, 0x4;
    }


}
void Cbuf_AddText(const char* text)
{
    __asm {
        mov eax, text;
        push eax;
        mov ecx, 0;
        push ecx;
        mov esi, 0x530320;
        call esi;
        add esp, 0x8;
    }
}