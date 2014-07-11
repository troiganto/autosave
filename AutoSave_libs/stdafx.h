// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

// Program name and version
#define APP_NAME L"Broken Pen's AutoSave"
#define DEFAULT_REGISTRY_KEY APP_NAME
#define CONNECTED_SHORTCUTS_REGISTRY_VALUE_NAME L"connectedShortcutsList"
#define SHORT_APP_NAME L"AutoSave"
#define APP_VERSION2 L"1.0.0"
#ifdef _WIN64
#define APP_VERSION APP_VERSION2
#else
#define APP_VERSION APP_VERSION2 L" (x86)"
#endif
#define APP_DATE3(x) L ## x
#define APP_DATE2(x) APP_DATE3(x)
#define APP_DATE APP_DATE2(__DATE__)


// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN
// Windows Header Files:
#include <windows.h>
#include <windowsx.h>
#include <CommCtrl.h>
#include <Prsht.h>
#include <Shellapi.h>
#include <Shlwapi.h>


// C RunTime Header Files

#include <regex>
#include <string>
#include <vector>
#include <unordered_map>
#include <tchar.h>
#include <Strsafe.h>

#include <exception>
#include <iostream>
#include <cassert>

#include <memory.h>
#include <malloc.h>
#include <stdlib.h>

// Debugging macros

#ifndef NDEBUG
#define declare_logging() FILE* logger_stream
#define begin_logging() _wfreopen_s(&logger_stream, L"AutoSave.log", L"w", stderr)
#define end_logging() fclose(logger_stream)
#define log(x) std::wclog << (x) << L"\n"
#define dump_var(x) std::wclog << #x << ": " << (x) << L"\n";
#define full_log(x) fprintf(std::wclog, "Line %d in file %s: %s", __LINE__, __FILE__, (x))
#else
#define declare_logging() 
#define begin_logging() 
#define end_logging() 
#define log(x) 
#define dump_var(x)
#define full_log(x) 
#endif

// Other macros

#define LoadAppIcon(id) LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(id))

