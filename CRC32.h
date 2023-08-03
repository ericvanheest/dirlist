// CRC32.h: interface for the CCRC32 class.
//
//////////////////////////////////////////////////////////////////////

#include <wtypes.h>

#if !defined(AFX_BITSTREAM_H__59AF8143_330E_11D4_B4D0_00D0B74AADD5__INCLUDED_)
#define AFX_BITSTREAM_H__59AF8143_330E_11D4_B4D0_00D0B74AADD5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define BS_BITS_PER_BYTE 8
#define CRC32_BUFFERSIZE	65536

class CCRC32
{
public:
	DWORD Get_CRC(BYTE *pbData, DWORD dwSize);
	void BeginCRC(DWORD dwSeed = 0xffffffff);
	DWORD ContinueCRC(BYTE *pbData, DWORD dwSize);
	DWORD GetCurrentCRC();
	ULONG Reflect(ULONG ref, char ch);
    BOOL Error();

	DWORD FileCRC(LPCTSTR szFilename);
	CCRC32();
	virtual ~CCRC32();

	HANDLE m_hProgressFile;
	__int64 m_liFileSize;
	__int64 m_liBytesRead;
    BOOL m_bError;

	BOOL FileInit (LPCTSTR szFilename);
	BOOL FileContinue (float *fPercent);
	BOOL FileComplete (DWORD *dwCRC);

	BYTE m_pbBuffer[CRC32_BUFFERSIZE];

private:
	ULONG m_crc32_table[256]; // Lookup table array
	ULONG m_dwCRC;
};

#endif // !defined(AFX_BITSTREAM_H__59AF8143_330E_11D4_B4D0_00D0B74AADD5__INCLUDED_)
