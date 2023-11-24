
#include "pch.hpp"

void FatalError(const std::string& errMsg, const std::source_location& loc)
{
	std::stringstream total;
	total << "Fatal exception in "; 
	total << std::quoted(loc.file_name());
	total << "\non line[" << loc.line() << ", " << loc.column() << "] in function " << loc.function_name() << '\n';

	total << "\nReason: " << errMsg;

	MessageBox(NULL, total.str().c_str(), "Fatal Error!", MB_ICONERROR);

	CG_ReleaseHooks();

	//other cleanup

	exit(-1);
}