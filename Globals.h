// globals.h
//

#include <windows.h>
#include <wtypes.h>
#include "CRC32.h"
#include "MD5.h"

#pragma warning(disable: 4786)

#define SZ_FMT_DEFAULT      _T("%d %14.14s %p")
#define SZ_PREDEF_CRC       _T("%c %l") //jbh, changed from %p to %l
#define SZ_PREDEF_MD5       _T("%m *%l") //jbh, changed from %p to %l
#define SZ_PREDEF_SHA1      _T("%h *%l")
#define SZ_PREDEF_SHA256    _T("%H *%l")
#define SZ_PREDEF_CRCSORT   _T("%c $f")
#define SZ_PREDEF_SORTSIZE  _T("$14.14s %p")
#define SZ_PREDEF_SIZE      _T("%14.14s %p")
#define SZ_PREDEF_RELATIVE  _T("%d %14.14s %p")
#define SZ_PREDEF_DIRSIZE   _T("%14.14s %p")
#define SZ_PREDEF_DSORTSIZE _T("$14.14s %p")
#define SZ_DIR_DEFAULT      _T(".")

#define HASH_NONE      0
#define HASH_MD5       1
#define HASH_SHA1      2
#define HASH_SHA256    3

typedef std::map<TString, TString> STRING2STRING;
typedef std::map<__int64, TString> I642STRING;

extern TString g_strCurrentDir;

extern CCRC32 g_crc;
extern CMD5 g_md5;
extern STRING2STRING g_mapSortStr;
extern I642STRING g_mapSortI64;

class CCLOptions
{
public:
    CCLOptions();
    void Process();

    void AddWildcard(TString strFmt);

    BOOL m_bRecursive;
    BOOL m_bRelative;
    BOOL m_bFullTime;
    BOOL m_bHelp;
    BOOL m_bVersion;    //jbh, added to enable version display.
    BOOL m_bFullSizes;
    BOOL m_bLimitType;
    BOOL m_bCompare;
    BOOL m_bProgress;
    BOOL m_bDirNameAsOutput;
    BOOL m_bOutputCurrentDir;
    BOOL m_bSort;
    BOOL m_bSortInt;
    BOOL m_bSortReverse;
    BOOL m_bWarnedIFC;
    BOOL m_bHumanReadable;
    BOOL m_bReverseLimit;
    BOOL m_bPrintUnsorted;
    BOOL m_bForceUnicodeOutput;
    __int64 m_iCurrentSort;
    __int64 m_iPartialHash;
    DWORD m_dwLimitAttr;
    TString m_strDirectory;
    TString m_strFormat;
    TString m_strBaseDirectory;
    TString m_strCurrentSort;
    TString m_strCheckFile;
    TString m_strOutputFile;
    TString m_strFileList;
    TString m_strRelativeRoot;
    int m_iHashSpecified;
    HANDLE m_hOutput;
    FILE *m_fpOutput;

    std::vector<TString> m_vWildcards;
};


// Utility functions

extern void AddSortString(TString strKey, TString strVal);
extern void AddSortI64(__int64 iKey, TString strVal);
extern TString CurrentDirectory();
extern TString FullPath(TString strFile);
extern TString LCString(TString str);
extern TString RelativePath(TString strPath, TString strBaseDirectory, BOOL bShowRootDir);
extern TString basename(TString strFull);
extern BOOL IsDirectory(TString strDirTest, TString strDirParent);
extern BOOL ContainsWildcard(TString strTest);
extern BOOL ContainsFormatChar(TString strTest);
extern __int64 CalculateDirectorySize(TString strPath);
extern TString AttributeString(WIN32_FIND_DATA &ffd);
extern TString CRCString(TString strPath);
extern TString SHA1String(TString strPath);
extern TString SHA256String(TString strPath);
extern TString MD5String(TString strPath, __int64 iPartial = 0, TString *strPartialHashes = NULL);
extern TString AddCommas(TString strNumber);
extern TString StringDate(FILETIME ft);
extern TString StringTime(FILETIME ft, BOOL bFull);
extern TString GetDirectory(TString strFull);
extern TString chomp(TString str);
extern TString GetCWDBase();
extern TString GetHashName(int iHash);
extern TString EnsureBackslash(TString str);
extern __int64 GetFileSize64(TString strFile);
extern TString PathCombineIfRelative(TString strBase, TString strTail);
