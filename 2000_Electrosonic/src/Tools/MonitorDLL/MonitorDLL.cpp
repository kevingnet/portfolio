// MonitorDLL.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "MonitorDLL.h"
#include "windows.h"
#include "malloc.h"
#include "winuser.h"
#include "winbase.h"
#include <stdlib.h>

typedef struct _MYHOOKDATA 
{
	int nType;
	HOOKPROC hkprc; 
    HHOOK hhook;
} MYHOOKDATA; 

MYHOOKDATA myhookdata[3]; 



// Shared DATA
#pragma data_seg(".SHARDATA")
static int MouseCode = 0;

static int KeyboardCode = 0;

static CHAR buf[64];
static CBTACTIVATESTRUCT * Active;
static int CBTWParam = 0;
static char	szCode[128];

static HANDLE	hInstance;
#pragma data_seg()


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			hInstance = hModule;
			break;
		case DLL_THREAD_ATTACH:

			break;
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:

			break;
    }
    return TRUE;
}


/**************************************************************** 
  WH_CBT hook procedure 
 ****************************************************************/  
LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{ 
	
//	LPTSTR ActText;

	if (nCode < 0)  // do not process message 
        return CallNextHookEx(myhookdata[0].hhook, nCode, wParam, lParam);
	
	switch (nCode)     { 
        case HCBT_ACTIVATE:
			
			//szCode = wParam;
			CBTWParam = lParam;
//			Active = (CBTACTIVATESTRUCT *) lParam;
//			CBTWParam = GetWindowText(Active->hWndActive, buf, sizeof(buf)) ; 
//			CBTWParam = Active->hWndActive;

            break;

        case HCBT_CREATEWND:
			CBTWParam = wParam;

            break;
		case HCBT_DESTROYWND: 
			CBTWParam = wParam;
             break;  
         
    } 

    return CallNextHookEx(myhookdata[0].hhook, nCode, wParam, 
        lParam);
}  


 /**************************************************************** 
  WH_MOUSE hook procedure 
 ****************************************************************/  
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{ 
	LPMOUSEHOOKSTRUCT MouseHookParam;

	if (nCode < 0)  // do not process the message 
        return CallNextHookEx(myhookdata[1].hhook, nCode, 
            wParam, lParam);  
	MouseHookParam = (MOUSEHOOKSTRUCT *) lParam;
	MouseCode ++; 

	if(MouseCode > 10) MouseCode = 1;

    return CallNextHookEx(myhookdata[1].hhook, nCode, wParam, 
        lParam);

} 

/**************************************************************** 
  WH_KEYBOARD hook procedure 
 ****************************************************************/  
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{ 
    if (nCode < 0)  // do not process message 
        return CallNextHookEx(myhookdata[2].hhook, nCode, 
            wParam, lParam);

	KeyboardCode ++;
	if(KeyboardCode > 10) KeyboardCode = 1;


    return CallNextHookEx(myhookdata[2].hhook, nCode, wParam, 
        lParam);

 } 

MONITORDLL_API bool Initialize(HWND hwndMainWindow) //HWND hwndMainWindow
{
//	hwndMain = hwndMainWindow;

	myhookdata[0].nType = WH_CBT; 
    myhookdata[0].hkprc = CBTProc; 

    myhookdata[1].nType = WH_MOUSE;
    myhookdata[1].hkprc = MouseProc; 

	myhookdata[2].nType = WH_KEYBOARD; 
    myhookdata[2].hkprc = KeyboardProc;

	for ( int i = 0; i < 3; i++) {
		myhookdata[i].hhook = SetWindowsHookEx( 
                            myhookdata[i].nType, 
                            myhookdata[i].hkprc, 
  //                            (HINSTANCE) NULL, GetCurrentThreadId()); 
                           (HINSTANCE)hInstance, 0 ); //GetCurrentThreadId()
	}
	
	return true;
}

MONITORDLL_API void GetLastMouseEvent( int& MCode) //hours, int& mins, int& secs ) //bool& MouseTime) //
{
	MCode = MouseCode;
}

MONITORDLL_API void GetLastKeyboardEvent( int& KCode)  //hours, int& mins, int& secs )
{	
	KCode	=  KeyboardCode;

}

MONITORDLL_API void GetCBTMessage(char CBTMsg[128])
{
//	CString
	itoa(CBTWParam, CBTMsg, 16);

}
MONITORDLL_API bool Shutdown( void )
{
		for ( int i = 0; i < 3; i++) {
			UnhookWindowsHookEx(myhookdata[i].hhook); 
	}
	return true;
}
