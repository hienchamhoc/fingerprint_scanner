/******************************************************************************
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) Microsoft.  All rights reserved.

This source code is only intended as a supplement to Microsoft Development
Tools and/or WinHelp documentation.  See these sources for detailed information
regarding the Microsoft samples programs.
******************************************************************************/

/*
    Output formatting routines...
*/
#include "stdafx.h"


typedef struct _SUBFACTOR_TEXT {
    WINBIO_BIOMETRIC_SUBTYPE SubFactor;
    LPCTSTR Text;
} SUBFACTOR_TEXT, * PSUBFACTOR_TEXT;

static const SUBFACTOR_TEXT g_SubFactorText[] = {
    {WINBIO_SUBTYPE_NO_INFORMATION,             _T("(No information)")},
    {WINBIO_ANSI_381_POS_RH_THUMB,              _T("RH thumb")},
    {WINBIO_ANSI_381_POS_RH_INDEX_FINGER,       _T("RH index finger")},
    {WINBIO_ANSI_381_POS_RH_MIDDLE_FINGER,      _T("RH middle finger")},
    {WINBIO_ANSI_381_POS_RH_RING_FINGER,        _T("RH ring finger")},
    {WINBIO_ANSI_381_POS_RH_LITTLE_FINGER,      _T("RH little finger")},
    {WINBIO_ANSI_381_POS_LH_THUMB,              _T("LH thumb")},
    {WINBIO_ANSI_381_POS_LH_INDEX_FINGER,       _T("LH index finger")},
    {WINBIO_ANSI_381_POS_LH_MIDDLE_FINGER,      _T("LH middle finger")},
    {WINBIO_ANSI_381_POS_LH_RING_FINGER,        _T("LH ring finger")},
    {WINBIO_ANSI_381_POS_LH_LITTLE_FINGER,      _T("LH little finger")},
    {WINBIO_SUBTYPE_ANY,                        _T("Any finger")},
};
static const SIZE_T k_SubFactorTextTableSize = sizeof(g_SubFactorText) / sizeof(SUBFACTOR_TEXT);


typedef struct _REJECT_DETAIL_TEXT {
    WINBIO_REJECT_DETAIL RejectDetail;
    LPCTSTR Text;
} REJECT_DETAIL_TEXT, * PREJECT_DETAIL_TEXT;

static const REJECT_DETAIL_TEXT g_RejectDetailText[] = {
    {WINBIO_FP_TOO_HIGH,        _T("Scan your fingerprint a little lower.")},
    {WINBIO_FP_TOO_LOW,         _T("Scan your fingerprint a little higher.")},
    {WINBIO_FP_TOO_LEFT,        _T("Scan your fingerprint more to the right.")},
    {WINBIO_FP_TOO_RIGHT,       _T("Scan your fingerprint more to the left.")},
    {WINBIO_FP_TOO_FAST,        _T("Scan your fingerprint more slowly.")},
    {WINBIO_FP_TOO_SLOW,        _T("Scan your fingerprint more quickly.")},
    {WINBIO_FP_POOR_QUALITY,    _T("The quality of the fingerprint scan was not sufficient to make a match.  Check to make sure the sensor is clean.")},
    {WINBIO_FP_TOO_SKEWED,      _T("Hold your finger flat and straight when scanning your fingerprint.")},
    {WINBIO_FP_TOO_SHORT,       _T("Use a longer stroke when scanning your fingerprint.")},
    {WINBIO_FP_MERGE_FAILURE,   _T("Unable to merge samples into a single enrollment. Try to repeat the enrollment procedure from the beginning.")},
};
static const SIZE_T k_RejectDetailTextTableSize = sizeof(g_RejectDetailText) / sizeof(REJECT_DETAIL_TEXT);


namespace BioHelper
{

    LPTSTR
        ConvertErrorCodeToString(
            __in HRESULT ErrorCode
        )
    {
        TCHAR* messageBuffer = NULL;
        DWORD messageLength = 0;

        std::vector<TCHAR> systemPath;
        UINT systemPathSize = 0;
        systemPathSize = GetSystemWindowsDirectory(NULL, 0);
        systemPath.resize(systemPathSize);
        systemPathSize = GetSystemWindowsDirectory((LPTSTR)&systemPath[0], systemPathSize);

        TSTRING libraryPath = &systemPath[0];
        libraryPath += _T("\\system32\\winbio.dll");

        HMODULE winbioLibrary = NULL;
        winbioLibrary = LoadLibraryEx(
            libraryPath.c_str(),
            NULL,
            LOAD_LIBRARY_AS_DATAFILE |
            LOAD_LIBRARY_AS_IMAGE_RESOURCE
        );
        if (winbioLibrary != NULL)
        {
            messageLength = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_HMODULE |
                FORMAT_MESSAGE_FROM_SYSTEM,
                winbioLibrary,
                ErrorCode,
                0,                      // LANGID
                (LPTSTR)&messageBuffer,
                0,                      // arg count
                NULL                    // arg array
            );
            if (messageLength > 0)
            {
                // success
                messageBuffer[messageLength] = _T('\0');
            }
            FreeLibrary(winbioLibrary);
            winbioLibrary = NULL;
        }

        if (messageBuffer == NULL)
        {
            messageLength = 11;    // "0x" + "%08x"
            messageBuffer = (TCHAR*)LocalAlloc(LPTR, (messageLength + 1) * sizeof(TCHAR));
            if (messageBuffer != NULL)
            {
                _stprintf_s(messageBuffer, messageLength, _T("0x%08x"), ErrorCode);
            }
        }

        // Caller must release buffer with LocalFree()
        return messageBuffer;
    }

    //-----------------------------------------------------------------------------

    LPCTSTR
        ConvertSubFactorToString(
            __in WINBIO_BIOMETRIC_SUBTYPE SubFactor
        )
    {
        SIZE_T index = 0;
        for (index = 0; index < k_SubFactorTextTableSize; ++index)
        {

            if (g_SubFactorText[index].SubFactor == SubFactor)
            {
                return g_SubFactorText[index].Text;
            }
        }
        return _T("<Unknown>");
    }

    //-----------------------------------------------------------------------------

    LPCTSTR
        ConvertRejectDetailToString(
            __in WINBIO_REJECT_DETAIL RejectDetail
        )
    {
        SIZE_T index = 0;
        for (index = 0; index < k_RejectDetailTextTableSize; ++index)
        {
            if (g_RejectDetailText[index].RejectDetail == RejectDetail)
            {
                return g_RejectDetailText[index].Text;
            }
        }
        return _T("Reason for failure couldn't be diagnosed.");
    }

    //-----------------------------------------------------------------------------


}; // namespace BioHelper