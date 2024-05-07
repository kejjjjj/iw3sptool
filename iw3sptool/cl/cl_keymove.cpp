#include "cl_keymove.hpp"
#include <cg/cg_offsets.hpp>
#include <utils/hook.hpp>
#include <iostream>

usercmd_s* CL_GetUserCmd(const int cmdNumber) {
	return &input->cmds[cmdNumber & 0x3F];
}
usercmd_s* test_cmd = 0;

void CL_FinishMove([[maybe_unused]]usercmd_s* cmd)
{
	__asm mov test_cmd, edi;

	void* ptr_ = hooktable::find<void, usercmd_s*>(HOOK_PREFIX(__func__))->get_jmp();

	__asm
	{
		mov edi, test_cmd;
		call ptr_;
	}
}