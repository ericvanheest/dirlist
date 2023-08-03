// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__FD4F0A53_EE7E_4B50_AA26_2A6BF9BC9D55__INCLUDED_)
#define AFX_STDAFX_H__FD4F0A53_EE7E_4B50_AA26_2A6BF9BC9D55__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _UNICODE
#define UNICODE

#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <tchar.h>

// TODO: reference additional headers your program requires here
#if defined( UNICODE ) || defined( _UNICODE )
typedef std::wstring  TString;
#else
typedef std::string   TString;
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__FD4F0A53_EE7E_4B50_AA26_2A6BF9BC9D55__INCLUDED_)
