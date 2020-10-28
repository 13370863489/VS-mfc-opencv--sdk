#pragma once
#pragma warning(disable:4786)  
#include <map>  
#ifndef ULONG_PTR  
//#define ULONG_PTR unsigned long*  
#include "GdiPlus.h"  
using namespace Gdiplus;
#endif  
#pragma comment(lib,"gdiplus.lib")  
using namespace std;
void InitMySkin();
void UninitMySkin();
LRESULT CALLBACK
_MySkinHookFunc(int code, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK _MySkinListBoxProc(HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
);
LRESULT CALLBACK _MySkinDialogProc(HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
);
LRESULT CALLBACK _MySkinButtonProc(HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
);
LRESULT CALLBACK _MySkinStaticProc(HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
);
LRESULT CALLBACK _MySkinEditProc(HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
);