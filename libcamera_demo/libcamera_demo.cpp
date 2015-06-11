#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Camera.h"
#include <windows.h>

using namespace e;


int main(int argc, char* argv[])
{
	::CoInitialize(NULL);

	Camera camera;

	char buf[32];

	while (1)
	{
		scanf("%s", buf);

		if (strcmp(buf, "r") == 0)
		{
			bool ret = camera.Start();
			printf("camera start %s\n", ret?"ok":"fail");
			if (ret)
			{
				camera.GetFormat();
			}
		}
		else if (strcmp(buf, "s") == 0)
		{
			camera.Stop();
			printf("camera stop.\n");
		}
		else if (strcmp(buf, "q") == 0)
		{
			break;
		}
	}

	::CoUninitialize();

	return 0;
}