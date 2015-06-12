#pragma once
#include <windows.h>
#include "Camera.h"

namespace e
{
	class Beautify;
	class MainWin : public Camera
	{
	public:
		MainWin(void);
		virtual ~MainWin(void);
	public:
		bool Create(void);
		void ShowWindow(void);
		void MainLoop(void);
	private:
		void OnCreate(void);
		void OnKeyDown(int key);
		void OnDestroy(void);

		HBITMAP CreateBitmap(int width, int height, int bitCount);
		void SetBitmap(uint8* bits, int width, int height, int bitCount);
		void DrawBitmap(HDC hDC, HBITMAP hBitmap, int width, int height);
		void UpdateView(void);

		virtual void SampleHandle(uchar* buffer, int width, int height, int bitCount);

		LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK _WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	private:
		HWND hWnd;
		HBITMAP hBitmap;
		Beautify* beautify;
	};
}

