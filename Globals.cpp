// globals.cpp
//

#include "stdafx.h"
#include "globals.h"
#include "SHA.h"

TString g_strCurrentDir = _T("");

CCRC32 g_crc;
CSHA g_sha;
CMD5 g_md5;
STRING2STRING g_mapSortStr;
I642STRING g_mapSortI64;
STRING2STRING::iterator g_itStr;
I642STRING::iterator g_itI64;

void AddSortString(TString strKey, TString strVal)
{
    if (g_mapSortStr.empty())
        g_itStr = g_mapSortStr.insert(STRING2STRING::value_type(strKey, strVal)).first;
    else
    {
        g_itStr = g_mapSortStr.find(strKey);
        if (g_itStr == g_mapSortStr.end())
        {
            // Insert new entry
            g_itStr = g_mapSortStr.insert(g_itStr , STRING2STRING::value_type(strKey, strVal));
        } else
        {
            // Append to existing entry
            (*g_itStr).second.append(_T("\n") + strVal);
        }
    }
}

void AddSortI64(__int64 iKey, TString strVal)
{
    if (g_mapSortI64.empty())
        g_itI64 = g_mapSortI64.insert(I642STRING::value_type(iKey, strVal)).first;
    else
    {
        g_itI64 = g_mapSortI64.find(iKey);
        if (g_itI64 == g_mapSortI64.end())
        {
            // Insert new entry
            g_itI64 = g_mapSortI64.insert(g_itI64 , I642STRING::value_type(iKey, strVal));
        } else
        {
            // Append to existing entry
            (*g_itI64).second.append(_T("\n") + strVal);
        }
    }
}

TString CurrentDirectory()
{
    TCHAR szCurrentDir[MAX_PATH];
    DWORD dwLength = GetCurrentDirectory(MAX_PATH, szCurrentDir);
    if (dwLength <= MAX_PATH)
        return szCurrentDir;

    TCHAR *szLongPath = new TCHAR[dwLength];
    GetCurrentDirectory(dwLength, szCurrentDir);
    TString szResult = szLongPath;
    delete[] szLongPath;
    return szResult;
}

TString DirNameFromFullPath(TString str)
{
    size_t i = str.find_last_of('\\');
    if (i == str.npos)
        return str;

    str = str.substr(0, i);

    i = str.find_last_of('\\');
    if (i == str.npos)
        return str;

    return str.substr(i+1);
}

TString FullPath(TString strFile)
{
    TCHAR *sz;
    TCHAR szPath[MAX_PATH];
    DWORD dwLength = GetFullPathName(strFile.c_str(), MAX_PATH, szPath, &sz);
    if (dwLength <= MAX_PATH)
        return szPath;

    TCHAR *szLongPath = new TCHAR[dwLength];
    GetFullPathName(strFile.c_str(), dwLength, szLongPath, &sz);
    TString szResult = szLongPath;
    delete[] szLongPath;
    return szResult;
}

TString LCString(TString str)
{
    size_t i;
    for (i = 0; i < str.length(); i++)
    {
        str[i] = tolower(str[i]);
    }

    return str;
}

TString RelativePath(TString strPath, TString strBaseDirectory, BOOL bShowRootDir)
{
	// if directory < 2, we can't have a "trailing" backslash, so just return
	// whatever it is we got.
	if (strBaseDirectory.length() < 2)
		return strPath;

    TString strFullBase = FullPath(strBaseDirectory);

    TString strFullPath = FullPath(strPath);
    size_t iFind = LCString(strFullPath).find(LCString(strFullBase));
    if (iFind != 0)
        return strPath;

    strPath = strFullPath.substr(strFullBase.length());

	if (!bShowRootDir)
        return strPath;

    strFullPath = EnsureBackslash(DirNameFromFullPath(strFullBase));
    strFullPath.append(strPath);
    return strFullPath;

/*
    if (iFind != 0)
        return strFullPath;

    if (g_strCurrentDir.length() >= strFullPath.length())
        return strFullPath;

    return strFullPath.substr(g_strCurrentDir.length());

	

	if (bShowRootDir == TRUE) {
		return strFullPath.substr(strBaseDirectory.find_last_of(_T("\\"),(strBaseDirectory.length() - 2)) + 1);
	}
	else {
		return strFullPath.substr(strBaseDirectory.length());
	}
*/
}

TString basename(TString strFull)
{
    int iFind = strFull.find_last_of(_T("\\"));
    if (iFind == -1)
        return strFull;

    if (iFind == strFull.length() - 1)
        return strFull;

    return strFull.substr(iFind+1);
}

TString PathCombineIfRelative(TString strBase, TString strTail)
{
    if (strTail.length() < 2)
        return EnsureBackslash(strBase);

    if (strTail[0] == _T('/') || strTail[0] == _T('\\') || strTail[1] == _T(':'))
        return strTail;

    return EnsureBackslash(strBase) + strTail;
}

TString chomp(TString str)
{
    if (str.length() < 1)
        return str;

    int index = (int) str.length() - 1;
    while(index >= 0)
    {
        if (str[index] == _T('\r') || str[index] == _T('\n'))
            index--;
        else
            break;
    }

    return str.substr(0, index+1);
}

TString GetDirectory(TString strFull)
{
    int iFind = strFull.find_last_of(_T("\\"));
    if (iFind == -1)
        return _T(".");

    return strFull.substr(0, iFind+1);
}

TString EnsureBackslash(TString str)
{
    if (str.length() < 1)
        return _T("\\");

    if (str.at(str.length() - 1) == _T('\\'))
        return str;

    return str.append(_T("\\"));
}

BOOL IsDirectory(TString strDirTest, TString strDirParent)
{
    WIN32_FIND_DATA ffd;
    HANDLE hFind;

    if (strDirParent.length() > 0)
    {
        if (strDirParent.at(strDirParent.length() - 1) != _T('\\'))
            strDirParent.append(_T("\\"));
    }
    strDirParent.append(strDirTest);
    
    if (ContainsWildcard(strDirTest))
        return FALSE;       // Not going to play that game

    hFind = FindFirstFile(strDirParent.c_str(), &ffd);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        // Check for things like direct sharepoints (\\machine\share)
        strDirParent = EnsureBackslash(strDirParent);
        strDirParent.append(_T("*.*"));
        hFind = FindFirstFile(strDirParent.c_str(), &ffd);
        if (hFind == INVALID_HANDLE_VALUE)
        {
            return FALSE;
        }

        // If the share has files in it, treat it like a directory
        FindClose(hFind);
        return TRUE;
    }

    FindClose(hFind);

    if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        return TRUE;

    return FALSE;
}

BOOL ContainsWildcard(TString strTest)
{
    if (strTest.find_first_of(_T("*?")) != -1)
        return TRUE;

    return FALSE;
}

BOOL ContainsFormatChar(TString strTest)
{
    size_t index = strTest.find_first_of(_T("%$"));

    if (index == strTest.length() - 1)
        return FALSE;

    if (strTest[index+1] == _T('\\'))
        return FALSE;

    if (index != -1)
        return TRUE;

    return FALSE;
}

__int64 CalculateDirectorySize(TString strPath)
{
    __int64 iSize = 0;
    HANDLE hFind;
    WIN32_FIND_DATA ffd;
    TString strTemp;

    if (strPath.length() < 1)
        return 0;

    strTemp = strPath;
    if (*(strTemp.end() - 1) != _T('\\'))
        strTemp.append(_T("\\"));
    strTemp.append(_T("*.*"));

    hFind = FindFirstFile(strTemp.c_str(), &ffd);
    while (hFind != INVALID_HANDLE_VALUE)
    {
        if (!_tcscmp(ffd.cFileName, _T(".")))
            goto SkipFilename;

        if (!_tcscmp(ffd.cFileName, _T("..")))
            goto SkipFilename;

        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            strTemp = strPath;
            if (*(strTemp.end() - 1) != _T('\\'))
                strTemp.append(_T("\\"));
            strTemp.append(ffd.cFileName);
            iSize += CalculateDirectorySize(strTemp);
        } else
        {
            iSize += ((__int64) ffd.nFileSizeHigh << (__int64) 32) | (__int64) ffd.nFileSizeLow;
        }

SkipFilename:
        if (!FindNextFile(hFind, &ffd))
            break;
    }
    if (hFind != INVALID_HANDLE_VALUE)
        FindClose(hFind);

    return iSize;
}

////////////////////////////////////////////////////////////////////////////////
// class CCLOptions
////////////////////////////////////////////////////////////////////////////////

CCLOptions::CCLOptions()
{
    m_bFullSizes = FALSE;
    m_bRecursive = FALSE;
    m_bRelative = FALSE;
    m_bFullTime = FALSE;
    m_bHelp = FALSE;
    m_bVersion = FALSE; //jbh, added to enable version display.
    m_bLimitType = FALSE;
    m_bSort = FALSE;
    m_bSortInt = FALSE;
    m_bSortReverse = FALSE;
    m_bWarnedIFC = FALSE;
    m_bReverseLimit = FALSE;
    m_bCompare = FALSE;
    m_bProgress = FALSE;
    m_bDirNameAsOutput = FALSE;
    m_bPrintUnsorted = FALSE;
    m_iHashSpecified = HASH_NONE;
    m_bForceUnicodeOutput = FALSE;
    m_dwLimitAttr = 0;
    m_strDirectory = SZ_DIR_DEFAULT;
    m_strFormat = SZ_FMT_DEFAULT;
    m_strBaseDirectory = _T("");
    m_strOutputFile = _T("");
    m_hOutput = INVALID_HANDLE_VALUE;
    m_fpOutput = NULL;
    m_iCurrentSort = 0;
    m_bHumanReadable = FALSE;
    m_iPartialHash = 0;
}

void CCLOptions::AddWildcard(TString strFmt)
{
    m_vWildcards.push_back(strFmt);
}

void CCLOptions::Process()
{
    TString strWildcard = _T("");

    int iFind = m_strDirectory.find_last_of(_T('\\'));
    if (iFind > -1)
    {
        if ((!ContainsWildcard(m_strDirectory)) && (IsDirectory(m_strDirectory, _T(""))))
        {
            strWildcard = _T("*.*");
            m_strBaseDirectory = m_strDirectory;
        } else
        {
            strWildcard = m_strDirectory.substr(iFind+1);
            m_strBaseDirectory = m_strDirectory.substr(0, iFind+1);
        }
    } else
    {
        TString strCurrentDir = CurrentDirectory();
        if ((!ContainsWildcard(m_strDirectory)) && (IsDirectory(m_strDirectory, strCurrentDir)))
        {
            strWildcard = _T("*.*");
            m_strBaseDirectory = m_strDirectory;
        } else
        {
            strWildcard = m_strDirectory;
            m_strBaseDirectory = strCurrentDir;
        }
    }

    m_strBaseDirectory = EnsureBackslash(m_strBaseDirectory);
}

////////////////////////////////////////////////////////////////////////////////

extern TString AttributeString(WIN32_FIND_DATA &ffd)
{
    TCHAR szAttr[16];
    _sntprintf_s(szAttr, 16, _T("%c%c%c%c%c%c%c%c%c%c"),
        ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? _T('d') : _T('-'),
        ffd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM ? _T('s') : _T('-'),
        ffd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ? _T('h') : _T('-'),
        ffd.dwFileAttributes & FILE_ATTRIBUTE_READONLY ? _T('r') : _T('-'),
        ffd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE ? _T('a') : _T('-'),
        ffd.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED ? _T('c') : _T('-'),
        ffd.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED ? _T('e') : _T('-'),
        ffd.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE ? _T('o') : _T('-'),
        ffd.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE ? _T('p') : _T('-'),
        ffd.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY ? _T('t') : _T('-'));
    return szAttr;
}

TString SHA1String(TString strPath)
{
    return g_sha.GetSHAString(strPath);
}

TString SHA256String(TString strPath)
{
    return g_sha.GetSHAString(strPath, 256);
}

TString CRCString(TString strPath)
{
    TCHAR szCRC[12];
    DWORD dwCRC = g_crc.FileCRC(strPath.c_str());
    if (!g_crc.Error())
    {
        _sntprintf_s(szCRC, 12, _T("%.8x"), dwCRC);
    } else
    {
        _sntprintf_s(szCRC, 12, _T("%8.8s"), _T(""));
    }
    return szCRC;
}

__int64 GetFileSize64(TString strFile)
{
    WIN32_FIND_DATA ffd;
    HANDLE hFind = FindFirstFile(strFile.c_str(), &ffd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        FindClose(hFind);
        return ((__int64) ffd.nFileSizeHigh << (__int64) 32) | (__int64) ffd.nFileSizeLow;
    }
    return -1;
}

TString MD5String(TString strPath, __int64 iPartial, TString *strPartialHashes)
{
    TCHAR szMD5[65];
    BYTE bMD5[65];
    int i;

    DWORD dwErr = NOERROR;

    __int64 iTotalRead = g_md5.FileMD5(strPath.c_str(), bMD5, &dwErr, iPartial, strPartialHashes);
    if (iTotalRead >= 0)
    {
        if (iTotalRead != GetFileSize64(strPath))
        {
            _sntprintf_s(szMD5, 64, _T("<err: %5d %19I64d>"), dwErr, iTotalRead);
            _ftprintf(stderr, _T("Error %d reading file: %s\n"), dwErr, strPath.c_str());
        }
        else
        {
            for (i = 0; i < 16; i++)
                _sntprintf_s(&szMD5[2*i], 32 - (2*i) + 1, 3, _T("%.2x"), bMD5[i]);
        }
    } else
    {
        _sntprintf_s(szMD5, 64, _T("%32.32s"), _T(""));
    }

    return szMD5;
}

TString AddCommas(TString strNumber)
{
    int iComma = strNumber.length() - 3;

    while (iComma > 0)
    {
        strNumber.insert(iComma, _T(","));
        iComma -= 3;
    }

    return strNumber;
}

TString StringDate(FILETIME ft)
{
    TCHAR szResult[256];
    FILETIME ftLocal;
    SYSTEMTIME st;
    FileTimeToLocalFileTime(&ft, &ftLocal);
    FileTimeToSystemTime(&ftLocal, &st);

    _sntprintf_s(szResult, 256, _T("%4.4d/%2.2d/%2.2d"), st.wYear, st.wMonth, st.wDay);

    return szResult;
}

TString StringTime(FILETIME ft, BOOL bFull)
{
    TCHAR szResult[256];
    FILETIME ftLocal;
    SYSTEMTIME st;
    FileTimeToLocalFileTime(&ft, &ftLocal);
    FileTimeToSystemTime(&ftLocal, &st);

    if (bFull)
        _sntprintf_s(szResult, 256, _T("%2.2d:%2.2d:%2.2d.%3.3d"), st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    else
        _sntprintf_s(szResult, 256, _T("%2.2d:%2.2d"), st.wHour, st.wMinute);

    return szResult;
}

TString GetCWDBase()
{
    TCHAR sz[65536];
    GetCurrentDirectory(65535, sz);
    TString str = sz;
    if (str.length() < 1)
        return _T("ROOT");

    size_t iBS = str.find_last_of(_T("\\/"));
    if (iBS == str.npos)
        return _T("ROOT");

    if (iBS == str.length()-1 && str.length() > 1)
        return _T("ROOT_") + str.substr(0,1);

    return str.substr(iBS+1);
}

TString GetHashName(int iHash)
{
    switch (iHash)
    {
        case HASH_MD5: return _T("md5");
        case HASH_SHA1: return _T("sha1");
        case HASH_SHA256: return _T("sha256");
        default: return _T("dirlist");
    }
}
