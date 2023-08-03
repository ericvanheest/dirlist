#include <wtypes.h>

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

/* UINT2 defines a two byte word */
typedef unsigned short int UINT2;

/* UINT4 defines a four byte word */
typedef unsigned long int UINT4;

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

/* Constants for MD5Transform routine.
 */

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

#define MD5_BUFFERSIZE 65536

/* MD5 context. */
class CMD5_CTX
{
public:
	CMD5_CTX();
	~CMD5_CTX();

	CMD5_CTX(CMD5_CTX &copy);
	int operator=(const CMD5_CTX &copy);

  UINT4 state[4];                                   /* state (ABCD) */
  UINT4 count[2];        /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                         /* input buffer */
};

class CMD5
{
public:
	CMD5();
	~CMD5();

	CMD5_CTX GetContext();
	void SetContext(CMD5_CTX context);
	void MD5Init ();
	void MD5Update (unsigned char *, unsigned int);
	void MD5Final (unsigned char [16]);

	void MD5Transform (UINT4 [4], unsigned char [64]);
	void Encode (unsigned char *, UINT4 *, unsigned int);
	void Decode (UINT4 *, unsigned char *, unsigned int);
	void MD5_memcpy (POINTER, POINTER, unsigned int);
	void MD5_memset (POINTER, int, unsigned int);

	__int64 FileMD5 (LPCTSTR szFilename, BYTE *pbMD5, DWORD *pdwErr, __int64 iPartialChunks = 0, TString *strPartialHashes = NULL);

	HANDLE m_hProgressFile;
	__int64 m_liFileSize;
	__int64 m_liBytesRead;

	BOOL FileInit (LPCTSTR szFilename);
	BOOL FileContinue (float *fPercent);
	BOOL FileComplete (BYTE *pbMD5);

	BYTE m_pbBuffer[MD5_BUFFERSIZE];

public:
	unsigned char m_Padding[64];
	CMD5_CTX m_context;
};