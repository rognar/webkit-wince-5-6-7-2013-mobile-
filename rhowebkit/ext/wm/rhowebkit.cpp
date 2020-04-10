// rhowebkit.cpp : Defines the entry point for the DLL application.
//

#include "config.h"
#include "WebView.h"
#include <windows.h>
#include <wtf/PassOwnPtr.h>

WebView* g_webView = 0; //implement multiple webview support

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

extern "C" __declspec ( dllexport ) LRESULT CALLBACK initialize(HINSTANCE instanceHandle)
{
    WebView::initialize(instanceHandle);

	return S_OK;
}

extern "C" __declspec ( dllexport ) LRESULT CALLBACK cleanup()
{
    WebView::cleanup();

	return S_OK;
}

extern "C" __declspec ( dllexport ) LRESULT CALLBACK create(HWND hwnd, bool enableDoubleBuffer = true)
{
    if(!g_webView)
		g_webView = new WebView(hwnd, enableDoubleBuffer); //implement multiple webview support

	if(!g_webView) return -1;
	
	int tabIndex = 0;

	return tabIndex;
}

extern "C" __declspec ( dllexport ) LRESULT CALLBACK destroy(int tabIndex)
{
    if(!g_webView) return S_FALSE;
	
	delete g_webView;
	g_webView = 0;

	return S_OK;
}

extern "C" __declspec ( dllexport ) LRESULT CALLBACK load(LPCWSTR url, int tabIndex)
{
    if(!g_webView) return S_FALSE;
	
	g_webView->load(url);

	return S_OK;
}

extern "C" __declspec ( dllexport ) LRESULT CALLBACK reload(bool cache, int tabIndex)
{
    if(!g_webView) return S_FALSE;
		
	g_webView->reload();

	return S_OK;
}

extern "C" __declspec ( dllexport ) LRESULT CALLBACK stop(int tabIndex)
{
    if(!g_webView) return S_FALSE;

	g_webView->stop();

	return S_OK;
}

extern "C"  __declspec ( dllexport ) LRESULT CALLBACK RegisterForEvent(EngineEventID eeidEventID, ENGINEEVENTPROC pEventFunc)
{
	//if(!g_webView) return S_FALSE;
	
	WebView::RegisterForEvent(eeidEventID, pEventFunc);
	
	return TRUE;
}

extern "C" __declspec ( dllexport ) HWND CALLBACK getWKWindow(int tabIndex)
{
	if(!g_webView) return NULL;

	return 	g_webView->getWKWindow();
}

extern "C"  __declspec ( dllexport ) LRESULT CALLBACK jsInvoke(int tabIndex, LPCTSTR pfunctionJS)
{
	if(!g_webView) return S_FALSE;

	g_webView->jsInvoke(pfunctionJS);
	
	return S_OK;
}


extern "C" __declspec ( dllexport ) LRESULT CALLBACK switchTo(int tabIndex)
{
	if(!g_webView) return S_FALSE;

	return S_OK;
}

extern "C" __declspec ( dllexport ) LRESULT CALLBACK forward(int tabIndex)
{
	if(!g_webView) return S_FALSE;

	return S_OK;
}

extern "C" __declspec ( dllexport ) LRESULT CALLBACK resize(int tabIndex, RECT rc)
{
	if(!g_webView) return S_FALSE;

	g_webView->resize(rc);

	return S_OK;
}

extern "C" __declspec ( dllexport ) LRESULT CALLBACK back(int tabIndex, int pCount)
{
	if(!g_webView) return S_FALSE;

	g_webView->goBackOrForward(pCount);

	return TRUE;
}

extern "C" __declspec ( dllexport ) LRESULT CALLBACK zoomPage(float zoom, int tabIndex)
{
	if(!g_webView) return S_FALSE;

	return S_OK;
}

extern "C" __declspec ( dllexport ) LRESULT CALLBACK zoomText(TextZoomValue zoom, int tabIndex)
{
	if(!g_webView) return S_FALSE;

	return S_OK;
}

extern "C" __declspec ( dllexport ) LRESULT CALLBACK getTextZoom(TextZoomValue* zoom, int tabIndex)
{
	if(!g_webView) return S_FALSE;

	return S_OK;
}
