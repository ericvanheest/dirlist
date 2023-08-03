// dirlist.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "globals.h"
#include <io.h>
#include <atlbase.h>
#include <atlconv.h>
#include <Fcntl.h>

#define DIRLIST_VERSION _T("1.25")

////////////////////////////////////////////////////////////////////////////////


//BEGIN void Version() //
// - display the version information for the executable.
//
void Version()
{
    _tprintf(_T("\
DIRLIST - Versatile directory-listing program.\n\
\n\
Version: %s\n\
\n\
(C) 2004-2012 LockScroll.com - edv@vanheest.org\n\
"), DIRLIST_VERSION);
}
//ENDOF void Version() //

// Not implemented:            -p  Display size/hash progress\n\

void Usage(TCHAR *szExe)
{
	_tprintf(_T("\
Dirlist, version %s\n\
\n\
Usage:    %s [options] [dir [format] [wildcards]]\n\
\n\
Example:  %s C:\\WINDOWS\\*.dll \"%s\"\n\
\n\
Options:  -s  Recursive listing\n\
          -c  Compare hash listing to \"file\" and report\n\
          -f  Calculate full directory sizes\n\
          -t  Use complete time (hh:mm:ss.ms)\n\
          -h  Use human-readable file sizes (KB, MB, GB)\n\
          -r  Reverse any sort order specified\n\
          -l  Use relative pathnames (changes %%p to work as %%l)\n\
          -e  Print unsorted output to stderr\n\
          -o  Output to \"file\"\n\
          -d  Output to a file named [Current dir].[hashtype].txt\n\
          -u  Force UTF16 Unicode output (and input if using -c)\n\
          -aX Limit by type X, where X is any of [DHSRCEOPT], which signify:\n\
              Dir, Hidden, Sys, Readonly, Comp., Encrypt, Offline, Sparse, Temp\n\
\n\
          -p  size   Display a partial-file hash every \"size\" megabytes\n\
          -D  path   Similar to -d but puts the file in \"path\"\n\
          -L  path   Similar to -l but uses \"path\" as the relative root\n\
          -F  list   use \"list\" as the list of files to process\n\
") _T("\
\n\
Format:   As in \"printf\" but with the following format variables:\n\
          %%c  CRC-32 value                %%m  MD-5 value\n\
          %%d  File modification date/time %%D  File creation date/time\n\
          %%f  Long file name              %%F  Short file name\n\
          %%s  File size (no commas)       %%S  File size (with commas)\n\
          %%n  Directory name              %%a  File attributes\n\
          %%P  Absolute path and filename  %%L  Relative-root-path and filename\n\
          %%p  same as %%P except with -l   %%l  Relative-sub-path and filename\n\
          %%w  Length of the filename      %%h  SHA-1 value\n\
                                          %%H  SHA-256 value\n\
") _T("\
\n\
Sorting:  Use a '$' in place of a '%%' to indicate sorting on that variable\n\
\n\
Default formats:  The following options use predefined format strings:\n\
          --crc         \"%s\"               --sortcrc     \"%s\" /a-d\n\
          --sort        \"%s\"          --sortsize    \"%s\"\n\
          --dirsize     \"%s\" /ad /f   --sortdir     \"%s\" /ad /f\n\
          --md5         \"%s\"              --sha1        \"%s\"\n\
                                              --sha256      \"%s\"\n\
"), DIRLIST_VERSION,
    basename(szExe).c_str(), basename(szExe).c_str(),
    SZ_FMT_DEFAULT,
    SZ_PREDEF_CRC,
    SZ_PREDEF_CRCSORT,
    SZ_PREDEF_SIZE,
    SZ_PREDEF_SORTSIZE,
    SZ_PREDEF_DIRSIZE,
    SZ_PREDEF_DSORTSIZE,
    SZ_PREDEF_MD5,
    SZ_PREDEF_SHA1,
    SZ_PREDEF_SHA256
    );
}

CCLOptions ParseArgs(int argc, TCHAR *argv[])
{
    CCLOptions options;

    int iArg = 1;
    int iLineArg = 0;
    int iChar;

    while (iArg < argc)
    {
        if ((argv[iArg][0] == _T('-')) || (argv[iArg][0] == _T('/')))
        {
            iChar = 1;
            while (argv[iArg][iChar] != _T('\0'))
            {
                switch(argv[iArg][iChar])
                {
                    case _T('-'):
                        // --option
                        // if '--help', enable help display
                        if (!_tcsicmp(&argv[iArg][iChar+1], _T("help")))
                            options.m_bHelp = TRUE;
                        // if '--version', enable version display.
                        if (!_tcsicmp(&argv[iArg][iChar+1], _T("version")))
                            options.m_bVersion = TRUE;
                        // else, check for the "pre-packaged" format types.
                        else if (!_tcsicmp(&argv[iArg][iChar+1], _T("crc")))
                            options.m_strFormat = SZ_PREDEF_CRC;
                        else if (!_tcsicmp(&argv[iArg][iChar+1], _T("sha1")))
                        {
                            options.m_strFormat = SZ_PREDEF_SHA1;
                            options.m_iHashSpecified = HASH_SHA1;
                            options.m_bLimitType = TRUE;
                            options.m_bReverseLimit = TRUE;
                            options.m_dwLimitAttr = FILE_ATTRIBUTE_DIRECTORY;
                        }
                        else if (!_tcsicmp(&argv[iArg][iChar+1], _T("sha256")))
                        {
                            options.m_strFormat = SZ_PREDEF_SHA256;
                            options.m_iHashSpecified = HASH_SHA256;
                            options.m_bLimitType = TRUE;
                            options.m_bReverseLimit = TRUE;
                            options.m_dwLimitAttr = FILE_ATTRIBUTE_DIRECTORY;
                        }
                        else if (!_tcsicmp(&argv[iArg][iChar+1], _T("check")))
                        {
                            if (iArg >= argc - 1)
                            {
                                _ftprintf(stderr, _T("Error: --check specified without providing a hash file for comparison\n"));
                                exit(1);
                            }
                            iArg++;
                            options.m_strCheckFile = argv[iArg];
                            options.m_bCompare = TRUE;
                            iChar = _tcslen(argv[iArg])-1;
                            break;
                        }
                        else if (!_tcsicmp(&argv[iArg][iChar+1], _T("sortcrc")))
                        {
                            options.m_strFormat = SZ_PREDEF_CRCSORT;
                            options.m_bLimitType = TRUE;
                            options.m_bReverseLimit = TRUE;
                            options.m_dwLimitAttr = FILE_ATTRIBUTE_DIRECTORY;
                        } else if (!_tcsicmp(&argv[iArg][iChar+1], _T("sortsize")))
                            options.m_strFormat = SZ_PREDEF_SORTSIZE;
                        else if (!_tcsicmp(&argv[iArg][iChar+1], _T("size")))
                            options.m_strFormat = SZ_PREDEF_SIZE;
                        else if (!_tcsicmp(&argv[iArg][iChar+1], _T("md5")))
                        {
                            options.m_iHashSpecified = HASH_MD5;
                            options.m_strFormat = SZ_PREDEF_MD5;
                            options.m_bLimitType = TRUE;
                            options.m_bReverseLimit = TRUE;
                            options.m_dwLimitAttr = FILE_ATTRIBUTE_DIRECTORY;
                        }
                        else if (!_tcsicmp(&argv[iArg][iChar+1], _T("dirsize")))
                        {
                            options.m_strFormat = SZ_PREDEF_DIRSIZE;
                            options.m_bLimitType = TRUE;
                            options.m_bFullSizes = TRUE;
                            options.m_dwLimitAttr = FILE_ATTRIBUTE_DIRECTORY;
                        } else if (!_tcsicmp(&argv[iArg][iChar+1], _T("sortdir")))
                        {
                            options.m_strFormat = SZ_PREDEF_DSORTSIZE;
                            options.m_bLimitType = TRUE;
                            options.m_bFullSizes = TRUE;
                            options.m_dwLimitAttr = FILE_ATTRIBUTE_DIRECTORY;
                        } else if (!_tcsicmp(&argv[iArg][iChar+1], _T("relative")))
                            options.m_strFormat = SZ_PREDEF_RELATIVE;
                        else
                            _ftprintf(stderr, _T("Warning:  Unknown option \"--%s\" - ignored.\n"), &argv[iArg][iChar+1]);
                        while (argv[iArg][iChar] != _T('\0'))
                            iChar++;
                        iChar--;
                        break;
                    case _T('H'):
                    case _T('h'):
						options.m_bHumanReadable = TRUE;
						break;
                    case _T('?'):
                        options.m_bHelp = TRUE;
                        break;
                    //old-style relative mode.
                    case _T('L'):
                        options.m_bRelative = TRUE;
                        if (iArg >= argc - 1)
                        {
                            _ftprintf(stderr, _T("Error: -L specified without providing a path\n"));
                            exit(1);
                        }
                        iArg++;
                        options.m_strRelativeRoot = argv[iArg];
                        iChar = _tcslen(argv[iArg])-1;
                        break;
                    case _T('l'):
                        options.m_bRelative = TRUE;
                        break;
                    case _T('T'):
                    case _T('t'):
                        options.m_bFullTime = TRUE;
                        break;
                    case _T('S'):
                    case _T('s'):
                        options.m_bRecursive = TRUE;
                        break;
                    case _T('O'):
                    case _T('o'):
                        if (iArg >= argc - 1)
                        {
                            _ftprintf(stderr, _T("Error: -o specified without providing a file name\n"));
                            exit(1);
                        }
                        iArg++;
                        options.m_strOutputFile = argv[iArg];
                        iChar = _tcslen(argv[iArg])-1;
                        break;
                    case _T('P'):
                    case _T('p'):
                        if (iArg >= argc - 1)
                        {
                            _ftprintf(stderr, _T("Error: -p specified without providing a size\n"));
                            exit(1);
                        }
                        iArg++;
                        options.m_iPartialHash = _ttoi64(argv[iArg]);
                        iChar = _tcslen(argv[iArg])-1;
                        break;
                    case _T('d'):
                        options.m_bDirNameAsOutput = TRUE;
                        options.m_strOutputFile = _T("");
                        break;
                    case _T('D'):
                        if (iArg >= argc - 1)
                        {
                            _ftprintf(stderr, _T("Error: -D specified without providing a directory\n"));
                            exit(1);
                        }
                        iArg++;
                        options.m_strOutputFile = EnsureBackslash(argv[iArg]);
                        options.m_bDirNameAsOutput = TRUE;
                        iChar = _tcslen(argv[iArg])-1;
                        break;
                    case _T('R'):
                    case _T('r'):
                        options.m_bSortReverse = TRUE;
                        break;
                    case _T('e'):
                    case _T('E'):
                        options.m_bPrintUnsorted = TRUE;
                        break;
                    case _T('u'):
                    case _T('U'):
                        options.m_bForceUnicodeOutput = TRUE;
                        break;
                    case _T('c'):
                    case _T('C'):
                        if (iArg >= argc - 1)
                        {
                            _ftprintf(stderr, _T("Error: -c specified without providing a hash file for comparison\n"));
                            exit(1);
                        }
                        iArg++;
                        options.m_strCheckFile = argv[iArg];
                        iChar = _tcslen(argv[iArg])-1;
                        options.m_bCompare = TRUE;
                        break;
                    case _T('A'):
                    case _T('a'):
                        options.m_bLimitType = TRUE;
                        options.m_bReverseLimit = FALSE;
                        options.m_dwLimitAttr = 0;
                        iChar++;
                        if (argv[iArg][iChar] == _T('-'))
                        {
                            options.m_bReverseLimit = TRUE;
                            iChar++;
                        }
                        switch(argv[iArg][iChar])
                        {
                            case _T('d'):
                            case _T('D'):
                                options.m_dwLimitAttr |= FILE_ATTRIBUTE_DIRECTORY;
                                break;
                            case _T('r'):
                            case _T('R'):
                                options.m_dwLimitAttr |= FILE_ATTRIBUTE_READONLY;
                                break;
                            case _T('s'):
                            case _T('S'):
                                options.m_dwLimitAttr |= FILE_ATTRIBUTE_SYSTEM;
                                break;
                            case _T('h'):
                            case _T('H'):
                                options.m_dwLimitAttr |= FILE_ATTRIBUTE_HIDDEN;
                                break;
                            case _T('c'):
                            case _T('C'):
                                options.m_dwLimitAttr |= FILE_ATTRIBUTE_COMPRESSED;
                                break;
                            case _T('e'):
                            case _T('E'):
                                options.m_dwLimitAttr |= FILE_ATTRIBUTE_ENCRYPTED;
                                break;
                            case _T('o'):
                            case _T('O'):
                                options.m_dwLimitAttr |= FILE_ATTRIBUTE_OFFLINE;
                                break;
                            case _T('p'):
                            case _T('P'):
                                options.m_dwLimitAttr |= FILE_ATTRIBUTE_SPARSE_FILE;
                                break;
                            case _T('T'):
                            case _T('t'):
                                options.m_dwLimitAttr |= FILE_ATTRIBUTE_TEMPORARY;
                                break;
                            default:
                                _ftprintf(stderr, _T("Warning: invalid character following \"-a\" option - ignored\n"));
                                if (argv[iArg][iChar] == _T('\0'))
                                    iChar--;
                        }
                        break;
                    case _T('F'):
                        if (iArg >= argc - 1)
                        {
                            _ftprintf(stderr, _T("Error: -F specified without providing a file name\n"));
                            exit(1);
                        }
                        iArg++;
                        options.m_strFileList = argv[iArg];
                        iChar = _tcslen(argv[iArg])-1;
                        break;
                    case _T('f'):
                        options.m_bFullSizes = TRUE;
                        break;
                    default:
                        _ftprintf(stderr, _T("Warning:  Ignoring invalid option \"-%c\"\n"), argv[iArg][iChar]);
                }
                iChar++;
            }
        } else
        {
            if (ContainsFormatChar(argv[iArg]))
                options.m_strFormat = argv[iArg];
            else if (ContainsWildcard(argv[iArg]))
                options.AddWildcard(argv[iArg]);
            else
                options.AddWildcard(argv[iArg]);

            iLineArg++;
        }
        iArg++;
    }

    if (options.m_bDirNameAsOutput)
    {
        options.m_strOutputFile.append(GetCWDBase() + _T(".") + GetHashName(options.m_iHashSpecified) + _T(".txt"));
    }

    // Check some common problems
    if (options.m_vWildcards.size() < 1)
    {
        options.AddWildcard(_T("*.*"));
    }
    else if (options.m_strDirectory.compare(SZ_DIR_DEFAULT) == 0)
    {
        // Make sure the first "wildcard" isn't something like "\"
        if (IsDirectory(options.m_vWildcards.front(), _T("")))
        {
            options.m_strDirectory = options.m_vWildcards.front();
            if (options.m_vWildcards.size() < 2)
                options.m_vWildcards.front() = _T("*.*");
        }
        else
        {
            options.m_strDirectory = GetDirectory(options.m_vWildcards.front());
        }
    }

    if (options.m_strDirectory.length() == 2 && options.m_strDirectory[1] == _T(':'))
        options.m_strDirectory.append(_T("\\"));

    return options;
}

int FindNextVar(TString strFormat, int iStartPos)
{
    size_t iFind = 0;

    do
    {
        iFind = strFormat.find_first_of(_T("%$"), iStartPos);
        if (iFind == -1)
            return -1;
        if (iFind >= strFormat.length() - 1)
            return -1;
        iStartPos = iFind+1;
    } while ((strFormat.at(iFind+1) == _T('%')) || (strFormat.at(iFind+1) == _T('$')));

    iFind++;

    while (!isalpha(strFormat.at(iFind)))
    {
        iFind++;
        if (iFind >= strFormat.length())
            return -1;
    }

    return iFind;
}

__int64 FileSize64(CCLOptions &options, TString &strPath, WIN32_FIND_DATA &ffd)
{
    __int64 iSize;
    if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (options.m_bFullSizes))
        iSize = CalculateDirectorySize(strPath);
    else
        iSize =  ((__int64) ffd.nFileSizeHigh << (__int64) 32) | (__int64) ffd.nFileSizeLow;

    return iSize;
}

TString Int64ToString(__int64 iSize)
{
    TCHAR szResult[256];
    _sntprintf_s(szResult, 256, _T("%I64d"), iSize);
    return szResult;
}

TString HumanReadable(__int64 iSize)
{
    // Takes a size and converts it to a KB, MB, GB, or TB rating,
    // if appropriate.

    BOOL bExact = FALSE;
    double fResult;
    TString sPostFix = _T("");

    if (iSize < 1024)
    {
        fResult = (double) iSize;
        //JBH - added "B" display for regular bytes.
        sPostFix = _T(" B ");
        bExact = TRUE;
    } else if (iSize < 1024*1024)
    {
        fResult = iSize / 1024.0;
        sPostFix = _T(" KB");
    } else if (iSize < 1024*1024*1024)
    {
        fResult = iSize / (1024.0 * 1024.0);
        sPostFix = _T(" MB");
    } else if (iSize < (__int64) 1024*1024*1024*1024)
    {
        fResult = iSize / ( (__int64) 1024.0 * 1024.0 * 1024.0);
        sPostFix = _T(" GB");
    } else
    {
        fResult = iSize / ( (__int64) 1024.0 * 1024.0 * 1024.0 * 1024.0);
        sPostFix = _T(" TB");
    }

    TCHAR szTemp[32];
    if (bExact)
    {
        _sntprintf_s(szTemp, 32, _T("%.0f"), fResult);
    } else
    {
        _sntprintf_s(szTemp, 32, _T("%.1f"), fResult);
    }

    return TString(szTemp) + sPostFix;
}

TString TranslateVar(CCLOptions &options, TString strFormat, TCHAR cVar, TString &strCurrentDir, TString &strBaseDirectory , WIN32_FIND_DATA &ffd)
{
    TCHAR szResult[1024];
    TString strPath;
    TString strVar;
    TString strPartial;
    TString strResult;
    __int64 iVar;
    int iFind;

    if (strFormat.length() < 2)
        return strFormat;

    BOOL bSortOnVar = FALSE;

    // Check sort options
    iFind = strFormat.find_last_of(_T("%$"));
    if (iFind == -1)
        return strFormat;

    if (strFormat[iFind] == '$')
    {
        bSortOnVar = TRUE;
        options.m_bSort = TRUE;
    }
 
    // All variables are strings
    strFormat[strFormat.length() - 1] = 's';
    strFormat[iFind] = '%';

    strPath = strCurrentDir;
    strPath.append(ffd.cFileName);

    switch(cVar)
    {
        case _T('A'):
        case _T('a'):
            // file attributes
            strVar = AttributeString(ffd);
            break;

        case _T('C'):
        case _T('c'):
            // CRC-32 value
            strVar = CRCString(strPath);
            break;

        case _T('h'):
            strVar = SHA1String(strPath);
            break;
        case _T('H'):
            strVar = SHA256String(strPath);
            break;

        case _T('M'):
        case _T('m'):
            // MD-5 value
            strVar = MD5String(strPath, options.m_iPartialHash, &strPartial);
            if (options.m_iPartialHash > 0)
                strVar = strPartial + strVar;
            break;


        case _T('L'):
            // Relative Pathname, with root dir displayed.
            strVar = RelativePath(strPath,strBaseDirectory,TRUE);
            break;
        case _T('l'):
            // Relative Pathname, NO root dir displayed.
            strVar = RelativePath(strPath,strBaseDirectory,FALSE);
            break;
        case _T('P'):
            if (options.m_bRelative)
            {
                strVar = RelativePath(strPath,options.m_strRelativeRoot.length() > 0 ? options.m_strRelativeRoot : strBaseDirectory,TRUE);
            }
            else
                strVar = FullPath(strPath);
            break;
        case _T('p'):
            // Pathname
            if (options.m_bRelative)
            {
                strVar = RelativePath(strPath,options.m_strRelativeRoot.length() > 0 ? options.m_strRelativeRoot : strBaseDirectory,FALSE);
            }
            else
                strVar = FullPath(strPath);
            break;
        case _T('F'):
            // Short filename
            if (ffd.cAlternateFileName[0] != '\0')
                strVar = ffd.cAlternateFileName;
            else
                strVar = ffd.cFileName;
            break;

        case _T('f'):
            // Long filename
            strVar = ffd.cFileName;
            break;

        case _T('s'):
            // Filesize (no commas)
            iVar = FileSize64(options, strPath, ffd);
            if (options.m_bHumanReadable)
                strVar = HumanReadable(iVar);
            else
                strVar = Int64ToString(iVar);
            if (bSortOnVar)
                options.m_bSortInt = TRUE;
            break;

        case _T('w'):
        case _T('W'):
            // Length of the filename
            strVar = Int64ToString(_tcslen(ffd.cFileName));
            break;

        case _T('S'):
            // Filesize (commas)
            iVar = FileSize64(options, strPath, ffd);
            if (options.m_bHumanReadable)
            {
                strVar = HumanReadable(iVar);
            } else
            {
                strVar = AddCommas(Int64ToString(iVar));
            }

            if (bSortOnVar)
                options.m_bSortInt = TRUE;
            break;

        case _T('d'):
            // Modification date/time
            strVar = StringDate(ffd.ftLastWriteTime);
            strVar.append(_T(" "));
            strVar.append(StringTime(ffd.ftLastWriteTime, options.m_bFullTime));
            break;

        case _T('D'):
            // Creation date/time
            strVar = StringDate(ffd.ftCreationTime);
            strVar.append(_T(" "));
            strVar.append(StringTime(ffd.ftCreationTime, options.m_bFullTime));
            break;

        case _T('N'):
        case _T('n'):
            // Directory name (omit the trailing '\')
            if (strCurrentDir.at(strCurrentDir.length()-1) == '\\')
                strVar = strCurrentDir.substr(0,strCurrentDir.length() - 1);
            else
                strVar = strCurrentDir;
            break;

        default:
            if (!options.m_bWarnedIFC)
            {
                _ftprintf(stderr, _T("Warning: Invalid format character '%c'\n"), cVar);
                options.m_bWarnedIFC = TRUE;
            }
            strVar = _T("");
    }

    if (strVar.length() + strFormat.length() < 1024)
    {
        _sntprintf_s(szResult, 1024, strFormat.c_str(), strVar.c_str());
        strResult = szResult;
    }
    else
    {
        size_t iLen = (size_t) (strVar.length() + strFormat.length() + 512);
        TCHAR *psz = new TCHAR[iLen];
        _sntprintf_s(psz, iLen, iLen-1, strFormat.c_str(), strVar.c_str());
        strResult = psz;
        delete[] psz;
    }

    if (bSortOnVar)
    {
        if (options.m_bSortInt)
            options.m_iCurrentSort = iVar;
        else
            options.m_strCurrentSort = strVar;
    }

    return strResult;
}

void ProcessFile(CCLOptions &options, TString &strCurrentDir, TString &strBaseDirectory, WIN32_FIND_DATA &ffd)
{
    int iStrPos = 0;
    int iLen = 1;
    TString strOut = _T("");
    TString strSubFormat;

    while (iLen > 0)
    {
        iLen = FindNextVar(options.m_strFormat, iStrPos) - iStrPos + 1;
        if (iLen < 1)
        {
            strOut.append(options.m_strFormat.substr(iStrPos));
            break;
        }
        strSubFormat = options.m_strFormat.substr(iStrPos, iLen);
        strOut.append(TranslateVar(options, strSubFormat, strSubFormat.at(strSubFormat.length()-1), strCurrentDir, strBaseDirectory, ffd));
        iStrPos += iLen;
    }

    if (options.m_bPrintUnsorted)
    {
        // Print to stderr no matter what
        _ftprintf(stderr, _T("%s\n"), strOut.c_str());
    }

    if (options.m_bSort)
    {
        if (options.m_bSortInt)
            AddSortI64(options.m_iCurrentSort, strOut);
        else
            AddSortString(options.m_strCurrentSort, strOut);
    } else
    {
        //if (_isatty( _fileno( stdout ) ))
        if (!options.m_bForceUnicodeOutput)
        {
            _fputts(strOut.c_str(), options.m_fpOutput);
            _fputts(_T("\n"), options.m_fpOutput);
        }
        else
        {
            DWORD dwWritten;
            WriteFile(options.m_hOutput, strOut.c_str(), strOut.length() * sizeof(TCHAR), &dwWritten, NULL);
            WriteFile(options.m_hOutput, _T("\r\n"), 2 * sizeof(TCHAR), &dwWritten, NULL);
        }
    }
}

LPCTSTR UnicodePrefix(TString str)
{
    if (str.length() > 2)
        if (str.substr(0,2) == _T("\\\\"))
            return _T("");      // Don't re-prefix things
    return _T("\\\\?\\");
}

void RecurseDirList(CCLOptions &options, TString strCurrentDir, TString strBaseDirectory, bool bRoot)
{
    WIN32_FIND_DATA ffd;
    HANDLE hFind;
    TString strPath;
    TString strTemp;

    if (strCurrentDir.at(strCurrentDir.length() - 1) != _T('\\'))
        strCurrentDir.append(_T("\\"));

    std::vector <TString>::const_iterator cIter;
    cIter = options.m_vWildcards.begin( );

    while (cIter != options.m_vWildcards.end())
    {
        TString strWildcard = basename(*cIter);
        strPath = strCurrentDir + strWildcard;

        hFind = FindFirstFileW((UnicodePrefix(strPath) + FullPath(strPath)).c_str(), &ffd);
        while (hFind != INVALID_HANDLE_VALUE)
        {
            if (!_tcscmp(ffd.cFileName, _T(".")))
                goto SkipFilename;

            if (!_tcscmp(ffd.cFileName, _T("..")))
                goto SkipFilename;

            if (!options.m_strOutputFile.compare(ffd.cFileName))
                goto SkipFilename;

            if (!options.m_bLimitType)
                ProcessFile(options, strCurrentDir, strBaseDirectory, ffd);
            else if (ffd.dwFileAttributes & options.m_dwLimitAttr)
            {
                if (!options.m_bReverseLimit)
                    ProcessFile(options, strCurrentDir, strBaseDirectory, ffd);
            } else
            {
                if (options.m_bReverseLimit)
                    ProcessFile(options, strCurrentDir, strBaseDirectory, ffd);
            }

    SkipFilename:
            if (!FindNextFile(hFind, &ffd))
                break;
        }

        if (hFind != INVALID_HANDLE_VALUE)
	    {
            FindClose(hFind);
		    hFind = INVALID_HANDLE_VALUE;
	    }

        cIter++;
    }

    if (options.m_bRecursive)
    {
        strPath = strCurrentDir + _T("*.*");
        hFind = FindFirstFile(strPath.c_str(), &ffd);
        while (hFind != INVALID_HANDLE_VALUE)
        {
            if (!_tcscmp(ffd.cFileName, _T(".")))
                goto SkipDirname;

            if (!_tcscmp(ffd.cFileName, _T("..")))
                goto SkipDirname;

            if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                goto SkipDirname;

            strTemp = strCurrentDir;
            strTemp.append(ffd.cFileName);
            strTemp.append(_T("\\"));

            RecurseDirList(options, strTemp, strBaseDirectory, false);

SkipDirname:
            if (!FindNextFile(hFind, &ffd))
                break;
        }
    }
    if (hFind != INVALID_HANDLE_VALUE)
	{
        FindClose(hFind);
		hFind = INVALID_HANDLE_VALUE;
	}
}

int CompareHashes(CCLOptions options)
{
    TString szLine;
    FILE *file = NULL;
    int iDifferences = 0;
    int iTotal = 0;

    if (_tfopen_s(&file, options.m_strCheckFile.c_str(), _T("rt")) != 0)
    {
        _ftprintf(stderr, _T("Error opening check file \"%s\" for reading: %d\n"), options.m_strCheckFile.c_str(), GetLastError());
        return -1;
    }

    char sz[8192];
    wchar_t wsz[8192];

    bool bFinished = false;
    bool bFoundAny = false;
    while(!bFinished)
    {
        if (options.m_bForceUnicodeOutput)
        {
            // In this case means "force unicode input" as well
            if (fgetws(wsz, 8192, file) == NULL)
            {
                bFinished = true;
                break;
            }
            szLine = wsz;
        }
        else
        {
            if (fgets(sz, 8192, file) == NULL)
            {
                bFinished = true;
                break;
            }
            size_t chars;
            mbstowcs_s(&chars, wsz, 8192, sz, 8192);
            wsz[8191] = L'\0';
            szLine = wsz;
        }

        TString hashComputed;
        TString hashFromFile;
        TString filename;

        size_t index = szLine.find(_T(" *"));
        if (index == szLine.npos)
            continue;

        bFoundAny = true;

        hashFromFile = szLine.substr(0, index);
        filename = chomp(szLine.substr(index+2));

        bool bValidHash = true;
        for(size_t iCheck = 0; iCheck < hashFromFile.length(); iCheck++)
        {
            char c = tolower(hashFromFile.at(iCheck));
            if (! ( (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ) )
            {
                bValidHash = false;
                break;
            }
        }

        if (bValidHash)
        {
            if (options.m_iHashSpecified == HASH_NONE)
            {
                switch(hashFromFile.length())
                {
                    case 32:
                        options.m_iHashSpecified = HASH_MD5;
                        break;
                    case 40:
                        options.m_iHashSpecified = HASH_SHA1;
                        break;
                    case 64:
                        options.m_iHashSpecified = HASH_SHA256;
                        break;
                    default:
        //                    _ftprintf(stderr, _T("Warning: Unknown hash of length %d detected\n"), hashFromFile.length());
                        break;
                }
            }

            switch (options.m_iHashSpecified)
            {
                case HASH_NONE:
                    break;
                case HASH_MD5:
                    hashComputed = MD5String(filename);
                    break;
                case HASH_SHA1:
                    hashComputed = SHA1String(filename);
                    break;
                case HASH_SHA256:
                    hashComputed = SHA256String(filename);
                    break;
            }
        }

        if (options.m_bRelative)
            filename = basename(filename);

        TString strOut = filename + _T(": ");
        if (!bValidHash)
            strOut += _T("NO HASH");
        else if (hashComputed == hashFromFile)
            strOut += _T("OK");
        else
        {
            if (options.m_iHashSpecified == HASH_NONE)
                strOut += _T("UNKNOWN HASH TYPE");
            else
                strOut += _T("FAILED");
            iDifferences++;
        }
        iTotal++;

        if (!options.m_bForceUnicodeOutput)
        {
            _fputts(strOut.c_str(), options.m_fpOutput);
            _fputts(_T("\n"), options.m_fpOutput);
        }
        else
        {
            DWORD dwWritten;
            WriteFile(options.m_hOutput, strOut.c_str(), strOut.length() * sizeof(TCHAR), &dwWritten, NULL);
            WriteFile(options.m_hOutput, _T("\r\n"), 2 * sizeof(TCHAR), &dwWritten, NULL);
        }
    }
    
    fclose(file);

    if (iDifferences > 0)
    {
        TCHAR sz[256];
        _stprintf_s(sz, 256, _T("WARNING: %d of %d computed checksums did NOT match\n"), iDifferences, iTotal);

        if (!options.m_bForceUnicodeOutput)
            _fputts(sz, options.m_fpOutput);
        else
        {
            DWORD dwWritten;
            WriteFile(options.m_hOutput, sz, _tcslen(sz) * sizeof(TCHAR), &dwWritten, NULL);
        }
    }

    return iDifferences;
}

int _tmain(int argc, TCHAR * argv[])
{
    CCLOptions options = ParseArgs(argc, argv);
    g_strCurrentDir = CurrentDirectory();
    if (*(g_strCurrentDir.end() - 1) != _T('\\'))
        g_strCurrentDir.append(_T("\\"));
    
    // if it was requested, display the help information.
    if (options.m_bHelp)
    {
        Usage(argv[0]);
        return 0;
    }

    // if it was requested, display the Version information.
    if (options.m_bVersion)
    {
        Version();
        return 0;
    }

    options.Process();

    if (options.m_strOutputFile != _T(""))
    {
        options.m_hOutput = CreateFile(options.m_strOutputFile.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
        if (options.m_hOutput == INVALID_HANDLE_VALUE)
        {
            _ftprintf(stderr, _T("Error:  Could not open output file \"%s\" - Error %d\n"), options.m_strOutputFile.c_str(), GetLastError());
            exit(1);
        }
        int handle = _open_osfhandle((LONG)options.m_hOutput, _O_TEXT);
        options.m_fpOutput = (FILE *) _fdopen(handle, "w+");
    }
    else
    {
        options.m_hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        options.m_fpOutput = stdout;
    }

    if (options.m_bCompare)
    {
        return (CompareHashes(options) == 0 ? 0 : 1);
    }

    if (options.m_strFileList.length() > 0)
    {
        FILE *file = NULL;
        if (_tfopen_s(&file, options.m_strFileList.c_str(), _T("rt")) != 0)
        {
            _ftprintf(stderr, _T("Error:  Could not open input file list \"%s\" - Error %d\n"), options.m_strFileList.c_str(), GetLastError());
            exit(1);
        }

        TCHAR szLine[16384];
        TString strBaseDirectory = options.m_strRelativeRoot.length() > 0 ? options.m_strRelativeRoot : _T("");
        TString strCurrentDir;

        while (_fgetts(szLine, 16384, file) != NULL)
        {
            size_t len = _tcslen(szLine);
            if (len == 0)
                continue;

            TCHAR *szPath = szLine;

            TCHAR *szSum = _tcsstr(szLine, _T(" *"));
            if (szSum != NULL)
            {
                szPath = szSum + 2;
                len = _tcslen(szPath);
            }

            while(len > 0 && (szPath[len-1] == _T('\n') || szPath[len-1] == _T('\r')))
            {
                szPath[len-1] = _T('\0');
                len--;
            }

            WIN32_FIND_DATA ffd;
            HANDLE hFind = FindFirstFile(szPath, &ffd);
            if (hFind != INVALID_HANDLE_VALUE)
            {
                do
                {
                    strCurrentDir = PathCombineIfRelative(g_strCurrentDir, GetDirectory(szPath));
                    ProcessFile(options, strCurrentDir, strBaseDirectory, ffd);
                } while (FindNextFile(hFind, &ffd));
                FindClose(hFind);
            }
        }

        fclose(file);
    }
    else
    {
        RecurseDirList(options, options.m_strBaseDirectory == _T("") ? _T(".") : options.m_strBaseDirectory, options.m_strBaseDirectory, true);
    }

    if (options.m_bSort)
    {
        // Display sorted data
        if (options.m_bSortInt)
        {
            I642STRING::iterator it;
            if (options.m_bSortReverse)
            {
                it = g_mapSortI64.end();
                while (it != g_mapSortI64.begin())
                {
                    it--;
                    _ftprintf(options.m_fpOutput, _T("%s\n"), (*it).second.c_str());
                }
            } else
            {
                it = g_mapSortI64.begin();
                while (it != g_mapSortI64.end())
                {
                    _ftprintf(options.m_fpOutput, _T("%s\n"), (*it).second.c_str());
                        it++;
                }
            }
        } else
        {
            STRING2STRING::iterator it;
            if (options.m_bSortReverse)
            {
                it = g_mapSortStr.end();
                while (it != g_mapSortStr.begin())
                {
                    it--;
                    _ftprintf(options.m_fpOutput, _T("%s\n"), (*it).second.c_str());
                }
            } else
            {
                it = g_mapSortStr.begin();
                while (it != g_mapSortStr.end())
                {
                    _ftprintf(options.m_fpOutput, _T("%s\n"), (*it).second.c_str());
                        it++;
                }
            }
        }
    }

    if (options.m_strOutputFile != _T(""))
        fclose(options.m_fpOutput);

	return 0;
}

