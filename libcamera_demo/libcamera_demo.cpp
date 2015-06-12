#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MainWin.h"
#include <windows.h>

using namespace e;

// int main(int argc, char* argv[])
// {
// 	::CoInitialize(NULL);
// 
// 	Beautify* camera = new Beautify;
// 
// 	printf("'r' -> run.\n's' -> stop.\n'q' -> quit.\ninput a key:\n");
// 
// 	char buf[32] = { 0 };
// 
// 	while (1)
// 	{
// 		scanf("%s", buf);
// 
// 		if (strcmp(buf, "r") == 0)
// 		{
// 			bool ret = camera->Start();
// 			
// 			if (ret)
// 			{
// 				camera->GetFormat();
// 			}
// 
// 			printf("camera start %s\n", ret ? "ok" : "fail");
// 		}
// 		else if (strcmp(buf, "s") == 0)
// 		{
// 			camera->Stop();
// 			printf("camera stop.\n");
// 		}
// 		else if (strcmp(buf, "-") == 0)
// 		{
// 			camera->KeyPress('-');
// 		}
// 		else if (strcmp(buf, "=") == 0)
// 		{
// 			camera->KeyPress('=');
// 		}
// 		else if (strcmp(buf, "q") == 0)
// 		{
// 			break;
// 		}
// 	}
// 
// 	delete camera;
// 
// 	::CoUninitialize();
// 
// 	return 0;
// }

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