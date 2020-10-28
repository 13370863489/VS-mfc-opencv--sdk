#include "StdAfx.h"  
#include "MySkin.h"  
//#include "winuser.h"
map<HWND, WNDPROC> g_mOldFunc;
HHOOK g_hMySkinHook;
GdiplusStartupInput g_gdiplusStartupInput;
ULONG_PTR g_pGdiToken;
#define WM_MYDRAWITEM WM_USER+1  
#define  WM_MYCTLCOLOR WM_USER+2  
#define  WM_MYMEASUREITEM WM_USER+3  
#define  WM_GIFSTOP WM_USER+3  
#define GWL_WNDPROC         (-4)
void InitMySkin()
{
	g_hMySkinHook = SetWindowsHookEx(WH_CBT, _MySkinHookFunc, 0, GetCurrentThreadId());
	GdiplusStartup(&g_pGdiToken, &g_gdiplusStartupInput, NULL);
}
void UninitMySkin()
{
	UnhookWindowsHookEx(g_hMySkinHook);
	GdiplusShutdown(g_pGdiToken);
}
LRESULT CALLBACK
_MySkinHookFunc(int code, WPARAM wParam, LPARAM lParam)
{
	if (code == HCBT_CREATEWND)
	{
		HWND hWin = (HWND)wParam;
		CBT_CREATEWND* s = (CBT_CREATEWND*)lParam;
		TCHAR szClass[255] = { 0 };
		GetClassName(hWin, szClass, 255);

		if (_tcsicmp(szClass, "listbox") == 0)
		{
			int nStyle = GetWindowLong(hWin, GWL_STYLE);
			if (!(nStyle & LBS_OWNERDRAWFIXED))
			{
				SetWindowLong(hWin, GWL_STYLE, GetWindowLong(hWin, GWL_STYLE) | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS);
				g_mOldFunc[hWin] = (WNDPROC)SetWindowLong(hWin, GWL_WNDPROC, (long)_MySkinListBoxProc);
			}

		}
		else if (_tcsicmp(szClass, _T("#32770")) == 0)
		{
			SetWindowLong(hWin, GWL_STYLE, GetWindowLong(hWin, GWL_STYLE) & ~WS_SYSMENU);
			g_mOldFunc[hWin] = (WNDPROC)SetWindowLong(hWin, GWL_WNDPROC, (long)_MySkinDialogProc);

		}
		else   if (_tcsicmp(szClass, _T("button")) == 0)
		{
			int nStyle = GetWindowLong(hWin, GWL_STYLE);
			if (!(nStyle & BS_OWNERDRAW))
			{
				nStyle = (GetWindowLong(hWin, GWL_STYLE) & BS_TYPEMASK);
				if (nStyle == BS_PUSHBUTTON || nStyle == BS_DEFPUSHBUTTON)
				{
					SetWindowLong(hWin, GWL_STYLE, GetWindowLong(hWin, GWL_STYLE) | BS_OWNERDRAW);
					g_mOldFunc[hWin] = (WNDPROC)SetWindowLong(hWin, GWL_WNDPROC, (long)_MySkinButtonProc);

				}
			}
		}
		else   if (_tcsicmp(szClass, _T("static")) == 0)
		{
			int nStyle = GetWindowLong(hWin, GWL_STYLE);
			if (!(nStyle & SS_OWNERDRAW))
			{
				g_mOldFunc[hWin] = (WNDPROC)SetWindowLong(hWin, GWL_WNDPROC, (long)_MySkinStaticProc);
			}
		}
		else   if (_tcsicmp(szClass, _T("edit")) == 0)
		{
			g_mOldFunc[hWin] = (WNDPROC)SetWindowLong(hWin, GWL_WNDPROC, (long)_MySkinEditProc);
		}
		return 0;
	}
	else if (code == HCBT_DESTROYWND)
	{
		HWND hWin = (HWND)wParam;
		if (g_mOldFunc[hWin])
		{
			SetWindowLong(hWin, GWL_WNDPROC, (long)g_mOldFunc[hWin]);
			g_mOldFunc.erase(hWin);
		}
	}
	return CallNextHookEx(g_hMySkinHook, code, wParam, lParam);
}
LRESULT CALLBACK _MySkinDialogProc(HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
)
{
	if (uMsg == WM_MEASUREITEM)
	{
		LPMEASUREITEMSTRUCT ms = (LPMEASUREITEMSTRUCT)lParam;
		HWND hWin = GetDlgItem(hwnd, ms->CtlID);
		SendMessage(hWin, WM_MYMEASUREITEM, wParam, lParam);
		return 0;
	}
	else if (uMsg == WM_DRAWITEM)
	{
		LPDRAWITEMSTRUCT ds = (LPDRAWITEMSTRUCT)lParam;
		HWND h = GetDlgItem(hwnd, ds->CtlID);
		::SendMessage(ds->hwndItem, WM_MYDRAWITEM, wParam, lParam);
		return 0;
	}
	else if (uMsg == WM_CTLCOLOREDIT)
	{
		HBRUSH b = (HBRUSH)::SendMessage(HWND(lParam), WM_MYCTLCOLOR, wParam, lParam);
		return (long)b;
	}
	else if (uMsg == WM_CTLCOLORDLG)
	{
		CDC dc;
		dc.Attach((HDC)wParam);
		static CBrush b;
		static bool bFont = false;
		b.DeleteObject();
		b.CreateSolidBrush(RGB(253, 255, 240));
		dc.Detach();
		return (long)(HBRUSH)b;
	}
	else if (uMsg == WM_NCPAINT)
	{
		HDC hdc = GetWindowDC(hwnd);
		CDC* dc = CDC::FromHandle(hdc);
		CRect r1, r2, rLeft, rTop, rRight, rBottom;
		GetWindowRect(hwnd, r1);

		GetClientRect(hwnd, &r2);
		CWnd::FromHandle(hwnd)->ClientToScreen(&r2);
		rLeft.left = 0;
		rLeft.top = 0;
		rLeft.right = r2.left - r1.left;
		rLeft.bottom = r1.Height();
		rTop.left = 0;
		rTop.top = 0;
		rTop.right = r1.Width();
		rTop.bottom = r2.top - r1.top;
		rRight.left = r2.left - r1.left + r2.Width();
		rRight.top = 0;
		rRight.right = r1.Width();
		rRight.bottom = r1.Height();
		rBottom.left = 0;
		rBottom.top = r2.top - r1.top + r2.Height();
		rBottom.right = r1.Width();
		rBottom.bottom = r1.Height();
		dc->FillSolidRect(&rLeft, RGB(113, 113, 113));
		dc->FillSolidRect(&rTop, RGB(113, 113, 113));
		dc->FillSolidRect(&rRight, RGB(113, 113, 113));
		dc->FillSolidRect(&rBottom, RGB(113, 113, 113));
		CRect rClose;
		rClose.left = r1.Width() - 30;
		rClose.right = r1.Width() - 10;
		rClose.top = 3;
		rClose.bottom = 28;
		dc->Draw3dRect(&rClose, RGB(250, 250, 250), RGB(250, 250, 250));
		dc->SetBkMode(TRANSPARENT);
		dc->SetTextColor(RGB(250, 250, 250));
		dc->DrawText(_T("X"), &rClose, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		HWND hPar = GetParent(hwnd);
		if (hPar == GetDesktopWindow() || IsWindowEnabled(hPar) || hwnd == AfxGetMainWnd()->m_hWnd)
		{
			CRect rMini;
			rMini.left = rClose.left - 25;
			rMini.right = rClose.left - 5;
			rMini.top = 3;
			rMini.bottom = 28;
			dc->Draw3dRect(&rMini, RGB(250, 250, 250), RGB(250, 250, 250));
			dc->DrawText(_T("_"), &rMini, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}
		TCHAR szTitle[_MAX_PATH] = { 0 };
		GetWindowText(hwnd, szTitle, _MAX_PATH);
		dc->TextOut(5, 10, szTitle);
		ReleaseDC(hwnd, hdc);
		return 0;
	}
	else if (uMsg == WM_NCLBUTTONDOWN)
	{
		CPoint p;
		GetCursorPos(&p);
		CRect r, r1, r2;
		GetWindowRect(hwnd, &r);
		CRect rClose;
		rClose.left = r.Width() - 30;
		rClose.right = r.Width() - 10;
		rClose.top = 3;
		rClose.bottom = 28;
		r1 = rClose;
		CRect rMini;
		rMini.left = rClose.left - 25;
		rMini.right = rClose.left - 5;
		rMini.top = 3;
		rMini.bottom = 28;
		r2 = rMini;
		r1.OffsetRect(r.TopLeft());
		r2.OffsetRect(r.TopLeft());
		if (PtInRect(&r1, p))
		{
			PostMessage(hwnd, WM_SYSCOMMAND, SC_CLOSE, MAKELPARAM(p.x, p.y));
			return 0;
		}
		HWND hPar = GetParent(hwnd);
		if (hPar == GetDesktopWindow() || IsWindowEnabled(hPar) || hwnd == AfxGetMainWnd()->m_hWnd)
		{
			if (PtInRect(&r2, p))
			{
				PostMessage(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, MAKELPARAM(p.x, p.y));
				return 0;
			}
		}
	}
	else if (uMsg == WM_NCACTIVATE)
	{
		SendMessage(hwnd, WM_NCPAINT, 0, 0);
		return 1;
	}
	else if (uMsg == WM_SETTEXT)
	{
		CallWindowProc(g_mOldFunc[hwnd], hwnd, uMsg, wParam, lParam);
		SendMessage(hwnd, WM_NCPAINT, 0, 0);
		return 1;
	}
	return CallWindowProc(g_mOldFunc[hwnd], hwnd, uMsg, wParam, lParam);

}
LRESULT CALLBACK _MySkinListBoxProc(HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
)
{
	if (uMsg == WM_ERASEBKGND)
	{
		CDC dc;
		dc.Attach((HDC)wParam);
		CRect r;
		GetClientRect(hwnd, &r);
		dc.FillSolidRect(&r, RGB(255, 255, 176));
		dc.Detach();
		return 0;
	}
	else if (uMsg == WM_MYMEASUREITEM)
	{
		LPMEASUREITEMSTRUCT ms = (LPMEASUREITEMSTRUCT)lParam;
		ms->itemHeight = 30;
		return 0;
	}
	else if (uMsg == WM_MYDRAWITEM)
	{
		LPDRAWITEMSTRUCT ds = (LPDRAWITEMSTRUCT)lParam;
		CDC dc;
		dc.Attach(ds->hDC);
		CRect r = ds->rcItem;
		if ((ds->itemState & ODS_SELECTED))
		{
			dc.FillSolidRect(r.left, r.top, r.Width(), r.Height(), RGB(227, 251, 251));
			dc.DrawEdge(&r, EDGE_SUNKEN, BF_BOTTOM);
		}
		else/* if((ds->itemAction & ODA_DRAWENTIRE)  || !(ds->itemState & ODS_SELECTED))*/
		{
			dc.FillSolidRect(r.left, r.top, r.Width(), r.Height(), RGB(255, 255, 176));
			dc.DrawEdge(&r, EDGE_RAISED, BF_BOTTOM);
		}
		CPen p;
		p.CreatePen(PS_SOLID, 2, RGB(92, 92, 92));
		CPen* oldPen = dc.SelectObject(&p);
		dc.MoveTo(r.left, r.bottom - 1);
		dc.LineTo(r.right, r.bottom - 1);
		dc.SelectObject(oldPen);
		TCHAR txt[255] = { 0 };
		SendMessage(ds->hwndItem, LB_GETTEXT, ds->itemID, (long)txt);
		dc.SetBkMode(TRANSPARENT);
		//  dc.SetBkColor(RGB(222,22,2));  
		dc.SetTextColor(RGB(111, 111, 11));
		CFont f;
		f.CreatePointFont(120, _T("Î¢ÈíÑÅºÚ"));
		CFont* oldFont = dc.SelectObject(&f);
		dc.DrawText(txt, strlen(txt), &(ds->rcItem), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		dc.SelectObject(oldFont);
		dc.Detach();
		return 0;
	}
	return CallWindowProc(g_mOldFunc[hwnd], hwnd, uMsg, wParam, lParam);
}
LRESULT CALLBACK _MySkinButtonProc(HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
)
{
	if (uMsg == WM_MYDRAWITEM)
	{
		LPDRAWITEMSTRUCT ds = (LPDRAWITEMSTRUCT)lParam;
		CDC dc;
		CRect r = ds->rcItem;
		dc.Attach(ds->hDC);
		dc.Draw3dRect(&ds->rcItem, RGB(2, 25, 25), RGB(10, 0, 10));
		dc.FillSolidRect(&ds->rcItem, RGB(143, 143, 216));//Here you can define the required color to appear on the Button.  
		UINT state = ds->itemState; //This defines the state of the Push button either pressed or not.   
		if ((state & ODS_SELECTED))
		{
			dc.DrawEdge(&ds->rcItem, EDGE_SUNKEN, BF_RECT);
		}
		else
		{
			dc.DrawEdge(&ds->rcItem, EDGE_RAISED, BF_RECT);
		}
		dc.SetBkMode(TRANSPARENT);
		dc.SetTextColor(RGB(0, 0, 0));     //Setting the Text Color  

		TCHAR buffer[MAX_PATH];           //To store the Caption of the button.  
		ZeroMemory(buffer, MAX_PATH);     //Intializing the buffer to zero  
		::GetWindowText(ds->hwndItem, buffer, MAX_PATH); //Get the Caption of Button Window   

		dc.DrawText(buffer, &ds->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);//Redraw the Caption of Button Window   
		dc.Detach();
		return 0;
	}
	return CallWindowProc(g_mOldFunc[hwnd], hwnd, uMsg, wParam, lParam);
}
LRESULT CALLBACK _MySkinStaticProc(HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
)
{
	if (uMsg == WM_PAINT)
	{
		HWND hPar = GetParent(hwnd);
		HDC pDc = GetDC(hPar);
		COLORREF color = GetBkColor(pDc);
		ReleaseDC(hPar, pDc);
		CPaintDC dc(CWnd::FromHandle(hwnd));
		CRect r;
		GetClientRect(hwnd, &r);
		dc.FillSolidRect(&r, color);
		TCHAR szText[_MAX_PATH] = { 0 };
		GetWindowText(hwnd, szText, _MAX_PATH);
		CFont* ff = dc.GetCurrentFont();
		LOGFONT lf;
		memset(&lf, 0, sizeof(LOGFONT));
		lf.lfCharSet = DEFAULT_CHARSET;
		ff->GetLogFont(&lf);
		if (lf.lfHeight + 2 > r.Height())
		{
			lf.lfHeight = r.Height() + 2;
		}
		else
		{
			lf.lfHeight = lf.lfHeight + 2;
		}
		_tcscpy(lf.lfFaceName, _T("Î¢ÈíÑÅºÚ"));
		CFont f;
		f.CreateFontIndirect(&lf);
		//  f.CreatePointFont(90,_T("Î¢ÈíÑÅºÚ"));  
		CFont* oldFont = dc.SelectObject(&f);
		dc.SetBkMode(TRANSPARENT);
		dc.SetTextColor(RGB(7, 126, 152));
		int height = dc.DrawText(szText, &r, DT_EDITCONTROL | DT_WORDBREAK | DT_CALCRECT);
		//r.DeflateRect(0,height);  
		dc.DrawText(szText, &r, DT_EDITCONTROL | DT_WORDBREAK);
		dc.SelectObject(oldFont);
		return 0;
	}
	return CallWindowProc(g_mOldFunc[hwnd], hwnd, uMsg, wParam, lParam);
}
LRESULT CALLBACK _MySkinEditProc(HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
)
{
	if (uMsg == WM_MYCTLCOLOR)
	{
		CDC dc;
		dc.Attach((HDC)wParam);
		static CBrush b;
		static bool bFont = false;
		b.DeleteObject();
		b.CreateSolidBrush(RGB(181, 207, 230));
		dc.SetBkMode(TRANSPARENT);
		dc.SetBkColor(RGB(181, 207, 230));
		dc.SetTextColor(RGB(88, 3, 46));
		dc.Detach();
		if (!bFont)
		{
			static CFont f;
			f.CreatePointFont(120, _T("Î¢ÈíÑÅºÚ"));
			SendMessage(hwnd, WM_SETFONT, (WPARAM)(HFONT)f, MAKELPARAM(1, 0));
			bFont = 1;
		}
		return (long)(HBRUSH)b;
	}
	return CallWindowProc(g_mOldFunc[hwnd], hwnd, uMsg, wParam, lParam);
}