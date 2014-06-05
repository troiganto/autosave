// AutoSave.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "AutoSave.h"
#include "Application.h"

declare_logging();

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	begin_logging();
	log(L"------------------------");
	log(L"   AutoSave log file    ");
	log(L"------------------------\n");
	log(L"Command line:");
	log(pCmdLine);

	Application mainWindow(pCmdLine);
	mainWindow.registerWindowClass();
	if (!mainWindow.create())
	{
		log(L"Window creation failed! Exiting ...");
		return 0;
	}

	// We do /not/ show the main window.
	// 'tis a mere dummy.
	// mainWindow.showWindow(nCmdShow);
	mainWindow.mainLoop();


	end_logging();

	return 0;
}
