#pragma once


#define PB_ONTOPMOSTWINDOW				WM_USER + 9
#define PB_WINDOW_RESTORE				WM_USER + 10
#define PB_SCREEN_ORIENTATION_CHANGED	WM_USER + 11

/**
*  Events fired by the engine which can be registered for via 
*  RegisterForEvent.
*  Note the values given to this enumeration should be sequential and start from 
*  zero, this is to enable registered functions to be iterated over using 
*  pointer arithmetic.
*  \todo determine if my comments for EEID_PROGRESSCHANGE are accurate.
*/

enum EngineEventID
{
	EEID_BEFORENAVIGATE = 0,///< Fired Before a new page is navigated to, if the calling application instructs a CEngine::Navigate or the user clicks on a link.
	EEID_NAVIGATECOMPLETE,	///< Fired once the server responds to the client's navigation request, the page now starts loading.
	EEID_DOCUMENTCOMPLETE,  ///< Fired when the page is 100% loaded included all embedded images.
	EEID_NAVIGATIONERROR,	///< Fired if there is an error navigating to the page, for example attempting to navigate to http://www.motorola.com if the device does not have a network connection.
	EEID_METATAG,			///< Fired when a Meta Tag is parsed on the loaded page.  This event is fired once for each meta tag, so a page with 3 meta tags will invoke this event 3 times.
	/**
	* Fires when the title of a document in the object becomes available or changes.  
	* Because the title might change while an HTML page is downloading, the URL of 
	* the document is set as the title. If the HTML page specifies a title, it is parsed, 
	* and the title is changed to reflect the actual title.
	*/
	EEID_TITLECHANGE,		
	EEID_NAVIGATIONTIMEOUT, ///< Fired when the page load takes longer than the value specified by CEngine::SetNavigationTimeout
	EEID_STATUSTEXTCHANGE,	///< Fired when the status bar text on the currently loaded page changes.
	EEID_PROGRESSCHANGE,	///< Fired when the progress of a download operation is updated.  LOWORD(value) contains the maximum progress value.  HIWORD(value) contains the current progress value, or -1 if the download is complete.
	EEID_TOPMOSTHWNDAVAILABLE,	///<  Deprecated - use EEID_ENGINE_INITIALISED instead to get the engine window handle
	EEID_SETSIPSTATE,			///<  Fired when an editable field gains or loses focus (deprecated)
	EEID_CONSOLEMESSAGE,		///<  Fired when a console message is available from WebKit (could be a JS error or window.console.log)
	EEID_JSPOPUP_ALERT,			///<  Fired when the rendering engine encounters a JS Alert
	EEID_JSPOPUP_CONFIRM,		///<  Fired when the rendering engine encounters a JS Confirm
	EEID_JSPOPUP_PROMPT,		///<  Fired when the rendering engine encounters a JS Prompt
	EEID_JSPOPUP_AUTHENTICATION, ///<  Fired when the engine requires authentication
	EEID_GPS_POSITIONRECEIVED,	///<  Position message from GPS Driver
	EEID_GPS_LOCATIONERROR,		///<  Location Error message from the GPS Driver
	EEID_SETSIPINPUTTYPE,		///<  Fired when an editable field gains or loses focus (replaces EEID_SETSIPSTATE)
	EEID_APPLICATIONUNLICENSED,	///<  Fired when the Engine can not navigate to a specified URL because the application is unlicensed.
        EEID_IS_REQUEST_HANDLED,    ///< If this request is going to be handled by the platform return true.
	EEID_LOG,
	EEID_ENGINE_INITIALISED,	///<  Fired when the engine has completed initialisation and the TopMost Window handle is available.   Value contains the HWND cast to an integer.  TabID is not used.
	EEID_DIALOG,				///<  Fired when the engine pops up or closes a JS Dialog
	EEID_ENGINE_EXIT,			///<  Fired when the engine has shutdown.  Handler must return TRUE to override default behaviour (which is to call PostQuitMessage).
	//this event ID MUST remain at the END of the list
	EEID_MAXEVENTID			///< Maximum event Identifier.  This does not represent an event but can be used in sizing arrays etc. 
};

enum TextZoomValue
{
	TEXT_ZOOM_SMALLEST = 0,	///< Smallest possible text size
	TEXT_ZOOM_SMALLER = 1,	///< Slightly smaller than normal text size
	TEXT_ZOOM_NORMAL = 2,	///< Normal Text size
	TEXT_ZOOM_BIGGER = 3,	///< Slightly larger than normal text size
	TEXT_ZOOM_BIGGEST = 4	///< Largest possible text size
};

typedef LRESULT (CALLBACK *PWKINITIALIZE)(HINSTANCE);
typedef LRESULT (CALLBACK *PWKCLEANUP)();
typedef LRESULT (CALLBACK *PWKCREATE)(HWND, bool);
typedef LRESULT (CALLBACK *PWKDESTROY)(int);
typedef LRESULT (CALLBACK *PWKLOAD)(LPCWSTR, int);
typedef LRESULT (CALLBACK *PWKRELOAD)(bool, int);
typedef LRESULT (CALLBACK *PWKSTOP)(int);
typedef LRESULT (CALLBACK* ENGINEEVENTPROC)(EngineEventID,	LPARAM, int);
typedef LRESULT (CALLBACK *PWKREGISTERFOREVENT)(EngineEventID, ENGINEEVENTPROC);
typedef HWND (CALLBACK *PWGETWKWINDOW)(int);
typedef LRESULT (CALLBACK *PWKJSINVOKE)(int, LPCTSTR);
typedef LRESULT (CALLBACK *PWKSWITCH)(int);
typedef LRESULT (CALLBACK *PWKFORWARD)(int);
typedef LRESULT (CALLBACK *PWKRESIZE)(int, RECT);
typedef LRESULT (CALLBACK *PWKBACK)(int, int);
typedef LRESULT (CALLBACK *PWKZOOMPAGE)(float, int);
typedef LRESULT (CALLBACK *PWKZOOMTEXT)(TextZoomValue, int);
typedef LRESULT (CALLBACK *PWKGETTEXTZOOM)(TextZoomValue*, int);
