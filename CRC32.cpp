// CRC32.cpp: implementation of the CCRC32 class.
// Portions of this code were taken from CRCFile, which is:
//		Copyright © 2000 Richard A. Ellingson
//		http://www.createwindow.com
//		mailto:relling@hughes.net
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CRC32.h"
#include "Globals.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCRC32::CCRC32()
{
	// This is the official polynomial used by CRC-32 
	// in PKZip, WinZip and Ethernet. 
	ULONG ulPolynomial = 0x04c11db7;

	// 256 values representing ASCII character codes.
	for(int i = 0; i <= 0xFF; i++)
	{
		m_crc32_table[i]=Reflect(i, 8) << 24;
		for (int j = 0; j < 8; j++)
			m_crc32_table[i] = (m_crc32_table[i] << 1) ^ (m_crc32_table[i] & (1 << 31) ? ulPolynomial : 0);
		m_crc32_table[i] = Reflect(m_crc32_table[i], 32);
	}
}

CCRC32::~CCRC32()
{
}

// Reflection is a requirement for the official CRC-32 standard.
// You can create CRCs without it, but they won't conform to the standard.
ULONG CCRC32::Reflect(ULONG ref, char ch)
{// Used only by Init_m_crc32_table()

	ULONG value(0);

	// Swap bit 0 for bit 7
	// bit 1 for bit 6, etc.
	for(int i = 1; i < (ch + 1); i++)
	{
		if(ref & 1)
			value |= 1 << (ch - i);
		ref >>= 1;
	}
	return value;
}

// This function uses the m_crc32_table lookup table
// to generate a CRC for pbData
DWORD CCRC32::Get_CRC(BYTE *pbData, DWORD dwSize)
{
	// Be sure to use unsigned variables,
	// because negative values introduce high bits
	// where zero bits are required.
	ULONG  crc(0xffffffff);

	// Save the text in the buffer.
	// Perform the algorithm on each character
	// in the string, using the lookup table values.
	while(dwSize--)
		crc = (crc >> 8) ^ m_crc32_table[(crc & 0xFF) ^ *pbData++];
	// Exclusive OR the result with the beginning value.
	return crc^0xffffffff;
}

void CCRC32::BeginCRC(DWORD dwSeed)
{
	m_dwCRC = dwSeed;
}

DWORD CCRC32::ContinueCRC(BYTE *pbData, DWORD dwSize)
{
	while(dwSize--)
		m_dwCRC = (m_dwCRC >> 8) ^ m_crc32_table[(m_dwCRC & 0xFF) ^ *pbData++];

	return m_dwCRC; // ^0xffffffff;
}

DWORD CCRC32::GetCurrentCRC()
{
	return m_dwCRC^0xffffffff;
}

BOOL CCRC32::Error()
{
    return m_bError;
}

DWORD CCRC32::FileCRC(LPCTSTR szFilename)
{
    m_bError = FALSE;

	// Generate a CRC-32 for a particular file
	HANDLE hFile;
	hFile = CreateFile(szFilename,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
	if (hFile == INVALID_HANDLE_VALUE)
    {
        m_bError = TRUE;
		return 0x00000000;
    }

	BYTE pbBuffer[65536];
	DWORD dwBytesRead = 0;

	BeginCRC();

    __int64 iTotal = 0;

	while (ReadFile(hFile,pbBuffer,65536,&dwBytesRead,NULL))
	{
		if (dwBytesRead == 0)
			break;

        iTotal += dwBytesRead;

		ContinueCRC(pbBuffer,dwBytesRead);

		if (dwBytesRead != 65536)
			break;
	}

    if (iTotal != GetFileSize64(szFilename))
    {
        _ftprintf(stderr, _T("Error %d reading file: %s\n"), GetLastError(), szFilename);
    }

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	return GetCurrentCRC();
}

BOOL CCRC32::FileInit (LPCTSTR szFilename)
{
	m_hProgressFile = CreateFile(szFilename,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
	if (m_hProgressFile == INVALID_HANDLE_VALUE)
		return FALSE;

	DWORD dwHigh;
	LARGE_INTEGER liSize;

	liSize.LowPart = GetFileSize(m_hProgressFile,&dwHigh);
	liSize.HighPart = dwHigh;

	m_liFileSize = liSize.QuadPart;
	m_liBytesRead = 0;

	BeginCRC();

	return TRUE;
}

BOOL CCRC32::FileContinue (float *fPercent)
{
	*fPercent = 0.0;

	if (m_hProgressFile == INVALID_HANDLE_VALUE)
		return FALSE;

	DWORD dwBytesRead = 0;

	ReadFile(m_hProgressFile,m_pbBuffer,CRC32_BUFFERSIZE,&dwBytesRead,NULL);

	if (dwBytesRead == 0)
		return FALSE;

	ContinueCRC(m_pbBuffer,dwBytesRead);

	m_liBytesRead += dwBytesRead;

	*fPercent = m_liBytesRead / (float) m_liFileSize;

	if (dwBytesRead != CRC32_BUFFERSIZE)
		return FALSE;

	return TRUE;
}

BOOL CCRC32::FileComplete (DWORD *dwCRC)
{
	*dwCRC = GetCurrentCRC();

	CloseHandle(m_hProgressFile);

	return TRUE;
}
