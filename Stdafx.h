// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN     

#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1

#include <stdio.h>
#include <tchar.h>

#include <windows.h>
#include <winbio.h>

#include <vector>
#include <string>

#ifndef ARGUMENT_PRESENT
#define ARGUMENT_PRESENT(x) (((x) != NULL))
#endif

#ifndef TSTRING
#ifdef _UNICODE
typedef std::wstring TSTRING;
#else
typedef std::string TSTRING;
#endif
#endif

#include "BioHelper.h"