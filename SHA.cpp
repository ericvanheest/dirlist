#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include "Globals.h"
#include "SHA.h"

CSHA::CSHA()
{
    m_hProvider = NULL;
    m_bError = false;

    if (!CryptAcquireContext(&m_hProvider, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
        CryptAcquireContext(&m_hProvider, NULL, NULL, PROV_DSS, CRYPT_VERIFYCONTEXT);
}

CSHA::~CSHA()
{
    CryptReleaseContext(m_hProvider, 0);
}

TString CSHA::GetSHAString(TString sFile, DWORD dwBits)
{
    m_hCryptHash = NULL;
    m_bError = FALSE;
    ALG_ID algid = CALG_SHA1;
    const TCHAR * szBlank;

    switch(dwBits)
    {
        case 256:
            algid = CALG_SHA_256;
            szBlank = _T("                                                                                ");
            break;
        default:
            algid = CALG_SHA1;
            szBlank = _T("                                        ");
            break;
    }

	// Generate a SHA for a particular file
	HANDLE hFile;
	hFile = CreateFile(sFile.c_str(),GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
	if (hFile == INVALID_HANDLE_VALUE)
    {
        m_bError = TRUE;
		return szBlank;
    }

	BYTE pbBuffer[65536];
	DWORD dwBytesRead = 0;

    CryptCreateHash(m_hProvider, algid, 0, 0, &m_hCryptHash);

    if (m_hCryptHash == NULL)
    {
        CloseHandle(hFile);
        return szBlank;
    }

    __int64 iTotal = 0;

    DWORD dwErr = NOERROR;

	while (ReadFile(hFile,pbBuffer,65536,&dwBytesRead,NULL))
	{
		if (dwBytesRead == 0)
			break;

        iTotal += dwBytesRead;

        CryptHashData(m_hCryptHash, pbBuffer, dwBytesRead, 0);

		if (dwBytesRead != 65536)
			break;
	}

    dwErr = GetLastError();

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

    BYTE pbHash[64];
    DWORD dwDataSize = 64;

    CryptGetHashParam(m_hCryptHash, HP_HASHVAL, pbHash, &dwDataSize, 0);

    TCHAR szHex[3];

    TString sHash;

    if (iTotal != GetFileSize64(sFile))
    {
        TCHAR sz[64];
        _sntprintf_s(sz, 64, _T("<err: %5d %27I64d>"), dwErr, iTotal);
        sHash = sz;
        for(int iExtra = 0; iExtra < ((int) dwBits / 4 - 40); iExtra++)
            sHash.append(_T(" "));
        _ftprintf(stderr, _T("Error %d reading file: %s\n"), dwErr, sFile.c_str());
    }
    else
    {
        for(DWORD i = 0; i < dwDataSize; i++)
        {
            _sntprintf_s(szHex, 4, _T("%2.2x"), pbHash[i]);
            sHash.append(szHex);
        }
    }

    CryptDestroyHash(m_hCryptHash);
    return sHash;
}

