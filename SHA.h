#include <string>
#include <windows.h>
#include <wincrypt.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSHA
{
public:
    CSHA();
    ~CSHA();

    TString GetSHAString(TString sFile, DWORD dwBits = 128);
    bool m_bError;

private:
    HCRYPTPROV m_hProvider;
    HCRYPTHASH m_hCryptHash;
};
