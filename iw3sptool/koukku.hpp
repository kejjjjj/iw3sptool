#pragma once

#ifndef koukku_
#define koukku_
#include "pch.hpp"

#define INRANGE(x,a,b)    (x >= a && x <= b) 
#define getBits( x )    (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))
#define getByte( x )    (getBits(x[0]) << 4 | getBits(x[1]))

namespace hook //a VERY basic namespace to do the most basic things!
{
#ifdef MINHOOK_LIBRARY

	template <typename T>
	struct hookobj
	{

		hookobj() = default;
		hookobj(const auto org, const LPVOID detour/*, const LPVOID target = (LPVOID)org*/, const bool enabled = true) : pTarget((LPVOID)org), pDetour(detour), orgFnc(reinterpret_cast<T*>((DWORD)org))
		{
			init(reinterpret_cast<T*>(org), detour, enabled);
		}
		~hookobj() = default;
		bool init(auto org, const LPVOID detour/*, const LPVOID target = (LPVOID)org*/, const bool enabled = true)
		{
			std::cout << "calling init\n";
			pTarget = (LPVOID)org;
			pDetour = detour;
			orgFnc = reinterpret_cast<T*>((DWORD)org);
			if (ok = (pTarget && pDetour), !ok) {
				FatalError("either target or detour is nullptr");
				return false;
			}
#pragma warning(suppress : 26812)
			MH_STATUS status = MH_CreateHook(pTarget, pDetour, &(LPVOID&)orgFnc);
			std::cout << "create hook!\n";
			if (ok = (status == MH_OK), !ok) {

				if (status != MH_ERROR_ALREADY_CREATED) {
					FatalError(MH_StatusToString(status));
				}

				return false;
			}
			if (!enabled)
				return true;

			if (status = enable(), status != MH_OK) {
				FatalError(MH_StatusToString(status));


				return false;
			}
			return 1;
		}bool release() {
			//if (ok = (pTarget), !ok) {
			//	return 1;
			//}
#pragma warning(suppress : 26812)
			MH_STATUS status;
			if (status = disable(), status != MH_OK) {
				FatalError(MH_StatusToString(status));
				return false;
			}

			status = MH_RemoveHook(pTarget);
			if (ok = (status == MH_OK), !ok) {
				FatalError(MH_StatusToString(status));
				return false;
			}
			pTarget = pDetour = 0;
			return 1;
		}
		MH_STATUS enable() const noexcept {
			if (!pTarget)
				return MH_ERROR_NOT_INITIALIZED;
			std::cout << "enabling " << pTarget << '\n';
			return MH_EnableHook(pTarget);
		}
		MH_STATUS disable() const noexcept {
			if (!pTarget)
				return MH_ERROR_NOT_INITIALIZED;
			std::cout << "disabling " << pTarget << '\n';
			return MH_DisableHook(pTarget);
		}
		template<typename ... Args>
		T call(const Args... args) const noexcept {
			return orgFnc(args...);
		}

		template<typename Type, typename ... Args> 
		decltype(auto) cast_call(const Args... args) const
		{
			return (reinterpret_cast<Type>(orgFnc))(args...);
		}
		T get_ptr() const noexcept { return orgFnc; }
	private:
		bool ok = false;
		LPVOID pTarget = 0;
		LPVOID pDetour = 0;
		T* orgFnc = 0;
	};
#else
#error "MinHook library doesn't exist"
#endif


	void nop(DWORD address);
	void write_addr(void* addr, const char* bytes, size_t len);
	void write_addr(void* addr, void* bytes, size_t len);

	void write_addr(DWORD addr, const char* bytes, size_t len);
	void memcopy(void* dst, void* src, size_t len);

	void get_bytes(void* addr, size_t len, BYTE* buffer);

	DWORD find_pattern(std::string moduleName, std::string pattern);
	DWORD find_pattern(DWORD start_address, DWORD end_address, std::string pattern);


};

#endif