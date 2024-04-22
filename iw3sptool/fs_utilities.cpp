#include "pch.hpp"

std::string fs::exe_file_name()
{
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	return std::string(buffer);
}
std::string fs::exe_path()
{
	std::string f = exe_file_name();
	return f.substr(0, f.find_last_of("\\/"));
}
std::string fs::root_path()
{
	return exe_path() + "\\agent";
}
std::string fs::get_extension(const std::string& _file)
{
	size_t const extensionPos = _file.find_last_of(".");

	if (extensionPos == std::string::npos)
		return "";

	return _file.substr(extensionPos);
}
std::string fs::previous_directory(std::string& directory)
{
	size_t pos = directory.find_last_of('\\');
	if (pos < 1 || pos == std::string::npos)
		return directory;

	return directory.substr(0, pos);
}
std::string fs::get_file_name(const std::string& fullpath)
{
	size_t pos = fullpath.find_last_of('\\');

	if (pos < 1 || pos == std::string::npos)
		return fullpath;

	return fullpath.substr(pos + 1);
}
std::string fs::get_file_name_no_extension(const std::string& fullpath)
{
	auto _file = get_file_name(fullpath);
	auto extension = get_extension(_file);
	return _file.substr(0, _file.size() - extension.size());

}
bool fs::io_open(std::ifstream& fp, const std::string& path, const fs::fileopen type)
{
	if (fp.is_open())
		return true;

	fp.open(path, static_cast<std::ios_base::openmode>(std::underlying_type_t<fs::fileopen>(type)));

	if (!fp.is_open())
		return false;


	return true;
}
void fs::io_close(std::ifstream& f)
{
	if (f.is_open())
		f.close();
}

void fs::create_file(const std::string& path)
{
	std::fstream* nf = new std::fstream(path, std::ios_base::out);
	*nf << "";
	if (nf->is_open())
		nf->close();
	delete nf;
}
bool fs::create_directory(const std::string& path)
{
	return _mkdir((path).c_str()) != -1;
}

std::list<std::string> fs::files_in_directory(const std::string& path)
{
	std::list<std::string> files;

	if (!_fs::exists(path)) {
		return {}; 
	}

	for (const auto& entry : _fs::directory_iterator(path)) {
		if (entry.is_directory())
			continue;

		std::string str = entry.path().string();
		files.push_back(std::move(str)); 
	}

	return (files); //compiler I hope you optimize this! 
}
bool fs::valid_file_name(const std::string& name)
{
	if (name.empty())
		return false;

	for (const auto& i : name) {
		if (!std::isalnum(i) && i != '-' && i != '_' && i != ' ')
			return false;

	}
	return true;
}
std::string fs::get_last_error()
{
	const DWORD errorMessageID = ::GetLastError();
	char* messageBuffer = nullptr;

	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char*)&messageBuffer, 0, NULL);

	//Com_PrintError(CON_CHANNEL_CONSOLEONLY, "IO_WriteData failed with: %s\n", messageBuffer);

	std::string output = std::string(messageBuffer, size);

	LocalFree(messageBuffer);
	return output;
}
char fs::get(std::fstream& fp)
{
	return static_cast<char>(fp.get());
}
char fs::get(std::ifstream& fp)
{
	return static_cast<char>(fp.get());
}
void fs::reset()
{

}

void IO_WriteData(std::ofstream& f, const void* from, const void* to)
{

	DWORD length = std::max((DWORD)to, (DWORD)from) - std::min((DWORD)to, (DWORD)from);

	std::cout << "length: " << length << '\n';

	DWORD base = (DWORD)from;
	f << '[';
	for (size_t i = 0; i < length; i += 1) {
		std::stringstream ss;
		std::string s;
		ss << std::hex << (size_t)(*(BYTE*)(base + i));

		if ((s = ss.str()).size() == 1)
			s.insert(s.begin(), '0');

		f << s;

	}
	f << "]";
}
void IO_WriteData(std::ofstream& f, const void* from, DWORD length)
{

	std::cout << "length: " << length << '\n';

	DWORD base = (DWORD)from;
	f << '[';
	for (size_t i = 0; i < length; i += 1) {
		std::stringstream ss;
		std::string s;
		ss << std::hex << (size_t)(*(BYTE*)(base + i));

		if ((s = ss.str()).size() == 1)
			s.insert(s.begin(), '0');

		f << s;

	}
	f << "]";
}

void IO_ReadBlock(std::ifstream& f, void* data, size_t amount_of_bytes)
{
	char ch = fs::get(f);

	if (f.eof())
		return;

	if (ch != '[') {
		FatalError(std::format("std::optional<T> Prediction::IO_ReadBlock(): expected {} instead of {}", '[', ch));
		return;
	}

	size_t bytes_read = 0;

	DWORD base = (DWORD)(data);

	do {

		std::string hex = "0x";

		for (int i = 0; i < 2; i++) {

			if (f.eof() || !f.good()) {
				FatalError("std::optional<T> Prediction::IO_ReadBlock(): unexpected end of file");
				return;
			}

			ch = fs::get(f);

			if (bytes_read == amount_of_bytes && ch != ']') {
				FatalError(std::format("bytes_read ({}) == sizeof(T) ({}) && ch != ']' ({})", bytes_read, amount_of_bytes, ch));
				return;
			}
			else if (bytes_read == amount_of_bytes && ch == ']') {
				//fs::get(f); //skip the newline
				return;
			}

			if (!IsHex(ch)) {
				FatalError("std::optional<T> Prediction::IO_ReadBlock(): unexpected end of file");
				return;
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
	return;
}