#include "scr.hpp"
#include <cg/cg_offsets.hpp>

char* Scr_GetString(int string)
{
    return (char*)&gScrMemTreePub->mt_buffer[12 * string + 4];
}
unsigned short SL_GetStringOfSize(const char* _str)
{
    int Unknown = 1;
    int _length = strlen(_str) + 1;
    _asm {
        push	_length
        push	Unknown
        push	_str
        mov	eax, 0x54CEC0
        call	eax
        add	esp, 0xC
    }
}