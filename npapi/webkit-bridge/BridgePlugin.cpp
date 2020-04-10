#include <windows.h>
#include <stdio.h>

#include <wininet.h>
#pragma comment(lib, "wininet.lib")

#include "bridge.h"
#include "npfunctions.h"
#include "bridge_cb.h"

//  These are the method / property names our plugin object will export
static NPIdentifier idFramework;

static NPObject* AllocateBridgePluginObject(NPP npp, NPClass *aClass)
{
	//  Called in response to NPN_CreateObject
	BridgePluginObject* obj = new BridgePluginObject(npp);

	return obj;
}

static int http_cb(const BridgeCB_String *request, BridgeCB_String *result, void *user_data);


//  NPAPI Macro
DECLARE_NPOBJECT_CLASS_WITH_BASE(BridgePluginObject,
                                 AllocateBridgePluginObject);

bool BridgePluginObject::Construct(const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	bool bRetVal = false;

	// Create Object, expect no arguments
	if (argCount == 0)
	{
		NPObject* genericObj = NPN_CreateObject(mNpp, GET_NPOBJECT_CLASS(BridgePluginObject));
		if (!genericObj)
			return false;
		BridgePluginObject* obj = (BridgePluginObject*)genericObj;
		OBJECT_TO_NPVARIANT(genericObj, *result);
		bRetVal = true;
	}
  return bRetVal;
}

/******************************************************************************/
BridgePluginObject::BridgePluginObject(NPP npp)
    : ScriptablePluginObjectBase(npp)
{
}

/******************************************************************************/
BridgePluginObject::~BridgePluginObject()
{
}

/******************************************************************************/
bool BridgePluginObject::HasMethod(NPIdentifier name)
{
	return (name == idFramework);
}

/******************************************************************************/
bool BridgePluginObject::HasProperty(NPIdentifier name)
{
	return false;
}

/******************************************************************************/
bool BridgePluginObject::GetProperty(NPIdentifier name, NPVariant *result)
{
	VOID_TO_NPVARIANT(*result);
	return false;
}

/******************************************************************************/
bool BridgePluginObject::SetProperty(NPIdentifier name, const NPVariant *value)
{
	return false;
}

/******************************************************************************/
char* BridgePluginObject::npStrDup(const char* sIn)
{
	char* sOut = (char*)NPN_MemAlloc(strlen(sIn) + 1);
	if (sOut)
		strcpy(sOut, sIn);

	return sOut;
}

/******************************************************************************/
void WriteLog (char *pformat, ...)
{
	WCHAR *pfilename = new WCHAR [MAX_PATH];
	GetModuleFileName (NULL, pfilename, MAX_PATH);
	WCHAR *pend = wcsrchr (pfilename, L'\\');
	wcscpy (pend + 1, L"rhobridge_log.txt");
	FILE *flog = _wfopen (pfilename, L"ab");
	delete [] pfilename;

	WCHAR *pwtime = new WCHAR [64];
	char *ptime = new char [64];
	GetTimeFormat (LOCALE_SYSTEM_DEFAULT, 0, NULL, NULL, pwtime, 64);
	wcstombs (ptime, pwtime, 64);
	delete [] pwtime;
	fputs (ptime, flog);
	delete [] ptime;

	fputs (": ", flog);

	va_list args;
	va_start (args, pformat);
	char *pentry = new char [128];
	vsprintf (pentry, pformat, args);
	fputs (pentry, flog);
	delete [] pentry;

	fputs ("\n", flog);
	fclose (flog);
}

/******************************************************************************/
bool BridgePluginObject::Invoke(NPIdentifier name, const NPVariant *args,
                               uint32_t argCount, NPVariant *result)
{
/*VOID_TO_NPVARIANT(*result);
return false;*/

	//  Convert to lower case to make our methods case insensitive
	char* szNameCmp = _strlwr(NPN_UTF8FromIdentifier(name));
	NPIdentifier methodName =  NPN_GetStringIdentifier(szNameCmp);
	NPN_MemFree(szNameCmp);

	if (methodName == idFramework)
	{
		if (argCount != 1)
		{
			WriteLog("Wrong number of arguments = %d", argCount);
			goto Exit;
		}

		if (!NPVARIANT_IS_STRING(args[0]))
		{
			WriteLog("First argument is not string");
			goto Exit;
		}

        const BridgeCB_String question = {
            args[0].value.stringValue.UTF8Length,
            (char *)args[0].value.stringValue.UTF8Characters
        };
		BridgeCB_String answer;

		if (BridgeCB_call(&question, &answer)) {
            STRINGN_TO_NPVARIANT(answer.data, answer.size, *result);
            return true;
		}
	}

Exit:
    VOID_TO_NPVARIANT(*result);
	return false;
}

bool BridgePluginObject::InvokeDefault(const NPVariant *args, uint32_t argCount,
													NPVariant *result)
{
	STRINGZ_TO_NPVARIANT(npStrDup("default method return val"), *result);
	return true;
}

//  Constructor for the Plugin, called when the embedded mime type is found on a web page (see npp_new).
//  <embed id="embed1" type="application/x-rho-bridge" hidden=true> </embed> 
CBridgePlugin::CBridgePlugin(NPP pNPInstance) :
  m_pNPInstance(pNPInstance),
  m_pNPStream(NULL),
  m_bInitialized(FALSE),
  m_pScriptableObject(NULL)
{
  	// Must initialise this before getting NPNVPluginElementNPObject, as it'll
	// call back into our GetValue method and require a valid plugin.
	pNPInstance->pdata = this;

    // Say that we're a windowless plugin.
    NPN_SetValue(m_pNPInstance, NPPVpluginWindowBool, false);

	//  Instantiate the values of the methods / properties we possess
	idFramework = NPN_GetStringIdentifier("framework");

    BridgeCB_set_callback_if_none(http_cb, NULL);

	// Export the object onto the page
	NPObject *sWindowObj;
	NPN_GetValue(m_pNPInstance, NPNVWindowNPObject, &sWindowObj);
	NPObject *myBridgeObject = NPN_CreateObject(m_pNPInstance,GET_NPOBJECT_CLASS(BridgePluginObject));
	NPVariant v;
	OBJECT_TO_NPVARIANT(myBridgeObject, v);
	NPIdentifier n = NPN_GetStringIdentifier("WebkitBridge");
	NPN_SetProperty(m_pNPInstance, sWindowObj, n, &v);
	NPN_ReleaseObject(myBridgeObject);
	NPN_ReleaseObject(sWindowObj);
}

CBridgePlugin::~CBridgePlugin()
{
}

NPBool CBridgePlugin::init(NPWindow* pNPWindow)
{
	m_Window = pNPWindow;
	m_bInitialized = TRUE;

	return m_bInitialized;
}

void CBridgePlugin::shut()
{
	m_bInitialized = FALSE;
}

NPBool CBridgePlugin::isInitialized()
{
	return m_bInitialized;
}

NPObject* CBridgePlugin::GetScriptableObject()
{
	return NULL;
}

static int http_cb(const BridgeCB_String *request, BridgeCB_String *result, void *user_data)
{
	/*HINTERNET hinternet, hconnection, hrequest;
	char *pbuffer, *pdata;
	DWORD read, length, index, written, available;
	BOOL success;

	//  Called when a method is called on an object
	bool bReturnVal = false;

	// Makes a POST request. It passes the specified text as the request body,
	// and returns the body of the server response.
	// The server and resource are specified in a test file called RhoLocalServer.txt

	// TESTING
	//char test_body[] = "{\"method\":\"isCreated\",\"params\": [],\"__rhoClass\":\"Rho.NativeTabbar\",\"__rhoID\":\"0\",\"jsonrpc\":\"2.0\",\"id\":0}";

	WCHAR *pfilename = new WCHAR [MAX_PATH];
	GetModuleFileName (NULL, pfilename, MAX_PATH);
	WCHAR *pend = wcsrchr (pfilename, L'\\');
	wcscpy (pend + 1, L"RhoLocalServer.txt");
	DEBUGMSG(TRUE, (L"[Bridge] pfilename = %s\n", pfilename));

	FILE *furl = _wfopen (pfilename, L"rt");
	char *pline = new char [128];
	fgets (pline, 128, furl);
	fclose (furl);
	delete [] pfilename;
	DEBUGMSG(TRUE, (L"[Bridge] pline = %S\n", pline));

	// Get ASCII host and port
	char *pcolon = strrchr (pline, ':');
	*pcolon = '\0';

	// Find host and port parts
	char *phost;
	char *pslash = strstr (pline, "//");
	if (pslash)
		phost = pslash + 2;
	else
		phost = pline;
	char *pport = pcolon + 1;

	// Get wide char host
	WCHAR host [32];
	mbstowcs (host, phost, 32);

	// Get port number
	int port = atoi (pport);

	delete [] pline;

	DEBUGMSG(TRUE, (L"[Bridge] host = %s, port = %d\n", host, port));

	hinternet = InternetOpen (L"NPAPI", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (!hinternet)
	{
		WriteLog ("InternetOpen error = %d", GetLastError());
		goto Exit;
	}
	DEBUGMSG(TRUE, (L"[Bridge] InternetOpen succeeded\n"));

	hconnection = InternetConnect (hinternet, host, port, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
	if (!hconnection)
	{
		WriteLog ("InternetConnect error = %d", GetLastError());
		goto Exit;
	}
	DEBUGMSG(TRUE, (L"[Bridge] InternetConnect succeeded\n"));
    
	WCHAR presource[] = L"/system/js_api_entrypoint";
	hrequest = HttpOpenRequest (hconnection, L"POST", presource, L"HTTP/1.0", NULL, NULL,
		INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_RELOAD, 0);
	if (!hrequest)
	{
		WriteLog ("HttpOpenRequest error = %d", GetLastError());
		goto Exit;
	}
	DEBUGMSG(TRUE, (L"[Bridge] HttpOpenRequest succeeded\n"));

    success = HttpSendRequest(hrequest, NULL, 0, request->data, request->size);
	if (!success)
	{
		WriteLog ("HttpSendRequest error = %d", GetLastError());
		goto Exit;
	}
	DEBUGMSG(TRUE, (L"[Bridge] HttpSendRequest succeeded\n"));

	// Get value of Content-Length
	index = 0;
	read = sizeof length;
	success = HttpQueryInfo (hrequest, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &length, &read, &index);
	if (!success)
	{
		WriteLog ("HttpQueryInfo error = %d", GetLastError());
		DEBUGMSG(TRUE, (L"[Bridge] Content-Length not available\n"));
		goto Exit;
	}

	DEBUGMSG(TRUE, (L"[Bridge] Content-Length = %d\n", length));

	if (length == 0)
	{
		WriteLog ("Content-Length is zero");
		goto Exit;
	}

	pdata = (char*) NPN_MemAlloc (length);
	pbuffer = pdata;
	written = 0;
	while (written < length)
	{
		// Get amount of data available
		success = InternetQueryDataAvailable (hrequest, &available, 0, 0);
		if (!success)
		{
			WriteLog ("InternetQueryDataAvailable error = %d", GetLastError());
			goto Exit;
		}

		// If there is none then wait and try again
		if (available == 0)
		{
			WriteLog ("No bytes available, waiting for retry");
			Sleep (50);
			continue;
		}

		// Read the available data
		success = InternetReadFile (hrequest, pbuffer, available, &read);
		if (!success)
		{
			WriteLog ("InternetReadFile error = %d", GetLastError());
			goto Exit;
		}

		if (read != available)
		{
			WriteLog ("%d bytes read, doesn't match available");
			goto Exit;
		}

		pbuffer += read;
		written += read;
	}
    result->data = pdata;
	result->size = length;

	bReturnVal = true;

Exit:
	if (hrequest) InternetCloseHandle (hrequest);
	if (hconnection) InternetCloseHandle (hconnection);
	if (hinternet) InternetCloseHandle (hinternet);

	return bReturnVal;*/

	return true;
}
