#include "pch.hpp"

cmd_function_s* Cmd_FindCommand(const char* name)
{
    static const DWORD fnc = 0x530EF0;
    __asm
    {
        mov esi, name;
        call fnc;
    }
}
cmd_function_s* Cmd_AddCommand(const char* cmdname, void(__cdecl* function)())
{
    cmd_function_s* cmd = Cmd_FindCommand(cmdname);

    if (cmd)
        return cmd;

    Com_Printf(CON_CHANNEL_CONSOLEONLY, "adding a new func command: %s\n", cmdname);
    std::cout << "adding new func command: " << cmdname << '\n';

    static cmd_function_s _cmd{};

    __asm {
        push function;
        mov edi, offset _cmd;
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

unsigned short SL_GetStringOfSize(const char* str)
{
    int Unknown = 1;
    int _length = strlen(str) + 1;
    _asm {
        push	_length
        push	Unknown
        push	str
        mov	eax, 0x54CEC0
        call	eax
        add	esp, 0xC
    }
}