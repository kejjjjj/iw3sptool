#pragma once
#include <type_traits>
#include <string>
#include <memoryapi.h>

template <typename T>
using is_any_pointer = std::disjunction<
	std::is_integral<std::remove_pointer_t<T>>,
	std::is_pointer<T>
>;

template<typename T>
concept MemoryAddress_t = std::is_integral_v<T> || std::is_pointer_v<T>;


namespace Engine
{
	template<typename Return, MemoryAddress_t Mem, typename ... Args>
	constexpr inline Return call(const Mem address, Args... args)
	{
		return (reinterpret_cast<Return(*)(Args...)>(address))(args...);
	}
}


namespace Engine::Tools
{

	template <typename T, typename = typename std::enable_if<is_any_pointer<T>::value>::type>
	void write_bytes(T&& dst, const std::string& bytes) {
		DWORD oldProtect = {};
		const auto size = bytes.length();
		VirtualProtect(reinterpret_cast<LPVOID>(dst), size, PAGE_EXECUTE_READWRITE, &oldProtect);
		memcpy_s(reinterpret_cast<LPVOID>(dst), size, bytes.c_str(), size);
		VirtualProtect(reinterpret_cast<LPVOID>(dst), size, oldProtect, NULL);
	}

	template <typename T, typename = typename std::enable_if<is_any_pointer<T>::value>::type>
	void nop(T&& dst) {
		write_bytes(std::forward<T&&>(dst), "\x90\x90\x90\x90\x90");
	}
	
}