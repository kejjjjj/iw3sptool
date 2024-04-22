#pragma once

#include "pch.hpp"

namespace _fs = std::filesystem;


namespace fs
{
	enum class fileopen : int
	{
		FILE_IN = 1,
		FILE_OUT = 2,
		FILE_APP = 8,
		FILE_BINARY = 32
	};

	inline struct file_s
	{
		DWORD lines_read;
		DWORD current_column;
		char current_character;

	}file;

	std::string exe_file_name();
	std::string exe_path();
	std::string root_path();
	std::string get_extension(const std::string& path);
	std::string previous_directory(std::string& directory);
	std::string get_file_name(const std::string& fullpath);
	std::string get_file_name_no_extension(const std::string& fullpath);

	bool io_open(std::ifstream& fp, const std::string& path, const fileopen type);
	void io_close(std::ifstream& f);

	void create_file(const std::string& path);
	bool create_directory(const std::string& path);

	std::list<std::string> files_in_directory(const std::string& path);

	std::string get_last_error();

	inline bool directory_exists(const std::string& d) { return _fs::exists(d); }
	inline bool file_exists(const std::string& f) { return _fs::exists(f); }
	bool valid_file_name(const std::string& name);
	char get(std::fstream& fp);
	char get(std::ifstream& fp);

	void reset();
}

template<typename T>
void IO_WriteData(std::ofstream& f, const T& data)
{
	DWORD base = (DWORD)&data;
	f << '[';
	for (int i = 0; i < sizeof(T); i += 1) {
		std::stringstream ss;
		std::string s;
		ss << std::hex << (int)(*(BYTE*)(base + i));

		if ((s = ss.str()).size() == 1)
			s.insert(s.begin(), '0');

		f << s;

	}
	f << "]";
}
void IO_WriteData(std::ofstream& f, const void* from, const void* to);
void IO_WriteData(std::ofstream& f, const void* from, DWORD length);

template<typename T>
__forceinline std::optional<T> IO_ReadBlock(std::ifstream& f, size_t amount_of_bytes = sizeof(T))
{
	T data{ };

	char ch = fs::file.current_character;

	if (ch != '[')
		ch = fs::get(f);

	if (f.eof())
		return std::nullopt;

	if (ch != '[') {
		FatalError(std::format("std::optional<T> Prediction::IO_ReadBlock(): expected {} instead of {}", '[', ch));
		return std::nullopt;
	}

	size_t bytes_read = 0;

	if (!amount_of_bytes)
		amount_of_bytes = sizeof(T);

	DWORD base = (DWORD)(&data);

	do {

		std::string hex = "0x";

		for (int i = 0; i < 2; i++) {

			if (f.eof() || !f.good()) {
				FatalError("std::optional<T> IO_ReadBlock(): unexpected end of file");
				return std::nullopt;
			}

			ch = fs::get(f);

			if (bytes_read == amount_of_bytes && ch != ']') {
				FatalError(std::format("bytes_read ({}) == sizeof(T) ({}) && ch != ']' ({})", bytes_read, sizeof(T), ch));
				return std::nullopt;
			}
			else if (bytes_read == amount_of_bytes && ch == ']') {
				//fs::get(f); //skip the newline
				return data;
			}

			if (!IsHex(ch)) {
				FatalError("std::optional<T> IO_ReadBlock(): unexpected end of file");
				return std::nullopt;
			}
			hex.push_back(ch);


		}

		//here it HAS to be from 0 to 255
		auto hex_byte = std::strtol(hex.c_str(), NULL, 0);
		*(BYTE*)base = (BYTE)hex_byte;

		base += 1;
		bytes_read++;

	} while (true);

	FatalError("std::optional<T> Prediction::IO_ReadBlock(): unexpected end of file");
	return std::nullopt;
}
void IO_ReadBlock(std::ifstream& f, void* dst, size_t amount_of_bytes = 0);