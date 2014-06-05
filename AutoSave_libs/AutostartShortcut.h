// AutostartShortcut.h : Contains functions for creating and deleting
// a shortcut in the startup folder.
// All functions may throw OleException.

#pragma once

#include "stdafx.h"
#include "Shortcut.h"
#include "OleUtils.h"

using std::wstring;

namespace AutostartShortcut
{
	void create();
	void remove();
	wstring locate();
}
