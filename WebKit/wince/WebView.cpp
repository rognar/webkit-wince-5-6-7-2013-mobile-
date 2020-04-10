/*
 * Copyright (C) 2006, 2007, 2008, 2009, 2010 Apple, Inc.  All rights reserved.
 * Copyright (C) 2010 Patrick Gansterer <paroga@paroga.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "config.h"
#include "WebView.h"

#include "ChromeClientWinCE.h"
#include "ContextMenuClientWinCE.h"
#include "DragClientWinCE.h"
#include "EditorClientWinCE.h"
#include "FocusController.h"
#include "Frame.h"
#include "FrameLoadRequest.h"
#include "FrameLoader.h"
#include "FrameLoaderClientWinCE.h"
#include "FrameView.h"
#include "GraphicsContext.h"
#include "HTMLFormElement.h"
#include "InitializeThreading.h"
#include "InspectorClientWinCE.h"
#include "IntSize.h"
#include "NotImplemented.h"
#include "Page.h"
#include "PlatformKeyboardEvent.h"
#include "PlatformMouseEvent.h"
#include "PlatformStrategiesWinCE.h"
#include "PlatformWheelEvent.h"
#include "ResourceRequest.h"
#include "Settings.h"
#include "SharedBuffer.h"
#include "HistoryItem.h"
#include "WebCoreInstanceHandle.h"
#include <WebCore/platform/win/BitmapInfo.h>
#include "ScriptSourceCode.h"
#include "ScriptValue.h"
#include <wtf/MainThread.h>
#include <string>
#include <cairo-win32.h>

using namespace WebCore;

const LPCWSTR kWebViewWindowClassName = L"WebViewWindowClass";
ENGINEEVENTPROC WebView::m_CallBackFuncs[EEID_MAXEVENTID];
extern WTF::OwnPtr<WebView> webView;


extern "C" void onNavError(const char* url)
{ 
	if(WebView::m_CallBackFuncs[EEID_NAVIGATIONERROR])
		WebView::m_CallBackFuncs[EEID_NAVIGATIONERROR](EEID_NAVIGATIONERROR, (LPARAM)(url), 0);

	//webView->load(L"http://172.16.172.247/HTML/badlink.html");
}

void convertToStringW( const char* szValue, std::wstring& res )
{ 
    if ( !szValue || !*szValue )
        return;

    int nSize = MultiByteToWideChar(CP_UTF8, 0, szValue, -1, NULL, 0);
    if ( nSize > 1 )
    {
        res.resize(nSize-1);
        MultiByteToWideChar(CP_UTF8, 0, szValue, -1, &res[0], nSize);
    }
}


LRESULT CALLBACK WebView::webViewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (WebView* webView = reinterpret_cast<WebView*>(GetWindowLong(hWnd, 0)))
        return webView->wndProc(hWnd, message, wParam, lParam);

    return DefWindowProc(hWnd, message, wParam, lParam);
}

PassRefPtr<SharedBuffer> loadResourceIntoBuffer(const char* name)
{
    notImplemented();
    return 0;
}


WebView::WebView(HWND hwnd, unsigned features)
    : m_frame(0)
    , m_page(0)
	, m_tabIndex(0)
    , m_parentWindowHandle(hwnd)
    , m_enableDoubleBuffer(features & EnableDoubleBuffering)
{	
	RECT rcClient;
    GetClientRect(hwnd, &rcClient);

    m_windowHandle = CreateWindow(kWebViewWindowClassName, 0, WS_CHILD,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, hwnd, 0, WebCore::instanceHandle(), 0);

    SetWindowLong(m_windowHandle, 0, reinterpret_cast<LONG>(this));

    MoveWindow(m_windowHandle, 0, 0, rcClient.right, rcClient.bottom, TRUE);
    ShowWindow(m_windowHandle, SW_SHOW);

	if(m_CallBackFuncs[EEID_TOPMOSTHWNDAVAILABLE])
		m_CallBackFuncs[EEID_TOPMOSTHWNDAVAILABLE](EEID_TOPMOSTHWNDAVAILABLE, (LPARAM)(m_windowHandle), 0);

    Page::PageClients pageClients;
    pageClients.chromeClient = new WebKit::ChromeClientWinCE(this);
    pageClients.contextMenuClient = new WebKit::ContextMenuClientWinCE(this);
    pageClients.editorClient = new WebKit::EditorClientWinCE(this);
    pageClients.dragClient = new WebKit::DragClientWinCE();
    pageClients.inspectorClient = new WebKit::InspectorClientWinCE(this);
    m_page = new Page(pageClients);

    Settings* settings = m_page->settings();
    settings->setDefaultFixedFontSize(14);
    settings->setDefaultFontSize(14);
    settings->setMinimumFontSize(8);
    settings->setMinimumLogicalFontSize(8);
    settings->setScriptEnabled(true);
    settings->setLoadsImagesAutomatically(true);

    WebKit::FrameLoaderClientWinCE* loaderClient = new WebKit::FrameLoaderClientWinCE(this);
    RefPtr<Frame> frame = Frame::create(m_page, 0, loaderClient);
    m_frame = frame.get();
    loaderClient->setFrame(m_frame);

    m_page->mainFrame()->init();

    if (view()) {
        RECT windowRect;
        frameRect(&windowRect);
        view()->resize(IntRect(windowRect).size());
    }
}

WebView::~WebView()
{
    delete m_page;
    DestroyWindow(m_windowHandle);
}

void WebView::initialize(HINSTANCE instanceHandle)
{
    JSC::initializeThreading();
    WTF::initializeMainThread();
    PlatformStrategiesWinCE::initialize();

    WebCore::setInstanceHandle(instanceHandle);

    WNDCLASS wc;
    wc.style          = CS_DBLCLKS;
    wc.lpfnWndProc    = WebView::webViewWndProc;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = sizeof(void *);
    wc.hInstance      = instanceHandle;
    wc.hIcon          = 0;
    wc.hCursor        = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground  = 0;
    wc.lpszMenuName   = 0;
    wc.lpszClassName  = kWebViewWindowClassName;

    RegisterClass(&wc);

	memset(&m_CallBackFuncs,0,sizeof(ENGINEEVENTPROC)*EEID_MAXEVENTID);
}

void WebView::cleanup()
{
    UnregisterClass(kWebViewWindowClassName, WebCore::instanceHandle());
}

PassRefPtr<Frame> WebView::createFrame(const KURL& url, const String& name, HTMLFrameOwnerElement* ownerElement, const String& referrer,
                                       bool /*allowsScrolling*/, int /*marginWidth*/, int /*marginHeight*/)
{
    Frame* coreFrame = m_frame;

    WebKit::FrameLoaderClientWinCE *loaderClient = new WebKit::FrameLoaderClientWinCE(this);
    RefPtr<Frame> childFrame = Frame::create(m_page, ownerElement, loaderClient);
    loaderClient->setFrame(childFrame.get());

    coreFrame->tree()->appendChild(childFrame);
    childFrame->tree()->setName(name);
    childFrame->init();

    // The creation of the frame may have run arbitrary JavaScript that removed it from the page already.
    if (!childFrame->page())
        return 0;

    coreFrame->loader()->loadURLIntoChildFrame(url, referrer, childFrame.get());

    // The frame's onload handler may have removed it from the document.
    if (!childFrame->tree()->parent())
        return 0;

    return childFrame.release();
}

void WebView::runJavaScriptAlert(const String& message)
{
	std::wstring msg;
	convertToStringW(message.ascii().data(), msg);
	MessageBox(m_windowHandle, msg.c_str(), L"", MB_ICONWARNING | MB_OK);
}

void WebView::jsInvoke(LPCTSTR pfunctionJS)
{
	WebCore::ScriptSourceCode sourceJS(pfunctionJS);
	m_frame->script()->evaluate(sourceJS);
}

HWND WebView::getWKWindow()
{
	return m_windowHandle;
}

void WebView::resize(RECT rc)
{
	MoveWindow(m_windowHandle, rc.left, 
        rc.top, 
        (rc.right-rc.left), 
        (rc.bottom-rc.top), 
        FALSE);
}

void WebView::goBackOrForward(int distance)
{
	load(m_frame->loader()->history()->previousItem()->urlString());
}


bool WebView::runJavaScriptConfirm(const String& message)
{
    notImplemented();
    return false;
}

bool WebView::runJavaScriptPrompt(const String& message, const String& defaultValue, String& result)
{
    notImplemented();
    return false;
}

void WebView::frameRect(RECT* rect) const
{
    GetWindowRect(m_windowHandle, rect);
}

FrameView* WebView::view() const
{
    return m_frame ? m_frame->view() : 0;
}

void WebView::load(LPCWSTR url)
{
    load(String(url));
}

void WebView::load(const String &url)
{
    load(WebCore::ResourceRequest(url));
}

void WebView::load(const WebCore::ResourceRequest &request)
{
    frame()->loader()->load(FrameLoadRequest(frame(), request));
}

void WebView::reload()
{
    frame()->loader()->reload();
}

void WebView::stop()
{
	frame()->loader()->stopAllLoaders();
}

void WebView::paint(HDC hDC, const IntRect& clipRect)
{
    FrameView* frameView = view();
    if (!frameView)
        return;

    OwnPtr<HRGN> clipRgn = adoptPtr(CreateRectRgn(clipRect.x(), clipRect.y(), clipRect.maxX(), clipRect.maxY()));
    SelectClipRgn(hDC, clipRgn.get());

    frameView->updateLayoutAndStyleIfNeededRecursive();

    GraphicsContext gc(hDC);
    frameView->paint(&gc, clipRect);

	//GraphicsContext gc(hDC);
	//gc.fillRect(clipRect, Color::darkGray, ColorSpaceDeviceRGB, CompositeSourceOver);

	/*cairo_surface_t *surface;
    cairo_t *cr;
    surface = cairo_win32_surface_create(hDC);
    cr = cairo_create (surface);

    cairo_select_font_face (cr, "serif", CAIRO_FONT_SLANT_NORMAL,
                                         CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, 32.0);
    cairo_set_source_rgb (cr, 0.0, 0.0, 1.0);
    cairo_move_to (cr, 10.0, 50.0);
    cairo_show_text (cr, "Hello, World");
    cairo_destroy (cr);
    cairo_surface_destroy (surface);*/
}

bool WebView::handlePaint(HWND hWnd)
{
    RECT updateRect;
    if (!GetUpdateRect(hWnd, &updateRect, false))
        return false;

    PAINTSTRUCT ps;
    HDC hDC = BeginPaint(m_windowHandle, &ps);

    IntRect clipRect(updateRect);

    if (m_enableDoubleBuffer) {
        if (!m_doubleBufferDC) {
            RECT rcClient;
            GetClientRect(m_windowHandle, &rcClient);

            m_doubleBufferDC = adoptPtr(CreateCompatibleDC(hDC));
            //m_doubleBufferBitmap = adoptPtr(CreateCompatibleBitmap(hDC, rcClient.right, rcClient.bottom));
			void* pixels = NULL;
			BitmapInfo bitmapInfo = BitmapInfo::createBottomUp(IntSize(rcClient.right, rcClient.bottom));
			m_doubleBufferBitmap = adoptPtr(CreateDIBSection(0, &bitmapInfo, DIB_RGB_COLORS, &pixels, 0, 0));
            SelectObject(m_doubleBufferDC.get(), m_doubleBufferBitmap.get());
        }

        paint(m_doubleBufferDC.get(), clipRect);

        BitBlt(hDC, clipRect.x(), clipRect.y(), clipRect.width(), clipRect.height(), m_doubleBufferDC.get(), clipRect.x(), clipRect.y(), SRCCOPY);
    } else
        paint(hDC, clipRect);

    EndPaint(m_windowHandle, &ps);
    return true;
}

bool WebView::handleMouseEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static LONG globalClickCount;
    static IntPoint globalPrevPoint;
    static MouseButton globalPrevButton;
    static LONG globalPrevMouseDownTime;

    // Create our event.
    // On WM_MOUSELEAVE we need to create a mouseout event, so we force the position
    // of the event to be at (MINSHORT, MINSHORT).
    PlatformMouseEvent mouseEvent(hWnd, message, wParam, lParam);

    bool insideThreshold = abs(globalPrevPoint.x() - mouseEvent.position().x()) < ::GetSystemMetrics(SM_CXDOUBLECLK)
                           && abs(globalPrevPoint.y() - mouseEvent.position().y()) < ::GetSystemMetrics(SM_CYDOUBLECLK);
    LONG messageTime = 0;

    bool handled = false;
    if (message == WM_LBUTTONDOWN || message == WM_MBUTTONDOWN || message == WM_RBUTTONDOWN) {
        // FIXME: I'm not sure if this is the "right" way to do this
        // but without this call, we never become focused since we don't allow
        // the default handling of mouse events.
        SetFocus(m_windowHandle);

        PlatformMouseEvent moveEvent(hWnd, WM_MOUSEMOVE, 0, lParam, false);
        moveEvent.setClickCount(0);
        m_page->mainFrame()->eventHandler()->handleMouseMoveEvent(moveEvent);

        // Always start capturing events when the mouse goes down in our HWND.
        SetCapture(m_windowHandle);

        if (insideThreshold && mouseEvent.button() == globalPrevButton)
            globalClickCount++;
        else
            // Reset the click count.
            globalClickCount = 1;
        globalPrevMouseDownTime = messageTime;
        globalPrevButton = mouseEvent.button();
        globalPrevPoint = mouseEvent.position();

        mouseEvent.setClickCount(globalClickCount);
        handled = m_page->mainFrame()->eventHandler()->handleMousePressEvent(mouseEvent);
    } else if (message == WM_LBUTTONDBLCLK || message == WM_MBUTTONDBLCLK || message == WM_RBUTTONDBLCLK) {
        globalClickCount++;
        mouseEvent.setClickCount(globalClickCount);
        handled = m_page->mainFrame()->eventHandler()->handleMousePressEvent(mouseEvent);
    } else if (message == WM_LBUTTONUP || message == WM_MBUTTONUP || message == WM_RBUTTONUP) {
        // Record the global position and the button of the up.
        globalPrevButton = mouseEvent.button();
        globalPrevPoint = mouseEvent.position();
        mouseEvent.setClickCount(globalClickCount);
        m_page->mainFrame()->eventHandler()->handleMouseReleaseEvent(mouseEvent);
        ReleaseCapture();
    } else if (message == WM_MOUSEMOVE) {
        if (!insideThreshold)
            globalClickCount = 0;
        mouseEvent.setClickCount(globalClickCount);
        handled = m_page->mainFrame()->eventHandler()->mouseMoved(mouseEvent);
    }

    return handled;
}

bool WebView::handleMouseWheel(HWND hWnd, WPARAM wParam, LPARAM lParam, bool isHorizontal)
{
    PlatformWheelEvent wheelEvent(hWnd, wParam, lParam, isHorizontal);
    return frame()->eventHandler()->handleWheelEvent(wheelEvent);
}

bool WebView::handleKeyDown(WPARAM virtualKeyCode, LPARAM keyData, bool systemKeyDown)
{
    Frame* frame = m_page->focusController()->focusedOrMainFrame();

    PlatformKeyboardEvent keyEvent(m_windowHandle, virtualKeyCode, keyData, PlatformEvent::RawKeyDown, systemKeyDown);
    bool handled = frame->eventHandler()->keyEvent(keyEvent);

    // These events cannot be canceled, and we have no default handling for them.
    // FIXME: match IE list more closely, see <http://msdn2.microsoft.com/en-us/library/ms536938.aspx>.
    if (systemKeyDown && virtualKeyCode != VK_RETURN)
        return false;

    if (handled) {
        MSG msg;
        if (!systemKeyDown)
            ::PeekMessage(&msg, m_windowHandle, WM_CHAR, WM_CHAR, PM_REMOVE);
        return true;
    }

    return handled;
}

bool WebView::handleKeyPress(WPARAM charCode, LPARAM keyData, bool systemKeyDown)
{
    Frame* frame = m_page->focusController()->focusedOrMainFrame();

    PlatformKeyboardEvent keyEvent(m_windowHandle, charCode, keyData, PlatformEvent::Char, systemKeyDown);
    // IE does not dispatch keypress event for WM_SYSCHAR.
    if (systemKeyDown)
        return frame->eventHandler()->handleAccessKey(keyEvent);
    if (frame->eventHandler()->keyEvent(keyEvent))
        return true;

    return false;
}
	//nado ne sravnivat a strstr nakhodit, togda ne budet zaciklivaniya, i nado ponyat pochemu ne rabotaet file protofol, mozhno v wind32 proverit potom v wm
	//i ne ponyatno pochemu eti eventy ne srabatyvaut pri pervom perehode, na yndex perekhozhu, a kogd na nesushetsvuushii ne srabatyvaet event
//koroche nad ostavit messageboxy... tak ne ponyatno... vobshem nado vyasnit pochemu ne srabatyvaet event kogda iz rho otkryvaesh ssylku, a is webkita ok
//tut raznica odna eto tipa WM!!! i mozhet na wm eto i ne rabotaet, nado logi stavit, khotya na wm rabotate fiel protokol kogda on ne poddrzhivaetsya
//i po yandeksu kogda klikaesh tozhe voitui fignya... nado posmotret chto za url!!!
//mozhno sobrat broser pod wm i tam postavit message box na eti eventy i tozhe sdelt na rhodes app na wm i sravnit
//mozhet v standaloen app  srabatyvaut erory a v rho net!!!
//dlya nachala prosto message box v rho, nado udostoveritsya!!! mozhet ne v etom delo, mozhet vse prikhodit, no fukciya ne otrabatuvaet onnav!!!
/*nadosetmainerro imenno lovit, tam i nav error i velichina taimauta gdeto, posmotru po colsteku kak setmainerror vyzyvaestsya i budu logi stavit chtob
ponyat pochemu v rho eto ne rabotaet
blya znau pokhodu, mozhet iz za nashego http servera to chto u nas base otnositelnyi, nu kstati kohda nash server govorit chto net takogo dokumenta
to po lubomy dolshen setmainerror ili folback idti, nado posmotret chto vozvrashaetsya... proverit applicatio/index111.html peregruzit devais i mozhet komp i zaeiti servisy!!!!*/
bool WebView::handleKeyUp(WPARAM virtualKeyCode, LPARAM keyData, bool systemKeyDown)
{
    PlatformKeyboardEvent keyEvent(m_windowHandle, virtualKeyCode, keyData, PlatformEvent::KeyUp, systemKeyDown);

    Frame* frame = m_page->focusController()->focusedOrMainFrame();
    return frame->eventHandler()->keyEvent(keyEvent);
}

LRESULT WebView::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    bool handled = false;

    if (view()) {
        switch (message) {
        case WM_PAINT:
            handled = handlePaint(hWnd);
            break;

        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        case WM_MBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
            if (frame()->eventHandler() && view()->didFirstLayout())
                handled = handleMouseEvent(hWnd, message, wParam, lParam);
            break;

        case WM_MOUSEWHEEL:
            if (frame()->eventHandler() && view()->didFirstLayout())
                handled = handleMouseWheel(hWnd, wParam, lParam, wParam & MK_SHIFT);
            break;

        case WM_SYSKEYDOWN:
            handled = handleKeyDown(wParam, lParam, true);
            break;

        case WM_KEYDOWN:
            handled = handleKeyDown(wParam, lParam, false);
            break;

        case WM_SYSKEYUP:
            handled = handleKeyUp(wParam, lParam, true);
            break;

        case WM_KEYUP:
            handled = handleKeyUp(wParam, lParam, false);
            break;

        case WM_SYSCHAR:
            handled = handleKeyPress(wParam, lParam, true);
            break;

        case WM_CHAR:
            handled = handleKeyPress(wParam, lParam, false);
            break;

        case WM_CLOSE:
            PostMessage(m_parentWindowHandle, WM_CLOSE, wParam, lParam);
            handled = true;
            break;

        default:
            break;
        }
    }

    if (handled)
        return 0;

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void WebView::RegisterForEvent(EngineEventID eeidEventID, ENGINEEVENTPROC pEventFunc)
{
	m_CallBackFuncs[eeidEventID] = pEventFunc;
}