/*
 * Author: Tomáš Růžička, t_ruzicka (at) email.cz
 * 2015
 */

#include "headers.h"
#include "PrivatePoolSetup.cpp"
#include "PrivatePoolIdentify.cpp"
#include "PrivatePoolEnroll.cpp"

void BmpSetImageData(SBmpImage *bmp, const std::vector<uint8> &data, uint32 width, uint32 height)
{
  bmp->data.resize(data.size());

  for(uint32 y = 0; y < height; y++)
  {
    const uint32 yy = (height - y - 1) * width;

    for(uint32 x = 0; x < width; x++)
    {
      const uint32 xy = yy + x;

      bmp->data[xy].r = data[y * width + x];
      bmp->data[xy].g = data[y * width + x];
      bmp->data[xy].b = data[y * width + x];
      bmp->data[xy].a = 255;
    }
  }

  bmp->dataPaddingSize = sizeof(uint32) - ((width * NBmp::BMP_24B_COLORS) % sizeof(uint32));
  if(bmp->dataPaddingSize == sizeof(uint32))
    bmp->dataPaddingSize = 0;

  bmp->signature = NBmp::BMP_HEADER_SIGNATURE;
  bmp->fileSize = NBmp::BMP_HEADER_FILE_DATA_OFFSET + bmp->colorTable.size() * NBmp::BMP_32B_COLORS + (width * NBmp::BMP_24B_COLORS + bmp->dataPaddingSize) * height;
  bmp->reserved = NBmp::BMP_HEADER_RESERVED;
  bmp->dataOffset = NBmp::BMP_HEADER_FILE_DATA_OFFSET + bmp->colorTable.size() * NBmp::BMP_32B_COLORS;
  bmp->infoHeader.headerSize = NBmp::BMP_INFO_HEADER_SIZE;
  bmp->infoHeader.width = width;
  bmp->infoHeader.height = height;
  bmp->infoHeader.planes = NBmp::BMP_INFO_HEADER_PLANES;
  bmp->infoHeader.bitsPerPixel = NBmp::BMP_INFO_HEADER_BITS_PER_PIXEL_24;
  bmp->infoHeader.compression = NBmp::BMP_INFO_HEADER_COMPRESSION;
  bmp->infoHeader.imageSize = (width * NBmp::BMP_24B_COLORS + bmp->dataPaddingSize) * height;
  bmp->infoHeader.pixelPerMeterX = NBmp::BMP_INFO_HEADER_PIXEL_PER_METER_X;
  bmp->infoHeader.pixelPerMeterY = NBmp::BMP_INFO_HEADER_PIXEL_PER_METER_Y;
  bmp->infoHeader.colors = NBmp::BMP_INFO_HEADER_COLORS;
  bmp->infoHeader.usedColors = NBmp::BMP_INFO_HEADER_USED_COLORS;
}

void BmpSave(const SBmpImage *bmp, std::string filename)
{
  const uint32 dataPadding = 0;

  CFile file(filename);

  file.write(&bmp->signature[0], sizeof(char)* NBmp::BMP_HEADER_SIGNATURE_LENGHT);
  file.write(&bmp->fileSize, sizeof(uint32));
  file.write(&bmp->reserved, sizeof(uint32));
  file.write(&bmp->dataOffset, sizeof(uint32));
  file.write(&bmp->infoHeader.headerSize, sizeof(uint32));
  file.write(&bmp->infoHeader.width, sizeof(uint32));
  file.write(&bmp->infoHeader.height, sizeof(uint32));
  file.write(&bmp->infoHeader.planes, sizeof(uint16));
  file.write(&bmp->infoHeader.bitsPerPixel, sizeof(uint16));
  file.write(&bmp->infoHeader.compression, sizeof(uint32));
  file.write(&bmp->infoHeader.imageSize, sizeof(uint32));
  file.write(&bmp->infoHeader.pixelPerMeterX, sizeof(uint32));
  file.write(&bmp->infoHeader.pixelPerMeterY, sizeof(uint32));
  file.write(&bmp->infoHeader.colors, sizeof(uint32));
  file.write(&bmp->infoHeader.usedColors, sizeof(uint32));

  for(uint32 i = 0; i < bmp->colorTable.size(); i++)
  {
    file.write(&bmp->colorTable[i].r, sizeof(uint8));
    file.write(&bmp->colorTable[i].g, sizeof(uint8));
    file.write(&bmp->colorTable[i].b, sizeof(uint8));
    file.write(&bmp->colorTable[i].a, sizeof(uint8));
  }

  for(uint32 y = 0; y < bmp->infoHeader.height; y++)
  {
    const uint32 yy = (bmp->infoHeader.height - y - 1) * bmp->infoHeader.width;

    for(uint32 x = 0; x < bmp->infoHeader.width; x++)
    {
      const uint32 xy = yy + x;

      file.write(&bmp->data[xy].b, sizeof(uint8));
      file.write(&bmp->data[xy].g, sizeof(uint8));
      file.write(&bmp->data[xy].r, sizeof(uint8));
    }

    file.write(&dataPadding, sizeof(uint8)* bmp->dataPaddingSize);
  }

  file.close();
}

HRESULT CaptureSample()
{
  HRESULT hr = S_OK;
  WINBIO_SESSION_HANDLE sessionHandle = NULL;
  WINBIO_UNIT_ID unitId = 0;
  WINBIO_REJECT_DETAIL rejectDetail = 0;
  PWINBIO_BIR sample = NULL;
  SIZE_T sampleSize = 0;

  // Connect to the system pool. 
  hr = WinBioOpenSession(
    WINBIO_TYPE_FINGERPRINT,    // Service provider
    WINBIO_POOL_SYSTEM,         // Pool type
    WINBIO_FLAG_RAW,            // Access: Capture raw data
    NULL,                       // Array of biometric unit IDs
    0,                          // Count of biometric unit IDs
    WINBIO_DB_DEFAULT,          // Default database
    &sessionHandle              // [out] Session handle
    );

  if(FAILED(hr))
  {
    std::cout << "WinBioOpenSession failed. hr = 0x" << std::hex << hr << std::dec << "\n";

    if(sample != NULL)
    {
      WinBioFree(sample);
      sample = NULL;
    }

    if(sessionHandle != NULL)
    {
      WinBioCloseSession(sessionHandle);
      sessionHandle = NULL;
    }
    
    return hr;
  }

  // Capture a biometric sample.
  std::cout << "Calling WinBioCaptureSample - Swipe sensor...\n";

  hr = WinBioCaptureSample(
    sessionHandle,
    WINBIO_NO_PURPOSE_AVAILABLE,
    WINBIO_DATA_FLAG_RAW,
    &unitId,
    &sample,
    &sampleSize,
    &rejectDetail
    );

  if(FAILED(hr))
  {
    if(hr == WINBIO_E_BAD_CAPTURE)
      std:: cout << "Bad capture; reason: " << rejectDetail << "\n";
    else
      std::cout << "WinBioCaptureSample failed.hr = 0x" << std::hex << hr << std::dec << "\n";

    if(sample != NULL)
    {
      WinBioFree(sample);
      sample = NULL;
    }

    if(sessionHandle != NULL)
    {
      WinBioCloseSession(sessionHandle);
      sessionHandle = NULL;
    }

    return hr;
  }

  std::cout << "Swipe processed - Unit ID: " << unitId << "\n";
  std::cout << "Captured " << sampleSize << " bytes.\n";

  if(sample != NULL)
  {
    PWINBIO_BIR_HEADER BirHeader = (PWINBIO_BIR_HEADER)(((PBYTE)sample) + sample->HeaderBlock.Offset);
    PWINBIO_BDB_ANSI_381_HEADER AnsiBdbHeader = (PWINBIO_BDB_ANSI_381_HEADER)(((PBYTE)sample) + sample->StandardDataBlock.Offset);
    PWINBIO_BDB_ANSI_381_RECORD AnsiBdbRecord = (PWINBIO_BDB_ANSI_381_RECORD)(((PBYTE)AnsiBdbHeader) + sizeof(WINBIO_BDB_ANSI_381_HEADER));

    DWORD width = AnsiBdbRecord->HorizontalLineLength; // Width of image in pixels
    DWORD height = AnsiBdbRecord->VerticalLineLength; // Height of image in pixels

    std::cout << "Image resolution: " << width << " x " << height << "\n";

    PBYTE firstPixel = (PBYTE)((PBYTE)AnsiBdbRecord) + sizeof(WINBIO_BDB_ANSI_381_RECORD);

    SBmpImage bmp;
    std::vector<uint8> data(width * height);
    memcpy(&data[0], firstPixel, width * height);

    SYSTEMTIME st;
    GetSystemTime(&st);
    std::stringstream s;
    s << st.wYear << "." << st.wMonth << "." << st.wDay << "." << st.wHour << "." << st.wMinute << "." << st.wSecond << "." << st.wMilliseconds;
    std::string bmpFile = "data/fingerPrint_"+s.str()+".bmp";

    BmpSetImageData(&bmp, data, width, height);
    BmpSave(&bmp, bmpFile);
    //ShellExecuteA(NULL, NULL, bmpFile.c_str(), NULL, NULL, SW_SHOWNORMAL);

    CFile raw("rawData.bin");
    raw.write(&data[0], data.size());
    raw.close();

    WinBioFree(sample);
    sample = NULL;
  }

  if(sessionHandle != NULL)
  {
    WinBioCloseSession(sessionHandle);
    sessionHandle = NULL;
  }

  return hr;
}

HRESULT EnrollSysPool(
    BOOL discardEnrollment,
    WINBIO_BIOMETRIC_SUBTYPE subFactor
   // WINBIO_IDENTITY identity
)
{
    HRESULT hr = S_OK;
     WINBIO_IDENTITY identity = { 0 };
    WINBIO_SESSION_HANDLE sessionHandle = NULL;
    WINBIO_UNIT_ID unitId = 0;
    WINBIO_REJECT_DETAIL rejectDetail = 0;
    BOOLEAN isNewTemplate = TRUE;

    // Connect to the system pool. 
    hr = WinBioOpenSession(
        WINBIO_TYPE_FINGERPRINT,    // Service provider
        WINBIO_POOL_SYSTEM,         // Pool type
        WINBIO_FLAG_DEFAULT,        // Configuration and access
        NULL,                       // Array of biometric unit IDs
        0,                          // Count of biometric unit IDs
        NULL,                       // Database ID
        &sessionHandle              // [out] Session handle
    );
    if (FAILED(hr))
    {
        wprintf_s(L"\n WinBioOpenSession failed. ");
        wprintf_s(L"hr = 0x%x\n", hr);
        goto e_Exit;
    }

    // Locate a sensor.
    wprintf_s(L"\n Swipe your finger on the sensor...\n");
    hr = WinBioLocateSensor(sessionHandle, &unitId);
    if (FAILED(hr))
    {
        wprintf_s(L"\n WinBioLocateSensor failed. hr = 0x%x\n", hr);
        goto e_Exit;
    }

    // Begin the enrollment sequence. 
    wprintf_s(L"\n Starting enrollment sequence...\n");
    hr = WinBioEnrollBegin(
        sessionHandle,      // Handle to open biometric session
        subFactor,          // Finger to create template for
        unitId              // Biometric unit ID
    );
    if (FAILED(hr))
    {
        wprintf_s(L"\n WinBioEnrollBegin failed. hr = 0x%lx\n", hr);
        if (hr == WINBIO_E_DATABASE_FULL) {
            wprintf_s(L"\n Chinh la no hr+1= %lx\n",hr+1);
            wprintf_s(L"\n Chinh la no\n");
        }
        else
        {
            wprintf_s(L"\n Chinh la no hr+1= %lx\n", hr + 1);
            wprintf_s(L"\n Khong phai\n");
        }
        goto e_Exit;
    }

    // Capture enrollment information by swiping the sensor with
    // the finger identified by the subFactor argument in the 
    // WinBioEnrollBegin function.
    for (int swipeCount = 1;; ++swipeCount)
    {
        wprintf_s(L"\n Swipe the sensor to capture %s sample.",
            (swipeCount == 1) ? L"the first" : L"another");

        hr = WinBioEnrollCapture(
            sessionHandle,  // Handle to open biometric session
            &rejectDetail   // [out] Failure information
        );

        wprintf_s(L"\n Sample %d captured from unit number %d.",
            swipeCount,
            unitId);

        if (hr == WINBIO_I_MORE_DATA)
        {
            wprintf_s(L"\n    More data required.\n");
            continue;
        }
        if (FAILED(hr))
        {
            if (hr == WINBIO_E_BAD_CAPTURE)
            {
                wprintf_s(L"\n  Error: Bad capture; reason: %d",
                    rejectDetail);
                continue;
            }
            else
            {
                wprintf_s(L"\n WinBioEnrollCapture failed. hr = 0x%x", hr);
                goto e_Exit;
            }
        }
        else
        {
            wprintf_s(L"\n    Template completed.\n");
            break;
        }
    }

    // Discard the enrollment if the appropriate flag is set.
    // Commit the enrollment if it is not discarded.
    if (discardEnrollment == TRUE)
    {
        wprintf_s(L"\n Discarding enrollment...\n\n");
        hr = WinBioEnrollDiscard(sessionHandle);
        if (FAILED(hr))
        {
            wprintf_s(L"\n WinBioLocateSensor failed. hr = 0x%x\n", hr);
        }
        goto e_Exit;
    }
    else
    {
        wprintf_s(L"\n Committing enrollment...\n");
        hr = WinBioEnrollCommit(
            sessionHandle,      // Handle to open biometric session
            &identity,          // WINBIO_IDENTITY object for the user
            &isNewTemplate);    // Is this a new template

        if (FAILED(hr))
        {
            wprintf_s(L"\n WinBioEnrollCommit failed. hr = 0x%x\n", hr);
            goto e_Exit;
        }
        wprintf_s(L"\n identity type = %lo\n", identity.Type);
        wprintf_s(L"\n identity Null = %lo\n", identity.Value.Null);
        wprintf_s(L"\n identity SecureId = %lo\n", identity.Value.SecureId);
        wprintf_s(L"\n identity TemplateGuid = %lo\n", identity.Value.TemplateGuid);
        wprintf_s(L"\n identity Wildcard = %lo\n", identity.Value.Wildcard);
        wprintf_s(L"\n identity AccountSid Data = %lo\n", identity.Value.AccountSid.Data);
        wprintf_s(L"\n identity AccountSid Size = %lo\n", identity.Value.AccountSid.Size);

    }


e_Exit:
    if (sessionHandle != NULL)
    {
        WinBioCloseSession(sessionHandle);
        sessionHandle = NULL;
    }

    wprintf_s(L" Press any key to continue...");
    getchar();

    return hr;
}
HRESULT EnumEnrollments(
    WINBIO_UNIT_ID* unitArray,
    SIZE_T unitCount
)
{
    // Declare variables.
    HRESULT hr = S_OK;
    WINBIO_IDENTITY identity = { 0 };
    WINBIO_SESSION_HANDLE sessionHandle = NULL;
    WINBIO_UNIT_ID unitId = 0;
    PWINBIO_BIOMETRIC_SUBTYPE subFactorArray = NULL;
    WINBIO_BIOMETRIC_SUBTYPE SubFactor = 0;
    SIZE_T subFactorCount = 0;
    WINBIO_REJECT_DETAIL rejectDetail = 0;
    WINBIO_BIOMETRIC_SUBTYPE subFactor = WINBIO_SUBTYPE_NO_INFORMATION;

    // Connect to the system pool. 
    hr = WinBioOpenSession(
        WINBIO_TYPE_FINGERPRINT,    // Service provider
        WINBIO_POOL_PRIVATE,         // Pool type
        WINBIO_FLAG_BASIC,        // Configuration and access
        unitArray,                       // Array of biometric unit IDs
        unitCount,                          // Count of biometric unit IDs
        WINBIO_DB_ONCHIP,                       // Database ID
        &sessionHandle              // [out] Session handle
    );
    if (FAILED(hr))
    {
        wprintf_s(L"\n WinBioOpenSession failed. hr = 0x%x\n", hr);
        goto e_Exit;
    }

    // Locate the biometric sensor and retrieve a WINBIO_IDENTITY object.
    wprintf_s(L"\n Calling WinBioIdentify - Swipe finger on sensor...\n");
    hr = WinBioIdentify(
        sessionHandle,              // Session handle
        &unitId,                    // Biometric unit ID
        &identity,                  // User SID
        &subFactor,                 // Finger sub factor
        &rejectDetail               // Rejection information
    );
    wprintf_s(L"\n Swipe processed - Unit ID: %d\n", unitId);
    if (FAILED(hr))
    {
        if (hr == WINBIO_E_UNKNOWN_ID)
        {
            wprintf_s(L"\n Unknown identity.\n");
        }
        else if (hr == WINBIO_E_BAD_CAPTURE)
        {
            wprintf_s(L"\n Bad capture; reason: %d\n", rejectDetail);
        }
        else
        {
            wprintf_s(L"\n WinBioEnumBiometricUnits failed. hr = 0x%x\n", hr);
        }
        goto e_Exit;
    }

    // Retrieve the biometric sub-factors for the template.
    hr = WinBioEnumEnrollments(
        sessionHandle,              // Session handle
        unitId,                     // Biometric unit ID
        &identity,                  // Template ID
        &subFactorArray,            // Subfactors
        &subFactorCount             // Count of subfactors
    );
    if (FAILED(hr))
    {
        wprintf_s(L"\n WinBioEnumEnrollments failed. hr = 0x%x\n", hr);
        goto e_Exit;
    }
    
    wprintf_s(L"\n identity infor. type = %x\n", identity.Type);
    wprintf_s(L"\n identity infor. Value Null = %x\n", identity.Value.Null);
    wprintf_s(L"\n identity infor. Value SecureId = %x\n", identity.Value.SecureId);
    wprintf_s(L"\n identity infor. Value AccountSid Size = %x\n", identity.Value.AccountSid.Size);
    wprintf_s(L"\n identity infor. Value AccountSid Data = %x\n", identity.Value.AccountSid.Data);
    wprintf_s(L"\n identity infor. Value TemplateGuid = %x\n", identity.Value.TemplateGuid);
    wprintf_s(L"\n identity infor. Value Wildcard = %x\n", identity.Value.Wildcard);

    // Print the sub-factor(s) to the console.
    wprintf_s(L"\n Enrollments for this user on Unit ID %d:", unitId);
    for (SIZE_T index = 0; index < subFactorCount; ++index)
    {
        SubFactor = subFactorArray[index];
        switch (SubFactor)
        {
        case WINBIO_ANSI_381_POS_RH_THUMB:
            wprintf_s(L"\n   RH thumb\n");
            break;
        case WINBIO_ANSI_381_POS_RH_INDEX_FINGER:
            wprintf_s(L"\n   RH index finger\n");
            break;
        case WINBIO_ANSI_381_POS_RH_MIDDLE_FINGER:
            wprintf_s(L"\n   RH middle finger\n");
            break;
        case WINBIO_ANSI_381_POS_RH_RING_FINGER:
            wprintf_s(L"\n   RH ring finger\n");
            break;
        case WINBIO_ANSI_381_POS_RH_LITTLE_FINGER:
            wprintf_s(L"\n   RH little finger\n");
            break;
        case WINBIO_ANSI_381_POS_LH_THUMB:
            wprintf_s(L"\n   LH thumb\n");
            break;
        case WINBIO_ANSI_381_POS_LH_INDEX_FINGER:
            wprintf_s(L"\n   LH index finger\n");
            break;
        case WINBIO_ANSI_381_POS_LH_MIDDLE_FINGER:
            wprintf_s(L"\n   LH middle finger\n");
            break;
        case WINBIO_ANSI_381_POS_LH_RING_FINGER:
            wprintf_s(L"\n   LH ring finger\n");
            break;
        case WINBIO_ANSI_381_POS_LH_LITTLE_FINGER:
            wprintf_s(L"\n   LH little finger\n");
            break;
        default:
            wprintf_s(L"\n   The sub-factor is not correct\n");
            break;
        }

    }

e_Exit:
    if (subFactorArray != NULL)
    {
        WinBioFree(subFactorArray);
        subFactorArray = NULL;
    }

    if (sessionHandle != NULL)
    {
        WinBioCloseSession(sessionHandle);
        sessionHandle = NULL;
    }

    wprintf_s(L"\n Press any key to exit...");
    getchar();

    return hr;
}

HRESULT EnumerateSensors(
    WINBIO_UNIT_ID **unitId,
    SIZE_T *unitCount
)
{
    // Declare variables.
    HRESULT hr = S_OK;
    PWINBIO_UNIT_SCHEMA unitSchema = NULL;
    //SIZE_T unitCount = 0;
    SIZE_T index = 0;

    // Enumerate the installed biometric units.
    hr = WinBioEnumBiometricUnits(
        WINBIO_TYPE_FINGERPRINT,        // Type of biometric unit
        &unitSchema,                    // Array of unit schemas
        unitCount);                   // Count of unit schemas

    if (FAILED(hr))
    {
        wprintf_s(L"\n WinBioEnumBiometricUnits failed. hr = 0x%x\n", hr);
        goto e_Exit;
    }
    WINBIO_UNIT_ID *unitId1 = new WINBIO_UNIT_ID[*unitCount];
    for (SIZE_T index = 0; index < *unitCount; ++index) {
        unitId1[index] = unitSchema[index].UnitId;
    }
    *unitId = unitId1;

    // Display information for each installed biometric unit.
    wprintf_s(L"\nSensors: \n");
    /*
    for (index = 0; index < unitCount; ++index)
    {
        wprintf_s(L"\n[%d]: \tUnit ID: %d\n",
            index,
            *unitSchema[index].UnitId);
        wprintf_s(L"\tDevice instance ID: %s\n",
            unitSchema[index].DeviceInstanceId);
        wprintf_s(L"\tPool type: %d\n",
            unitSchema[index].PoolType);
        wprintf_s(L"\tBiometric factor: %d\n",
            unitSchema[index].BiometricFactor);
        wprintf_s(L"\tSensor subtype: %d\n",
            unitSchema[index].SensorSubType);
        wprintf_s(L"\tSensor capabilities: 0x%08x\n",
            unitSchema[index].Capabilities);
        wprintf_s(L"\tDescription: %s\n",
            unitSchema[index].Description);
        wprintf_s(L"\tManufacturer: %s\n",
            unitSchema[index].Manufacturer);
        wprintf_s(L"\tModel: %s\n",
            unitSchema[index].Model);
        wprintf_s(L"\tSerial no: %s\n",
            unitSchema[index].SerialNumber);
        wprintf_s(L"\tFirmware version: [%d.%d]\n",
            unitSchema[index].FirmwareVersion.MajorVersion,
            unitSchema[index].FirmwareVersion.MinorVersion);
    }
    */


e_Exit:
    /*
    if (unitSchema != NULL)
    {
        WinBioFree(unitSchema);
        unitSchema = NULL;
    }
    */

    //wprintf_s(L"\nPress any key to exit...");
    //getchar();

    return hr;
}

VOID DisplayGuid(__in PWINBIO_UUID Guid)
{
    wprintf_s(
        L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        Guid->Data1,
        Guid->Data2,
        Guid->Data3,
        Guid->Data4[0],
        Guid->Data4[1],
        Guid->Data4[2],
        Guid->Data4[3],
        Guid->Data4[4],
        Guid->Data4[5],
        Guid->Data4[6],
        Guid->Data4[7]
    );
}

HRESULT EnumDatabases()
{
    // Declare variables.
    HRESULT hr = S_OK;
    PWINBIO_STORAGE_SCHEMA storageSchemaArray = NULL;
    SIZE_T storageCount = 0;
    SIZE_T index = 0;

    // Enumerate the databases.
    hr = WinBioEnumDatabases(
        WINBIO_TYPE_FINGERPRINT,    // Type of biometric unit
        &storageSchemaArray,        // Array of database schemas
        &storageCount);            // Number of database schemas
    if (FAILED(hr))
    {
        wprintf_s(L"\nWinBioEnumDatabases failed. hr = 0x%x\n", hr);
        goto e_Exit;
    }

    // Display information for each database.
    wprintf_s(L"\nDatabases:\n");
    for (index = 0; index < storageCount; ++index)
    {
        wprintf_s(L"\n[%d]: \tBiometric factor: 0x%08x\n",
            index,
            storageSchemaArray[index].BiometricFactor);

        wprintf_s(L"\tDatabase ID: ");
        DisplayGuid(&storageSchemaArray[index].DatabaseId);
        wprintf_s(L"\n");

        wprintf_s(L"\tData format: ");
        DisplayGuid(&storageSchemaArray[index].DataFormat);
        wprintf_s(L"\n");

        wprintf_s(L"\tAttributes:  0x%08x\n",
            storageSchemaArray[index].Attributes);

        wprintf_s(L"\tFile path:   %ws\n",
            storageSchemaArray[index].FilePath);

        wprintf_s(L"\tCnx string:  %ws\n",
            storageSchemaArray[index].ConnectionString);

        wprintf_s(L"\n");
    }

e_Exit:
    if (storageSchemaArray != NULL)
    {
        WinBioFree(storageSchemaArray);
        storageSchemaArray = NULL;
    }

    wprintf_s(L"\nPress any key to exit...");
    getchar();

    return hr;
}


//------------------------------------------------------------------------
// The following function displays a GUID to the console window.
//

HRESULT LogonIdentifiedUser()
{
    // Declare variables.
    HRESULT hr;
    WINBIO_SESSION_HANDLE sessionHandle = NULL;
    WINBIO_UNIT_ID  UnitId;
    WINBIO_IDENTITY Identity;
    WINBIO_BIOMETRIC_SUBTYPE SubFactor;
    WINBIO_REJECT_DETAIL RejectDetail;
    BOOL    bContinue = TRUE;

    // Connect to the system pool. 
    hr = WinBioOpenSession(
        WINBIO_TYPE_FINGERPRINT,    // Service provider
        WINBIO_POOL_SYSTEM,         // Pool type
        WINBIO_FLAG_DEFAULT,        // Configuration and access
        NULL,                       // Array of biometric unit IDs
        0,                          // Count of biometric unit IDs
        WINBIO_DB_DEFAULT,                       // Database ID
        &sessionHandle              // [out] Session handle
    );
    if (FAILED(hr))
    {
        wprintf_s(L"\n WinBioOpenSession failed. hr = 0x%x\n", hr);
        goto e_Exit;
    }

    // Locate the biometric sensor and retrieve a WINBIO_IDENTITY object.
    // You must swipe your finger on the sensor.
    wprintf_s(L"\n Calling WinBioIdentify - Swipe finger on sensor...\n");
    while (bContinue)
    {
        hr = WinBioIdentify(
            sessionHandle,          // Session handle    
            &UnitId,                // Biometric unit ID
            &Identity,              // User SID or GUID
            &SubFactor,             // Finger sub factor
            &RejectDetail           // rejection information
        );

        switch (hr)
        {
        case S_OK:
            bContinue = FALSE;
            break;
        default:
            wprintf_s(L"\n WinBioIdentify failed. hr = 0x%x\n", hr);
            break;
        }
    }

    if (SUCCEEDED(hr))
    {
        // Switch to the target after receiving a good identity.
        hr = WinBioLogonIdentifiedUser(sessionHandle);

        switch (hr)
        {
        case S_FALSE:
            printf("\n Target is the logged on user. No action taken.\n");
            break;
        case S_OK:
            printf("\n Fast user switch initiated.\n");
            break;
        default:
            wprintf_s(L"\n WinBioLogonIdentifiedUser failed. hr = 0x%x\n", hr);
            break;
        }
    }

e_Exit:

    if (sessionHandle != NULL)
    {
        WinBioCloseSession(sessionHandle);
        sessionHandle = NULL;
    }

    wprintf_s(L"\n Press any key to exit...");
    getchar();

    return hr;
}

VOID WinBioRemoveAllCredentials1() {
    HRESULT hr;
    hr = WinBioRemoveAllCredentials();
    if (FAILED(hr))
    {
        wprintf_s(L"\n WinBioRemoveAllCredentials failed. hr = 0x%x\n", hr);
    }
    else
    {
        wprintf_s(L"\n WinBioRemoveAllCredentials success\n");
    }
    getchar();
}


int main()
{
    WINBIO_UNIT_ID *unitId = NULL;
    SIZE_T unitCount = 0;
  WINBIO_IDENTITY identity = { 0 };
  CreateDirectoryA("data", NULL);
  //while(!FAILED(EnrollSysPool(false, WINBIO_ANSI_381_POS_RH_INDEX_FINGER)));
  //while (!FAILED(EnumEnrollments()));
  //EnumerateSensors(&unitId, &unitCount);
  //wprintf_s(L"\n unitCount=%d \n", unitCount);
  //wprintf_s(L"\n unitId=%d \n", unitId[0]);
  //EnumEnrollments(unitId, unitCount);
  //EnumDatabases();
  //LogonIdentifiedUser();
  //WinBioRemoveAllCredentials1();
  
  while (1) {
      char a = getchar();
      if (a == '1') {
          getchar();
          onInstall();
      };
      if (a == '2') {
          getchar();
          onUninstall();
      };
      if (a == '3') { 
          getchar();
          onAdd();
      };
      if (a == '4') {
          getchar();
          onRemove();
      }
      if (a == '5') {
          getchar();
          onEnrollOrPractice(true);
      };
      if (a == '6') {
          getchar();
          onEnrollOrPractice(false);
      };
      if (a == '7') {
          getchar();
          onIdentifyOrDelete(false);
      };
      if (a == '8') {
          getchar();
          onIdentifyOrDelete(true);
      };
  }
  /*
  while (1) {
      char a = getchar();
      if (a == '1') {
          getchar();
          onEnrollOrPractice(true);
      };
      if (a == '2') {
          getchar();
          onEnrollOrPractice(false);
      };
  };
  */
  wprintf_s(L"\nPress any key to exit...");
  getchar();
}

