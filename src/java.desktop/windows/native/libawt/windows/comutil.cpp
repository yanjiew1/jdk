/*
 * PROJECT:     ReactOS crt library
 * LICENSE:     BSD-2-Clause (https://spdx.org/licenses/BSD-2-Clause)
 * PURPOSE:     Compiler support for COM
 * COPYRIGHT:   Copyright 2013 Thomas Faber (thomas.faber@reactos.org)
 *              Copyright 2015 Hermès Bélusca-Maïto (hermes.belusca-maito@reactos.org)
 */

#include <comutil.h>

// MinGW do not provide ConvertStringToBSTR and ConvertBSTRToString
// implementations, so we provide our own here.
// The implementations are come from the ReactOS project.
#ifdef __MINGW32__
namespace _com_util
{

BSTR WINAPI ConvertStringToBSTR(const char *pSrc)
{
    DWORD cwch;
    BSTR wsOut(NULL);

    if (!pSrc) return NULL;

    /* Compute the needed size with the NULL terminator */
    cwch = ::MultiByteToWideChar(CP_ACP, 0, pSrc, -1, NULL, 0);
    if (cwch == 0) return NULL;

    /* Allocate the BSTR (without the NULL terminator) */
    wsOut = ::SysAllocStringLen(NULL, cwch - 1);
    if (!wsOut)
    {
        ::_com_issue_error(HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY));
        return NULL;
    }

    /* Convert the string */
    if (::MultiByteToWideChar(CP_ACP, 0, pSrc, -1, wsOut, cwch) == 0)
    {
        /* We failed, clean everything up */
        cwch = ::GetLastError();

        ::SysFreeString(wsOut);
        wsOut = NULL;

        ::_com_issue_error(!IS_ERROR(cwch) ? HRESULT_FROM_WIN32(cwch) : cwch);
    }

    return wsOut;
}

char* WINAPI ConvertBSTRToString(BSTR pSrc)
{
    DWORD cb, cwch;
    char *szOut = NULL;

    if (!pSrc) return NULL;

    /* Retrieve the size of the BSTR with the NULL terminator */
    cwch = ::SysStringLen(pSrc) + 1;

    /* Compute the needed size with the NULL terminator */
    cb = ::WideCharToMultiByte(CP_ACP, 0, pSrc, cwch, NULL, 0, NULL, NULL);
    if (cb == 0)
    {
        cwch = ::GetLastError();
        ::_com_issue_error(!IS_ERROR(cwch) ? HRESULT_FROM_WIN32(cwch) : cwch);
        return NULL;
    }

    /* Allocate the string */
    szOut = (char*)::operator new(cb * sizeof(char));
    if (!szOut)
    {
        ::_com_issue_error(HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY));
        return NULL;
    }

    /* Convert the string and NULL-terminate */
    szOut[cb - 1] = '\0';
    if (::WideCharToMultiByte(CP_ACP, 0, pSrc, cwch, szOut, cb, NULL, NULL) == 0)
    {
        /* We failed, clean everything up */
        cwch = ::GetLastError();

        ::operator delete(szOut);
        szOut = NULL;

        ::_com_issue_error(!IS_ERROR(cwch) ? HRESULT_FROM_WIN32(cwch) : cwch);
    }

    return szOut;
}

}
#endif
