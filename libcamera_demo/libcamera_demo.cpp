#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MainWin.h"
#include <windows.h>

using namespace e;

int main(int argc, char* argv[])
{
	::CoInitialize(NULL);

	try
	{
		MainWin* window = new MainWin;
		window->Create();
		window->ShowWindow();
		window->MainLoop();
		delete window;
	}
	catch (...)
	{
		
	}

	::CoUninitialize();

	return 0;
}