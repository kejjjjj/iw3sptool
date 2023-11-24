#pragma once

#include "pch.hpp"

void FatalError(const std::string& errMsg, const std::source_location& loc=std::source_location::current());
