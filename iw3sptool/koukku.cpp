#include "pch.hpp"

//#ifdef _WIN64



void hook::nop(DWORD address)
{
	write_addr(address, "\x90\x90\x90\x90\x90", 5);
}
void hook::write_addr(void* addr, const char* bytes, size_t len)
{
	write_addr(addr, (void*)bytes, len);
}
void hook::write_addr(void* addr, void* bytes, size_t len)
{
	DWORD oldProtect;

	VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(addr, bytes, len);
#pragma warning(suppress: 6387)
	VirtualProtect((LPVOID)addr, len, oldProtect, NULL);
}
void hook::write_addr(DWORD addr, const char* bytes, size_t len)
{
	return write_addr((void*)addr, (void*)bytes, len);
}
void hook::memcopy(void* dst, void* src, size_t len)
{
	return write_addr(dst, src, len);
}
void hook::get_bytes(void* addr, size_t len, BYTE* buffer)
{
	std::stringstream ss;
	std::string string;

	for (uint32_t i = 0; i < len; i++)
	{
		string = std::format("{:#x}", *(BYTE*)((size_t)addr + i));
		string.erase(0, 2);
		std::istringstream str(string);

		uint16_t val{};
		str >> std::hex >> val;
		buffer[i] = (BYTE)val;
	}

}
DWORD hook::find_pattern(std::string moduleName, std::string pattern)
{

	DWORD firstMatch = 0;
	DWORD rangeStart = reinterpret_cast<DWORD>(GetModuleHandleA(moduleName.c_str()));

	if (!rangeStart)
		return 0;

	MODULEINFO miModInfo; K32GetModuleInformation(GetCurrentProcess(), reinterpret_cast<HMODULE>(rangeStart), &miModInfo, sizeof(MODULEINFO));
	DWORD rangeEnd = rangeStart + miModInfo.SizeOfImage;

	return find_pattern(rangeStart, rangeEnd, pattern);


}
DWORD hook::find_pattern(DWORD start_address, DWORD end_address, std::string pattern)
{
	const char* pat = pattern.c_str();
	//std::cout << "using pattern: [" << pat << "]\n";
	DWORD firstMatch = 0;
	MEMORY_BASIC_INFORMATION mbi{};
	for (DWORD pCur = start_address; pCur < end_address; pCur++)
	{
		//if (!VirtualQuery((char*)pCur, &mbi, sizeof(mbi)) || mbi.State != MEM_COMMIT || mbi.Protect == PAGE_NOACCESS) 
		//	continue;
		sizeof(void(*)(int, int, int));
		if (!*pat)
			return firstMatch;

		if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == getByte(pat))
		{
			if (!firstMatch)
				firstMatch = pCur;

			if (!pat[2])
				return firstMatch;

			if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?')
				pat += 3;

			else
				pat += 2;    //one ?
		}
		else
		{
			pat = pattern.c_str();
			firstMatch = 0;
		}
	}
	std::cout << "pattern not found\n";
	return NULL;
}