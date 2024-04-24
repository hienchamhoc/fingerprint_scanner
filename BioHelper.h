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

#pragma once

namespace BioHelper
{

    typedef struct _POOL_CONFIGURATION {
        ULONG ConfigurationFlags;
        ULONG DatabaseAttributes;
        WINBIO_UUID DatabaseId;
        WINBIO_UUID DataFormat;
        WCHAR SensorAdapter[MAX_PATH];
        WCHAR EngineAdapter[MAX_PATH];
        WCHAR StorageAdapter[MAX_PATH];
    } POOL_CONFIGURATION, * PPOOL_CONFIGURATION;

    HRESULT
        CreateCompatibleConfiguration(
            __in WINBIO_UNIT_SCHEMA* UnitSchema,
            __out POOL_CONFIGURATION* Configuration
        );

    HRESULT
        RegisterDatabase(
            __in WINBIO_STORAGE_SCHEMA* StorageSchema
        );

    HRESULT
        UnregisterDatabase(
            __in WINBIO_UUID* DatabaseId
        );

    HRESULT
        RegisterPrivateConfiguration(
            __in WINBIO_UNIT_SCHEMA* UnitSchema,
            __in POOL_CONFIGURATION* Configuration
        );

    HRESULT
        UnregisterPrivateConfiguration(
            __in WINBIO_UNIT_SCHEMA* UnitSchema,
            __in WINBIO_UUID* DatabaseId,
            __out bool* ConfigurationRemoved
        );

    //
    // Display routines...
    //
    // Caller must release returned message 
    // buffer with LocalFree()
    LPTSTR
        ConvertErrorCodeToString(
            __in HRESULT ErrorCode
        );

    LPCTSTR
        ConvertSubFactorToString(
            __in WINBIO_BIOMETRIC_SUBTYPE SubFactor
        );

    LPCTSTR
        ConvertRejectDetailToString(
            __in WINBIO_REJECT_DETAIL RejectDetail
        );

}; // namespace BioHelper