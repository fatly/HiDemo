// libsourcefilter.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "GraphFilter.h"
using namespace e;

int _tmain(int argc, _TCHAR* argv[])
{
	::CoInitialize(NULL);
	int nCmd = 0;
	CGraphFilter* pGraph = new CGraphFilter();
	HRESULT hr = pGraph->Create();
	assert(SUCCEEDED(hr));
	printf("create graph filter ok\n");
	printf("press key : 1-start,2-pause,3-stop\n");

	while (scanf_s("%d", &nCmd))
	{
		switch (nCmd)
		{
		case 1:
			hr = pGraph->Start();
			printf("start ok\n");
			break;
		case 2:
			hr = pGraph->Pause();
			printf("pause ok\n");
			break;
		case 3:
			hr = pGraph->Stop();
			printf("stop ok\n");
			break;
		default:
			goto _out;
			break;
		}
	}

_out:
	delete pGraph;
	::CoUninitialize();
	system("pause");
	return 0;
}

