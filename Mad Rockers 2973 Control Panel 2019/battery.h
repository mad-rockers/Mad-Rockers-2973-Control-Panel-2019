#pragma once

#include <string>
#include <Windows.h>
#include <tchar.h>  
#include <fstream>
#include <locale>
#include <codecvt>
#include <ctime>
#include <vector>
#include <algorithm>

//add class inheritence to tools

LRESULT CALLBACK Battery_WndProc(
	_In_ HWND   hwnd,
	_In_ UINT   uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
);

class Battery {
	bool init = false;
	int lt = -1;
	HWND parent;
	HINSTANCE instance;
	HWND thisWindow;
	WNDCLASSEX wc;
	float batteryPercentage = 0.0f, batteryVoltage;
	bool makeClass() {

		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = 0;
		wc.lpfnWndProc = Battery_WndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = L"MR_Battery";
		wc.hInstance = instance;
		wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

		if (!RegisterClassEx(&wc))
			return false;
		return true;
	}
	bool makeWindow() {

		RECT win;
		win.left = 0;
		win.right = 200;
		win.top = 0;
		win.bottom = 100;
		AdjustWindowRect(&win, WS_CAPTION | WS_THICKFRAME, false);
		thisWindow = CreateWindow(L"MR_Battery", L"Battery", WS_OVERLAPPED | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, win.right, win.bottom, parent, NULL, instance, NULL);
		if (!thisWindow)
			return false;
		SetWindowLongPtr(thisWindow, GWLP_USERDATA, (LONG_PTR)this);

		render();

		return true;
	}
	void render() {
		if (!init)
			return;
		InvalidateRect(thisWindow, NULL, FALSE);
	}
public:
	vector<float> pcs;
	HWND getHWND() {
		return thisWindow;
	}
	void setBatteryPercentageVoltage( float _percentage, float _voltage) {
		batteryPercentage = _percentage;
		batteryVoltage = _voltage;

		/*if (lt == -1 || time(0) - lt > 1) {
			lt = time(0);
			pcs.push_back(_percentage);
		}*/

		pcs.push_back(_percentage);

		if (pcs.size() > 1000)
			pcs.erase(pcs.begin(), pcs.begin()+500);


		render();
	}
	void initialize(HWND _parent, HINSTANCE _instance) {
		if (init)
			return;

		parent = _parent;
		instance = _instance;
		if (!makeClass())
			return;
		if (!makeWindow())
			return;
		init = true;

	}
	void show() {
		if (init) {
			ShowWindow(thisWindow, SW_SHOW);
			RECT rec;
			GetWindowRect(parent, &rec);
			SetWindowPos(thisWindow, NULL, rec.left + 10, rec.top + 10, 0, 0, SWP_NOSIZE);
			UpdateWindow(thisWindow);
		}
	}
	bool hasData() {
		if (!init)
			return false;
		return false;
	}
	void shutdown() {
		if (!init)
			return;
		init = false;
		//Window close
		DestroyWindow(thisWindow);

	}
	~Battery() {
		shutdown();
	}

	friend LRESULT CALLBACK Battery_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

LRESULT CALLBACK Battery_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Battery* thisBattery = (Battery*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	PAINTSTRUCT ps;
	HDC hdc;
	switch (message)
	{
	case WM_PAINT: {
		hdc = BeginPaint(hWnd, &ps);
		HBRUSH panelGray = CreateSolidBrush(RGB(50, 50, 50));
		HBRUSH lightGray = CreateSolidBrush(RGB(100, 100, 100));
		HBRUSH red = CreateSolidBrush(RGB(255, 0, 0));
		HBRUSH green = CreateSolidBrush(RGB(0, 255, 0));
		RECT genRect;
		GetClientRect(hWnd, &genRect);
		FillRect(hdc, &genRect, panelGray);
		RECT rRect = { 5,5,genRect.right-5,genRect.bottom - 5 };
		FillRect(hdc, &rRect, lightGray);
		rRect = { 10,10,genRect.right - 10,genRect.bottom - 10 };
		FillRect(hdc, &rRect, red);

		vector<float>& pcs = thisBattery->pcs;

		vector<float> renderV(genRect.right-genRect.left-20,1000.0f);

		
		if (pcs.size() > 0) {
			float w = float(renderV.size()) / pcs.size();

			for (int i = 0; i < pcs.size(); i++) {
				for (int i2 = 0; i2 < (w>=1.0f?int(w):1); i2++) {
					if (i*w + i2 < renderV.size()) {
						renderV[i*w + i2] = min(renderV[i*w+i2],pcs[i]);
					}
				}
			}
			for (int i = 0; i < renderV.size(); i++) {
				if (renderV[i] == 1000.0f&&i != 0)
					renderV[i] = renderV[i - 1];
			}

			vector<float> renderV2(renderV.size());

			//First smooth pass
			for (int i = 0; i < renderV.size()-1; i++) {
				renderV2[i] = (renderV[i] + renderV[i + 1]) / 2.0f;
			}
			renderV2[renderV.size() - 1] = renderV[renderV.size() - 1];

			//Second smooth pass
			for (int i = 0; i < renderV2.size() - 1; i++) {
				renderV[i] = (renderV2[i] + renderV2[i + 1]) / 2.0f;
			}
			renderV[renderV.size() - 1] = renderV2[renderV.size() - 1];

			//Second smooth pass


			for (int i = 0; i < renderV.size(); i++) {
				rRect = { i + 10, int(genRect.top + 10 + (genRect.bottom - genRect.top - 20)*(1.0f - renderV[i])), i + 1 + 10, genRect.bottom - 10 };
				FillRect(hdc, &rRect, green);
			}
		}



		std::wstring str;
		str = to_wstring(thisBattery->batteryVoltage)+L"v "+to_wstring(int((thisBattery->batteryPercentage)*100))+L"% ";
		TextOut(hdc, 12, 12, str.c_str(), str.size());

		/*
		if (!thisFieldMap)
		TextOut(hdc,24,2,L"Not Linked", 11);
		else {
		if (thisFieldMap->hasData())
		{
		TextOut(hdc, 24, 2, L"Has Data", 8);
		}
		else
		{
		TextOut(hdc, 24, 2, L"No Data", 8);
		}
		}*/
		DeleteObject(panelGray); DeleteObject(red); DeleteObject(green); DeleteObject(lightGray);
		EndPaint(hWnd, &ps); }
				   break;
	case WM_CLOSE:
		ShowWindow(hWnd, SW_HIDE);
		return 1;
		break;
	case WM_DESTROY:

		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}