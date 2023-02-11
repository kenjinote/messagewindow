#define UNICODE
#pragma comment(linker,"/opt:nowin98")
#include<windows.h>
#include"resource.h"

TCHAR szClassName[]=TEXT("Window");
#define MONITORS_MAX_LIM 256	//	サポートする最大モニター数
RECT monitors[MONITORS_MAX_LIM];	//	各モニターの緒元
int monitors_max=0;					//	実際のモニター数
RECT maxboundrect;
TCHAR szText[1024];

BOOL myinfoenumproc(HMONITOR hMon,HDC hdcMon,LPRECT lpMon,LPARAM dwDate)
{
	monitors[monitors_max].bottom=lpMon->bottom;
	monitors[monitors_max].left=lpMon->left;
	monitors[monitors_max].right=lpMon->right;
	monitors[monitors_max].top=lpMon->top;
	UnionRect(&maxboundrect,&maxboundrect,&monitors[monitors_max]);
	if(MONITORS_MAX_LIM-1 <= monitors_max)
		return FALSE;
	++monitors_max;
	return TRUE;
}

CALLBACK DialogProc(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		if(LOWORD(wParam)==IDC_EDIT1)
		{
			if(HIWORD(wParam)==EN_CHANGE)
			{
				EnableWindow(GetDlgItem(hDlg,IDOK),GetWindowTextLength(GetDlgItem(hDlg,IDC_EDIT1)));
			}
		}
		else if(LOWORD(wParam)==IDOK)
		{
			GetDlgItemText(hDlg,IDC_EDIT1,szText,1024);
			EndDialog(hDlg,LOWORD(wParam));
			return TRUE;
		}
		else if(LOWORD(wParam)==IDCANCEL) 
		{
			EndDialog(hDlg,LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

LRESULT CALLBACK WndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
	case WM_CREATE:
		EnumDisplayMonitors(NULL,NULL,(MONITORENUMPROC)myinfoenumproc,0);
		{
			int n;
			LPTSTR* argv = CommandLineToArgvW(GetCommandLine(), &n);
			if(n==2)
			{
				lstrcpy(szText,argv[1]);
			}
			if (argv) LocalFree(argv);
		}
		if(szText[0]||IDOK==DialogBox(
			((LPCREATESTRUCT)lParam)->hInstance,
			MAKEINTRESOURCE(IDD_DIALOG1),
			hWnd,
			(DLGPROC)DialogProc))
		{
			typedef BOOL __stdcall SETLAYER(
				HWND hwnd,
				COLORREF crKey,
				BYTE bAlpha,
				DWORD dwFlags);
			SETLAYER *SetLayeredWindowAttributes;
			HINSTANCE hDllInst=LoadLibrary(TEXT("user32"));
			if(hDllInst)
			{
				SetLayeredWindowAttributes=(SETLAYER*)GetProcAddress(
					hDllInst,"SetLayeredWindowAttributes");
				if(SetLayeredWindowAttributes)
				{
					SetWindowLong(
						hWnd,
						GWL_EXSTYLE,
						GetWindowLong(hWnd,GWL_EXSTYLE)|0x80000);
					SetLayeredWindowAttributes(hWnd,0,128,2);
				}
			}
			MoveWindow(hWnd,maxboundrect.left,maxboundrect.top,(maxboundrect.right-maxboundrect.left),(maxboundrect.bottom-maxboundrect.top),0);
		}
		else
		{
			return -1;
		}
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc=BeginPaint(hWnd,&ps);
			RECT cliantrect;
			COLORREF OldColor=SetTextColor(hdc,RGB(255,255,255));
			DWORD OldBkMode=SetBkMode(hdc,TRANSPARENT);
			HFONT hFont=CreateFont(128,0,0,0,FW_NORMAL,FALSE,FALSE,0,SHIFTJIS_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,TEXT("メイリオ"));
			HFONT hOldFont=(HFONT)SelectObject(hdc,hFont);
			int i;
			for(i=0;i<monitors_max;i++)
			{
				CopyRect(&cliantrect,&monitors[i]);
				ScreenToClient(hWnd,(POINT*)&cliantrect);
				ScreenToClient(hWnd,(POINT*)&cliantrect.right);
				DrawText(hdc,szText,-1,&cliantrect,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
			}
			SelectObject(hdc,hOldFont);
			SetBkMode(hdc,OldBkMode);
			SetTextColor(hdc,OldColor);
			EndPaint(hWnd,&ps);
		}
		break;
	case WM_SYSKEYDOWN:
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd,msg,wParam,lParam);
	}
	return 0;
}

EXTERN_C void __cdecl WinMainCRTStartup()
{
	MSG msg;
	HINSTANCE hInstance=GetModuleHandle(0);
	WNDCLASS wndclass={
		0,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)GetStockObject(BLACK_BRUSH),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd=CreateWindowEx(
			WS_EX_TOPMOST|WS_EX_TOOLWINDOW,
			szClassName,
			TEXT("Window"),
			WS_POPUP,
			0,
			0,
			500,
			500,
			0,
			0,
			hInstance,
			0
		);
	ShowWindow(hWnd,SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while(GetMessage(&msg,0,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	ExitProcess(msg.wParam);
}

#if _DEBUG
void main(){}
#endif
