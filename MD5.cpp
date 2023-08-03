#include "stdafx.h"

/* MD5C.C - RSA Data Security, Inc., MD5 message-digest algorithm
 */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */

#include "md5.h"

/* F, G, H and I are basic MD5 functions.
 */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits.
 */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
Rotation is separate from addition to prevent recomputation.
 */
#define FF(a, b, c, d, x, s, ac) { \
 (a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) { \
 (a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) { \
 (a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) { \
 (a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }

CMD5::CMD5()
{
	memset(m_Padding,0x00,64);
	m_Padding[0] = 0x80;
}

CMD5::~CMD5()
{
}

/* MD5 initialization. Begins an MD5 operation, writing a new context.
 */
void CMD5::MD5Init()
{
  m_context.count[0] = m_context.count[1] = 0;
  /* Load magic initialization constants.
*/
  m_context.state[0] = 0x67452301;
  m_context.state[1] = 0xefcdab89;
  m_context.state[2] = 0x98badcfe;
  m_context.state[3] = 0x10325476;
}

/* MD5 block update operation. Continues an MD5 message-digest
  operation, processing another message block, and updating the
  context.
 */
void CMD5::MD5Update (
unsigned char *input,                                /* input block */
unsigned int inputLen                     /* length of input block */
)
{
  unsigned int i, index, partLen;

  /* Compute number of bytes mod 64 */
  index = (unsigned int)((m_context.count[0] >> 3) & 0x3F);

  /* Update number of bits */
  if ((m_context.count[0] += ((UINT4)inputLen << 3))
   < ((UINT4)inputLen << 3))
 m_context.count[1]++;
  m_context.count[1] += ((UINT4)inputLen >> 29);

  partLen = 64 - index;

  /* Transform as many times as possible.
*/
  if (inputLen >= partLen) {
 memcpy
   ((POINTER)&(m_context.buffer[index]), (POINTER)input, partLen);
 MD5Transform (m_context.state, m_context.buffer);

 for (i = partLen; i + 63 < inputLen; i += 64)
   MD5Transform (m_context.state, &input[i]);

 index = 0;
  }
  else
 i = 0;

  /* Buffer remaining input */
  memcpy
 ((POINTER)&(m_context.buffer[index]), (POINTER)&(input[i]),
  inputLen-i);
}

/* MD5 finalization. Ends an MD5 message-digest operation, writing the
  the message digest and zeroizing the context.
 */
void CMD5::MD5Final (
unsigned char digest[16]                         /* message digest */
)
{
  unsigned char bits[8];
  unsigned int index, padLen;

  /* Save number of bits */
  Encode (bits, m_context.count, 8);

  /* Pad out to 56 mod 64.
*/
  index = (unsigned int)((m_context.count[0] >> 3) & 0x3f);
  padLen = (index < 56) ? (56 - index) : (120 - index);
  MD5Update (m_Padding, padLen);

  /* Append length (before m_Padding) */
  MD5Update (bits, 8);

  /* Store state in digest */
  Encode (digest, m_context.state, 16);

  /* Zeroize sensitive information.
*/
  memset ((POINTER) &m_context, 0, sizeof (m_context));
}

/* MD5 basic transformation. Transforms state based on block.
 */
void CMD5::MD5Transform (
UINT4 state[4],
unsigned char block[64]
)
{
  UINT4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];

  Decode (x, block, 64);

  /* Round 1 */
  FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
  FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
  FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
  FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
  FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
  FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
  FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
  FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
  FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
  FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
  FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
  FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
  FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
  FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
  FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
  FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

 /* Round 2 */
  GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
  GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
  GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
  GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
  GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
  GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
  GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
  GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
  GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
  GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
  GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
  GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
  GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
  GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
  GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
  GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

  /* Round 3 */
  HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
  HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
  HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
  HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
  HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
  HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
  HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
  HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
  HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
  HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
  HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
  HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
  HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
  HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
  HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
  HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

  /* Round 4 */
  II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
  II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
  II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
  II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
  II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
  II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
  II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
  II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
  II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
  II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
  II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
  II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
  II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
  II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
  II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
  II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;

  /* Zeroize sensitive information.

*/
  memset ((POINTER)x, 0, sizeof (x));
}

/* Encodes input (UINT4) into output (unsigned char). Assumes len is
  a multiple of 4.
 */
void CMD5::Encode (
unsigned char *output,
UINT4 *input,
unsigned int len
)
{
  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4) {
 output[j] = (unsigned char)(input[i] & 0xff);
 output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
 output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
 output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
  }
}

/* Decodes input (unsigned char) into output (UINT4). Assumes len is
  a multiple of 4.
 */
void CMD5::Decode (
UINT4 *output,
unsigned char *input,
unsigned int len
)
{
  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4)
 output[i] = ((UINT4)input[j]) | (((UINT4)input[j+1]) << 8) |
   (((UINT4)input[j+2]) << 16) | (((UINT4)input[j+3]) << 24);
}


CMD5_CTX CMD5::GetContext()
{
	return m_context;
}

void CMD5::SetContext(CMD5_CTX context)
{
	m_context = context;
}

CMD5_CTX::CMD5_CTX(CMD5_CTX &copy)
{
	memcpy(state,copy.state,4 * sizeof(UINT4));
	memcpy(count,copy.count,2 * sizeof(UINT4));
	memcpy(buffer,copy.buffer,64 * sizeof(unsigned char));
}

int CMD5_CTX::operator=(const CMD5_CTX &copy)
{
	memcpy(state,copy.state,4 * sizeof(UINT4));
	memcpy(count,copy.count,2 * sizeof(UINT4));
	memcpy(buffer,copy.buffer,64 * sizeof(unsigned char));
    return 0;
}

CMD5_CTX::CMD5_CTX()
{
	memset(state,0x00,4 * sizeof(UINT4));
	memset(count,0x00,2 * sizeof(UINT4));
	memset(buffer,0x00,64 * sizeof(unsigned char));
}

CMD5_CTX::~CMD5_CTX()
{
}

__int64 CMD5::FileMD5(LPCTSTR szFilename, BYTE *pbMD5, DWORD *pdwErr, __int64 iPartialChunks, TString *strPartialHashes)
{
	// Generate an MD-5 for a particular file
	HANDLE hFile;
	hFile = CreateFile(szFilename,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return -1;

    __int64 iTotalSize = 0;
    __int64 iMB = iPartialChunks * 1024 * 1024;

	DWORD dwBytesRead = 0;

	MD5Init();

	while (ReadFile(hFile,m_pbBuffer,MD5_BUFFERSIZE,&dwBytesRead,NULL))
	{
		if (dwBytesRead == 0)
			break;

        iTotalSize += dwBytesRead;

		MD5Update(m_pbBuffer,dwBytesRead);

		if (dwBytesRead != MD5_BUFFERSIZE)
			break;

        if (strPartialHashes != NULL && iPartialChunks > 0)
        {
            if (iTotalSize % iMB == 0)
            {
                UINT iPartial = GetContext().state[0];
                TCHAR sz[64];
                TCHAR szPartial[64];
                _stprintf_s(szPartial, 64, _T("%8.8x"), iPartial);
                //for (int i = 0; i < 4; i++)
                //    _sntprintf_s(&szPartial[2*i], 32 - (2*i) + 1, 3, _T("%.2x"), iPartial);

                _stprintf_s(sz, 64, _T("%s  %d MB\n"), szPartial, iTotalSize / (1024*1024));
                strPartialHashes->append(sz);
            }
        }
	}

    if (pdwErr != NULL)
        *pdwErr = GetLastError();

	MD5Final(pbMD5);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	return iTotalSize;
}

BOOL CMD5::FileInit (LPCTSTR szFilename)
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

	MD5Init();

	return TRUE;
}

BOOL CMD5::FileContinue (float *fPercent)
{
	*fPercent = 0.0;

	if (m_hProgressFile == INVALID_HANDLE_VALUE)
		return FALSE;

	DWORD dwBytesRead = 0;

	ReadFile(m_hProgressFile,m_pbBuffer,MD5_BUFFERSIZE,&dwBytesRead,NULL);

	if (dwBytesRead == 0)
		return FALSE;

	MD5Update(m_pbBuffer,dwBytesRead);

	m_liBytesRead += dwBytesRead;

	*fPercent = m_liBytesRead / (float) m_liFileSize;

	if (dwBytesRead != MD5_BUFFERSIZE)
		return FALSE;

	return TRUE;
}

BOOL CMD5::FileComplete (BYTE *pbMD5)
{
	MD5Final(pbMD5);

	CloseHandle(m_hProgressFile);

	return TRUE;
}
