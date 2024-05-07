#pragma once

#include <optional>
#include <filesystem>
#include <Windows.h>
#include <list>
#include <fstream>
#include <string>

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

