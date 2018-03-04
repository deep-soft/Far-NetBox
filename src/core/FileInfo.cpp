#include <vcl.h>
#pragma hdrstop

#include <Common.h>
#include <nbutils.h>
#include <Exceptions.h>
#include <FileBuffer.h>
#include <Windows.hpp>
#include <Math.hpp>
#include "FileInfo.h"
//---------------------------------------------------------------------------
__removed #pragma package(smart_init)
//---------------------------------------------------------------------------
#define DWORD_ALIGN( base, ptr ) \
    ( (LPBYTE)(base) + ((((LPBYTE)(ptr) - (LPBYTE)(base)) + 3) & ~3) )
struct VS_VERSION_INFO_STRUCT32
{
  WORD wLength;
  WORD wValueLength;
  WORD wType;
  WCHAR szKey[1];
};
//---------------------------------------------------------------------------
static uintptr_t VERSION_GetFileVersionInfo_PE(const wchar_t *FileName, uintptr_t DataSize, void *Data)
{
  uintptr_t Len = 0;

  bool NeedFree = false;
  HMODULE Module = ::GetModuleHandle(FileName);
  if (Module == nullptr)
  {
    Module = ::LoadLibraryEx(FileName, nullptr, LOAD_LIBRARY_AS_DATAFILE);
    NeedFree = true;
  }
  if (Module == nullptr)
  {
    Len = 0;
  }
  else
  {
    try__finally
    {
      HRSRC Rsrc = ::FindResource(Module, MAKEINTRESOURCE(VS_VERSION_INFO), VS_FILE_INFO);
      if (Rsrc == nullptr)
      {
      }
      else
      {
        Len = ::SizeofResource(Module, static_cast<HRSRC>(Rsrc));
        HANDLE Mem = ::LoadResource(Module, static_cast<HRSRC>(Rsrc));
        if (Mem == nullptr)
        {
        }
        else
        {
          try__finally
          {
            VS_VERSION_INFO_STRUCT32 *VersionInfo = static_cast<VS_VERSION_INFO_STRUCT32 *>(LockResource(Mem));
            const VS_FIXEDFILEINFO *FixedInfo =
              reinterpret_cast<VS_FIXEDFILEINFO *>(DWORD_ALIGN(VersionInfo, VersionInfo->szKey + nb::StrLength(VersionInfo->szKey) + 1));

            if (FixedInfo->dwSignature != VS_FFI_SIGNATURE)
            {
              Len = 0;
            }
            else
            {
              if (Data != nullptr)
              {
                if (DataSize < Len)
                {
                  Len = DataSize;
                }
                if (Len > 0)
                {
                  memmove(Data, VersionInfo, Len);
                }
              }
            }
          },
          __finally
          {
            ::FreeResource(Mem);
          } end_try__finally
        }
      }
    },
    __finally
    {
      if (NeedFree)
      {
        ::FreeLibrary(Module);
      }
    } end_try__finally
  }

  return Len;
}
//---------------------------------------------------------------------------
static uintptr_t GetFileVersionInfoSizeFix(const wchar_t *FileName, DWORD *AHandle)
{
  uintptr_t Len;
  if (IsWin7())
  {
    *AHandle = 0;
    Len = VERSION_GetFileVersionInfo_PE(FileName, 0, nullptr);

    if (Len != 0)
    {
      Len = (Len * 2) + 4;
    }
  }
  else
  {
    Len = ::GetFileVersionInfoSize(const_cast<wchar_t *>(FileName), AHandle);
  }

  return Len;
}
//---------------------------------------------------------------------------
bool GetFileVersionInfoFix(const wchar_t *FileName, uint32_t Handle,
  uintptr_t DataSize, void *Data)
{
  bool Result;

  if (IsWin7())
  {
    VS_VERSION_INFO_STRUCT32 *VersionInfo = static_cast<VS_VERSION_INFO_STRUCT32 *>(Data);

    uintptr_t Len = VERSION_GetFileVersionInfo_PE(FileName, DataSize, Data);

    Result = (Len != 0);
    if (Result)
    {
      static const char Signature[] = "FE2X";
      uintptr_t BufSize = ToUIntPtr(VersionInfo->wLength + NBChTraitsCRT<char>::SafeStringLen(Signature));

      if (DataSize >= BufSize)
      {
        uintptr_t ConvBuf = DataSize - VersionInfo->wLength;
        memmove((static_cast<char *>(Data)) + VersionInfo->wLength, Signature, ConvBuf > 4 ? 4 : ConvBuf);
      }
    }
  }
  else
  {
    Result = ::GetFileVersionInfo(FileName, Handle, ToDWord(DataSize), Data) != 0;
  }

  return Result;
}
//---------------------------------------------------------------------------
// Return pointer to file version info block
void *CreateFileInfo(const UnicodeString AFileName)
{
  DWORD Handle;
  void *Result = nullptr;

  // Get file version info block size
  uintptr_t Size = GetFileVersionInfoSizeFix(AFileName.c_str(), &Handle);
  // If size is valid
  if (Size > 0)
  {
    Result = nb::calloc<void *>(Size, 1);
    // Get file version info block
    if (!GetFileVersionInfoFix(AFileName.c_str(), Handle, Size, Result))
    {
      nb_free(Result);
      Result = nullptr;
    }
  }
  else
  {
  }
  return Result;
}
//---------------------------------------------------------------------------
// Free file version info block memory
void FreeFileInfo(void *FileInfo)
{
  if (FileInfo)
    nb_free(FileInfo);
}
//---------------------------------------------------------------------------
typedef TTranslation TTranslations[65536];
typedef TTranslation *PTranslations;
//---------------------------------------------------------------------------
// Return pointer to fixed file version info
PVSFixedFileInfo GetFixedFileInfo(void *FileInfo)
{
  UINT Len;
  PVSFixedFileInfo Result = nullptr;
  if (FileInfo && !::VerQueryValue(FileInfo, L"\\", reinterpret_cast<void **>(&Result), &Len))
  {
    throw Exception(L"Fixed file info not available");
  }
  return Result;
}
//---------------------------------------------------------------------------
// Return number of available file version info translations
uint32_t GetTranslationCount(void *FileInfo)
{
  PTranslations P;
  UINT Len;
  if (!::VerQueryValue(FileInfo, L"\\VarFileInfo\\Translation", reinterpret_cast<void **>(&P), &Len))
    throw Exception(L"File info translations not available");
  return Len / 4;
}
//---------------------------------------------------------------------------
// Return i-th translation in the file version info translation list
TTranslation GetTranslation(void *FileInfo, intptr_t I)
{
  PTranslations P = nullptr;
  UINT Len;

  if (!::VerQueryValue(FileInfo, L"\\VarFileInfo\\Translation", reinterpret_cast<void **>(&P), &Len))
    throw Exception(L"File info translations not available");
  if (I * sizeof(TTranslation) >= Len)
    throw Exception(L"Specified translation not available");
  return P[I];
}
//---------------------------------------------------------------------------
// Return the name of the specified language
UnicodeString GetLanguage(Word Language)
{
  wchar_t P[256];

  uintptr_t Len = ::VerLanguageName(Language, P, _countof(P));
  if (Len > _countof(P))
    throw Exception(L"Language not available");
  return UnicodeString(P, Len);
}
//---------------------------------------------------------------------------
// Return the value of the specified file version info string using the
// specified translation
UnicodeString GetFileInfoString(void *FileInfo,
  TTranslation Translation, const UnicodeString StringName, bool AllowEmpty)
{
  UnicodeString Result;
  wchar_t *P;
  UINT Len;

  if (!::VerQueryValue(FileInfo, (UnicodeString(L"\\StringFileInfo\\") +
        ::IntToHex(Translation.Language, 4) +
        ::IntToHex(Translation.CharSet, 4) +
        L"\\" + StringName).c_str(), reinterpret_cast<void **>(&P), &Len))
  {
    if (!AllowEmpty)
    {
      throw Exception(L"Specified file info string not available");
    }
  }
  else
  {
    Result = UnicodeString(P, Len);
    PackStr(Result);
  }
  return Result;
}
//---------------------------------------------------------------------------
intptr_t CalculateCompoundVersion(intptr_t MajorVer,
  intptr_t MinorVer, intptr_t Release, intptr_t Build)
{
  intptr_t CompoundVer = Build + 10000 * (Release + 100 * (MinorVer +
        100 * MajorVer));
  return CompoundVer;
}
//---------------------------------------------------------------------------
intptr_t StrToCompoundVersion(const UnicodeString AStr)
{
  UnicodeString S(AStr);
  int64_t MajorVer = ::StrToInt64(CutToChar(S, L'.', false));
  int64_t MinorVer = ::StrToInt64(CutToChar(S, L'.', false));
  int64_t Release = S.IsEmpty() ? 0 : StrToInt64(CutToChar(S, L'.', false));
  int64_t Build = S.IsEmpty() ? 0 : StrToInt64(CutToChar(S, L'.', false));
  return CalculateCompoundVersion(ToIntPtr(MajorVer), ToIntPtr(MinorVer), ToIntPtr(Release), ToIntPtr(Build));
}
//---------------------------------------------------------------------------
intptr_t CompareVersion(const UnicodeString V1, const UnicodeString V2)
{
  intptr_t Result = 0;
  UnicodeString _V1(V1), _V2(V2);
  while ((Result == 0) && (!_V1.IsEmpty() || !_V2.IsEmpty()))
  {
    intptr_t C1 = StrToIntDef(CutToChar(_V1, L'.', false), 0);
    intptr_t C2 = StrToIntDef(CutToChar(_V2, L'.', false), 0);
    // Result = CompareValue(C1, C2);
    if (C1 < C2)
    {
      Result = -1;
    }
    else if (C1 > C2)
    {
      Result = 1;
    }
    else
      Result = 0;
  }
  return Result;
}
