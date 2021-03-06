/***************************************************************
  direct.c
***************************************************************/

#include <windows.h>
#include <tchar.h>
#include <direct.h>
#include <locale.h>
#if WINCE
#include "io.h"
#include <fcntl.h>
#include "wince.h" /* for wce_mbtowc */
/* global for chdir, getcwd */
char _currentdir[MAX_PATH+1];
WCHAR _wcurrentdir[MAX_PATH+1];
const __int64 _onesec_in100ns = (__int64)10000000;

/*char *getcwd(char* buffer, int maxlen)
{
	strcpy( buffer, _currentdir );
	return buffer;
}*/

//TAU
#ifdef WINCE

BOOL PathRemoveFileSpecW(LPWSTR moduleFileNameStr)
{
 /*   if (!*moduleFileNameStr)
        return FALSE;*/

    LPWSTR lastPos = 0;
    LPWSTR curPos = moduleFileNameStr;
    do {
        if (*curPos == L'/' || *curPos == L'\\')
            lastPos = curPos;
    } while (*++curPos);

    if (lastPos == curPos - 1)
        return FALSE;

    if (lastPos)
        *lastPos = 0;
    else {
        moduleFileNameStr[0] = L'\\';
        moduleFileNameStr[1] = 0;
    }

    return TRUE;
}

#if defined(STANDALONE)
struct lconv * __cdecl localeconv(void)
{
	return malloc(sizeof(struct lconv));
}
#endif
#endif

int _chdir(const char * dirname)
{
	if( MAX_PATH < strlen(dirname) )
		return -1;

	strcpy( _currentdir, dirname );
	return 0;
}

int _wchdir(const WCHAR * dirname)
{
	if( MAX_PATH < wcslen(dirname) )
		return -1;

	strcpy( _wcurrentdir, dirname );
	return 0;
}

int _wchmod(const wchar_t * _Filename, int _Mode)
{
    return 0;
}

time_t time( time_t *timer )
{
	SYSTEMTIME s;
	FILETIME   f;
	time_t     t;

	GetSystemTime( &s );

	SystemTimeToFileTime( &s, &f );

	t = wce_FILETIME2time_t(&f);
	if( timer==NULL ) 
		return t;
	else
	  *timer = t;
	return *timer;
}

wchar_t* wce_mbtowc(const char* a)
{
	int length;
	wchar_t *wbuf = 0;

	length = MultiByteToWideChar(CP_UTF8, 0, 
		a, -1, NULL, 0);
	wbuf = (wchar_t*)malloc( (length+1)*sizeof(wchar_t) );
	MultiByteToWideChar(CP_UTF8, 0,
		a, -1, wbuf, length);

	return wbuf;
}

char* wce_wctomb(const wchar_t* w)
{
	DWORD charlength;
	char* pChar;

	charlength = WideCharToMultiByte(CP_UTF8, 0, w,
					-1, NULL, 0, NULL, NULL);
	pChar = (char*)malloc(charlength+1);
	WideCharToMultiByte(CP_UTF8, 0, w,
		-1, pChar, charlength, NULL, NULL);

	return pChar;
}

int _rmdir(const char * dir)
{
	wchar_t *wdir;
	BOOL rc;

	/* replace with RemoveDirectory. */
	wdir = wce_mbtowc(dir);
	rc = RemoveDirectoryW(wdir);
	free(wdir);

	return rc==TRUE ? 0 : -1;
}

int _access(const char *filename, int flags)
{
    wchar_t *wfilename = wce_mbtowc(filename);
	DWORD attr = GetFileAttributesW(filename);
    if (attr  == 0xffffffff) {
        return GetLastError();
    }

	return 0;
}

long _get_osfhandle( int filehandle )
{
	return (long)filehandle;
}

int _chmod(const char *path, int mode)
{
	return 0;
}

static __int64 wce_FILETIME2int64(FILETIME f)
{
	__int64 t;

	t = f.dwHighDateTime;
	t <<= 32;
	t |= f.dwLowDateTime;
	return t;
}

static FILETIME wce_getFILETIMEFromYear(WORD year)
{
	SYSTEMTIME s={0};
	FILETIME f;

	s.wYear      = year;
	s.wMonth     = 1;
	s.wDayOfWeek = 1;
	s.wDay       = 1;

	SystemTimeToFileTime( &s, &f );
	return f;
}

time_t wce_FILETIME2time_t(const FILETIME* f)
{
	FILETIME f1601, f1970;
	__int64 t, offset;

	f1601 = wce_getFILETIMEFromYear(1601);
	f1970 = wce_getFILETIMEFromYear(1970);

	offset = wce_FILETIME2int64(f1970) - wce_FILETIME2int64(f1601);

	t = wce_FILETIME2int64(*f);

	t -= offset;
	return (time_t)(t / _onesec_in100ns);
}

int _rename(const char *oldname, const char *newname)
{
	wchar_t *wold, *wnew;
	int ret = 0;

	wold = wce_mbtowc(oldname);
	wnew = wce_mbtowc(newname);

	/* replace with MoveFile. */
	if ( MoveFileW(wold, wnew) == FALSE )
    {
        ///errno = map_errno(GetLastError());
        ret = -1;
    }

	free(wold);
	free(wnew);

	return ret;
}

int _unlink(const char *file)
{
    return 0;
}

int perror (const char *error)
{
	return 0;
}

long _findfirst( char *file, struct _finddata_t *fi )
{
	HANDLE h;
	WIN32_FIND_DATAW fda;
	char* fname;
    wchar_t* wfile = wce_mbtowc(file);
	h = FindFirstFileW( wfile, &fda );
	if( h==NULL )
	{
		///errno = EINVAL; return -1;
		return -1;
	}

	fi->attrib      = fda.dwFileAttributes;
	fi->time_create = wce_FILETIME2time_t( &fda.ftCreationTime );
	fi->time_access = wce_FILETIME2time_t( &fda.ftLastAccessTime );
	fi->time_write  = wce_FILETIME2time_t( &fda.ftLastWriteTime );
	fi->size        = fda.nFileSizeLow + (fda.nFileSizeHigh<<32);
	fname = wce_wctomb(fda.cFileName);
	strcpy( fi->name, fname );

	return (long)h;
}

int _findnext( long handle, struct _finddata_t *fi )
{
	WIN32_FIND_DATAW fda;
	char* fname;
	BOOL b;

	b = FindNextFileW( (HANDLE)handle, &fda );

	if( b==FALSE )
	{
		//errno = ENOENT; return -1;
		return -1;
	}

	fi->attrib      = fda.dwFileAttributes;
	fi->time_create = wce_FILETIME2time_t( &fda.ftCreationTime );
	fi->time_access = wce_FILETIME2time_t( &fda.ftLastAccessTime );
	fi->time_write  = wce_FILETIME2time_t( &fda.ftLastWriteTime );
	fi->size        = fda.nFileSizeLow + (fda.nFileSizeHigh<<32);
	fname = wce_wctomb(fda.cFileName);
	strcpy( fi->name, fname );

	return 0;
}


int _findclose( long handle )
{
	BOOL b;
	b = FindClose( (HANDLE)handle );
	return b==FALSE ? -1 : 0;
}

int _mkdir(const char * dir)
{
	wchar_t* wdir;
	BOOL rc;

	/* replace with CreateDirectory. */
	wdir = wce_mbtowc(dir);
	rc = CreateDirectoryW(wdir, NULL);
	free(wdir);

	return rc==TRUE ? 0 : -1;
}

#define  g_nMaxFileHandlers  256
static HANDLE g_arFileHandlers[g_nMaxFileHandlers] = {0};
int get_NewFileNumber()
{
    int i = 0;
    int nRes = -1;
    //RHO_LOCK(FileHandlers);
    for( i = 0; i < g_nMaxFileHandlers; i++ )
    {
        if ( g_arFileHandlers[i] == 0 )
        {
            nRes = i;
            break;
        }
    }
   // RHO_UNLOCK(FileHandlers);
    return nRes+1;
}

void set_FileNumber(int fNumber, HANDLE osfh)
{
   // RHO_LOCK(FileHandlers);
    g_arFileHandlers[fNumber-1] = osfh;
   // RHO_UNLOCK(FileHandlers);
}

HANDLE get_OSHandleByFileNumber(int fNumber)
{
    HANDLE res;
   // RHO_LOCK(FileHandlers);
    res = g_arFileHandlers[fNumber-1];
   // RHO_UNLOCK(FileHandlers);
    return res;
}

int _write(int fd, const void *buffer, unsigned count)
{
    DWORD dw;
    if ( fd < 0 ) {
        //char* buf = (char*) malloc(count+1);
        //memcpy(buf,buffer,count);
        //buf[count] = 0;
        //printf("%s",buf);
        //free(buf);
        //dw = count;
        //TBD: fix output of the long strings
       // dw = fwrite( buffer, 1, count, stdout);
    } else {
        HANDLE fHandle = get_OSHandleByFileNumber(fd);
        if ( WriteFile( fHandle, buffer, count, &dw, NULL ) == FALSE )
        {
           // errno = map_errno(GetLastError());
            return -1;
        }

    }
    return (int)dw;
}

int _open(const char *path, int oflag, va_list arg)
{
    wchar_t *wfile = wce_mbtowc(path);
    int nRet = _wopen(wfile, oflag, arg);
    free(wfile);

    return nRet;
}

/* replace "open" with "CreateFile", etc. */
int _wopen(const wchar_t *path, int oflag, va_list arg)
{
    DWORD fileaccess = 0;               /* OS file access (requested) */
    DWORD fileshare = 0;                /* OS file sharing mode */
    DWORD filecreate = 0;               /* OS method of opening/creating */
    DWORD fileattrib = 0;               /* OS file attribute flags */
    SECURITY_ATTRIBUTES SecurityAttributes;
    HANDLE osfh;
    DWORD lasterror = 0;
    int fNumber = -1;

    SecurityAttributes.nLength = sizeof( SecurityAttributes );
    SecurityAttributes.lpSecurityDescriptor = NULL;

    if (oflag & _O_NOINHERIT) {
        SecurityAttributes.bInheritHandle = FALSE;
    }else {
        SecurityAttributes.bInheritHandle = TRUE;
    }

    /*
     * decode the access flags
     */
    switch( oflag & (_O_RDONLY | _O_WRONLY | _O_RDWR) ) {

        case _O_RDONLY:         /* read access */
                fileaccess = GENERIC_READ;
                fileshare = FILE_SHARE_READ;
                break;
        case _O_WRONLY:         /* write access */
                /* giving it read access as well
                 * because in append (a, not a+), we need
                 * to read the BOM to determine the encoding
                 * (ie. ANSI, UTF8, UTF16)
                 */
                if ((oflag & _O_APPEND) /*&& (oflag & (_O_WTEXT | _O_U16TEXT | _O_U8TEXT)) != 0*/ )
                {
                    fileaccess = GENERIC_READ | GENERIC_WRITE;
                }
                else
                {
                    fileaccess = GENERIC_WRITE;
                }
                break;
        case _O_RDWR:           /* read and write access */
                fileaccess = GENERIC_READ | GENERIC_WRITE;
                break;
    }

    /*
     * decode open/create method flags
     */
    switch ( oflag & (_O_CREAT | _O_EXCL | _O_TRUNC) ) {
        case 0:
        case _O_EXCL:                   // ignore EXCL w/o CREAT
            filecreate = OPEN_EXISTING;
            break;

        case _O_CREAT:
            filecreate = OPEN_ALWAYS;
            break;

        case _O_CREAT | _O_EXCL:
        case _O_CREAT | _O_TRUNC | _O_EXCL:
            filecreate = CREATE_NEW;
            break;

        case _O_TRUNC:
        case _O_TRUNC | _O_EXCL:        // ignore EXCL w/o CREAT
            filecreate = TRUNCATE_EXISTING;
            break;

        case _O_CREAT | _O_TRUNC:
            filecreate = CREATE_ALWAYS;
            break;
    }

    /*
     * decode file attribute flags if _O_CREAT was specified
     */
    fileattrib = FILE_ATTRIBUTE_NORMAL;     /* default */

    fNumber = get_NewFileNumber();
    if ( fNumber == 0 )
    {
    	//errno = EMFILE;
        return -1;
    }

    if ( (osfh = CreateFileW( path,
                             fileaccess,
                             fileshare,
                             &SecurityAttributes,
                             filecreate,
                             fileattrib,
                             NULL )) == INVALID_HANDLE_VALUE  )
    {
        if ((fileaccess & (GENERIC_READ | GENERIC_WRITE)) == (GENERIC_READ | GENERIC_WRITE) &&
                (oflag & _O_WRONLY))
        {
            fileaccess &= ~GENERIC_READ;
            osfh = CreateFileW( path,
                     fileaccess,
                     fileshare,
                     &SecurityAttributes,
                     filecreate,
                     fileattrib, NULL );
        }
    }

    lasterror = GetLastError();

    if ( osfh != INVALID_HANDLE_VALUE  )
    {
        if ( (oflag & _O_APPEND) )
            SetFilePointer( osfh, 0, NULL, FILE_END );

        set_FileNumber(fNumber, osfh);
        return fNumber;
    }

    return -1;
}
#if 0
int rb_w32_mkdir(const char *path, int mode)
{
    return _mkdir(path);
}
#endif 
#else

char* wce_wctomb(const wchar_t* w)
{
	DWORD charlength;
	char* pChar;

	charlength = WideCharToMultiByte(CP_UTF8, 0, w,
					-1, NULL, 0, NULL, NULL);
	pChar = (char*)malloc(charlength+1);
	WideCharToMultiByte(CP_UTF8, 0, w,
		-1, pChar, charlength, NULL, NULL);

	return pChar;
}

wchar_t* wce_mbtowc(const char* a)
{
	int length;
	wchar_t *wbuf;

	length = MultiByteToWideChar(CP_UTF8, 0, 
		a, -1, NULL, 0);
	wbuf = (wchar_t*)malloc( (length+1)*sizeof(wchar_t) );
	MultiByteToWideChar(CP_UTF8, 0,
		a, -1, wbuf, length);

	return wbuf;
}
#endif