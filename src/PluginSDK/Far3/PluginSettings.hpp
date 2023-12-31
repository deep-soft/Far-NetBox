#ifndef __PLUGINSETTINGS_HPP__
#define __PLUGINSETTINGS_HPP__

#include "plugin.hpp"

class PluginSettings
{
  HANDLE handle{INVALID_HANDLE_VALUE};
  FARAPISETTINGSCONTROL SettingsControl{};

public:

  PluginSettings(const GUID & guid, FARAPISETTINGSCONTROL SettingsControl)
  {
    this->SettingsControl = SettingsControl;
    handle = INVALID_HANDLE_VALUE;

    FarSettingsCreate settings={sizeof(FarSettingsCreate),guid,handle};
    if (SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,&settings))
      handle = settings.Handle;
  }

  ~PluginSettings()
  {
    SettingsControl(handle,SCTL_FREE,0,nullptr);
  }

  intptr_t CreateSubKey(size_t Root, const wchar_t *Name)
  {
    FarSettingsValue value={sizeof(FarSettingsValue),Root,Name};
    return SettingsControl(handle,SCTL_CREATESUBKEY,0,&value);
  }

  intptr_t OpenSubKey(size_t Root, const wchar_t *Name)
  {
    // DEBUG_PRINTF(L"handle = %d, Name = %s", (int)handle, Name);
    FarSettingsValue value={sizeof(FarSettingsValue),Root,Name};
    intptr_t Result = SettingsControl(handle,SCTL_OPENSUBKEY,0,&value);
    // DEBUG_PRINTF(L"Result = %d", Result);
    return Result;
  }

  bool DeleteSubKey(size_t Root)
  {
    FarSettingsValue value={sizeof(FarSettingsValue),Root,nullptr};
    return SettingsControl(handle,SCTL_DELETE,0,&value) ? true : false;
  }

  bool DeleteValue(size_t Root, const wchar_t *Name)
  {
    FarSettingsValue value={sizeof(FarSettingsValue),Root,Name};
    return SettingsControl(handle,SCTL_DELETE,0,&value) ? true : false;
  }

  const wchar_t * Get(size_t Root, const wchar_t *Name, const wchar_t *Default)
  {
    FarSettingsItem item={sizeof(FarSettingsItem),Root,Name,FST_STRING};
    if (SettingsControl(handle,SCTL_GET,0,&item))
    {
      return item.String;
    }
    return Default;
  }

  void Get(size_t Root, const wchar_t *Name, wchar_t *Value, size_t Size, const wchar_t *Default)
  {
    lstrcpyn(Value, Get(Root,Name,Default), (int)Size);
  }

  uint64_t Get(size_t Root, const wchar_t *Name, uint64_t Default)
  {
    FarSettingsItem item={sizeof(FarSettingsItem),Root,Name,FST_QWORD};
    if (SettingsControl(handle,SCTL_GET,0,&item))
    {
      return item.Number;
    }
    return Default;
  }

  int64_t      Get(size_t Root, const wchar_t *Name, int64_t Default) { return (__int64)Get(Root,Name,(uint64_t)Default); }
  int          Get(size_t Root, const wchar_t *Name, int Default)  { return (int)Get(Root,Name,(uint64_t)Default); }
  unsigned int Get(size_t Root, const wchar_t *Name, unsigned int Default) { return (unsigned int)Get(Root,Name,(uint64_t)Default); }
  DWORD        Get(size_t Root, const wchar_t *Name, DWORD Default) { return (DWORD)Get(Root,Name,(uint64_t)Default); }
  bool         Get(size_t Root, const wchar_t *Name, bool Default) { return Get(Root,Name,Default?1ull:0ull)?true:false; }

  size_t Get(size_t Root, const wchar_t *Name, void *Value, size_t Size)
  {
    FarSettingsItem item={sizeof(FarSettingsItem),Root,Name,FST_DATA};
    if (SettingsControl(handle,SCTL_GET,0,&item))
    {
      Size = (item.Data.Size>Size)?Size:item.Data.Size;
      memcpy(Value,item.Data.Data,Size);
      return Size;
    }
    return 0;
  }

  bool Set(size_t Root, const wchar_t *Name, const wchar_t *Value)
  {
    FarSettingsItem item={sizeof(FarSettingsItem),Root,Name,FST_STRING};
    item.String=Value;
    return SettingsControl(handle,SCTL_SET,0,&item)!=FALSE;
  }

  bool Set(size_t Root, const wchar_t *Name, unsigned __int64 Value)
  {
    FarSettingsItem item={sizeof(FarSettingsItem),Root,Name,FST_QWORD};
    item.Number=Value;
    return SettingsControl(handle,SCTL_SET,0,&item)!=FALSE;
  }

  bool Set(size_t Root, const wchar_t *Name, __int64 Value) { return Set(Root,Name,(unsigned __int64)Value); }
  bool Set(size_t Root, const wchar_t *Name, int Value) { return Set(Root,Name,(unsigned __int64)Value); }
  bool Set(size_t Root, const wchar_t *Name, unsigned int Value) { return Set(Root,Name,(unsigned __int64)Value); }
  bool Set(size_t Root, const wchar_t *Name, DWORD Value) { return Set(Root,Name,(unsigned __int64)Value); }
  bool Set(size_t Root, const wchar_t *Name, bool Value) { return Set(Root,Name,Value?1ull:0ull); }

  bool Set(size_t Root, const wchar_t *Name, const void *Value, size_t Size)
  {
    FarSettingsItem item={sizeof(FarSettingsItem),Root,Name,FST_DATA};
    item.Data.Size=Size;
    item.Data.Data=Value;
    return SettingsControl(handle,SCTL_SET,0,&item)!=FALSE;
  }

	bool Enum(size_t Root, FarSettingsEnum* fse)
	{
		fse->Root=Root;
		fse->StructSize=sizeof(FarSettingsEnum);
		return SettingsControl(handle, SCTL_ENUM, 0, fse)!=FALSE;
	}

  bool GetSubKeyNames(FarSettingsEnum &settings)
  {
    return SettingsControl(handle, SCTL_ENUM, 0, &settings) != FALSE;
  }

  bool ValueExists(size_t Root, const wchar_t *Name)
  {
    FarSettingsItem item = {sizeof(FarSettingsItem), Root, Name, FST_DATA};
    return SettingsControl(handle, SCTL_GET, 0, &item) != FALSE;
  }

  int BinaryDataSize(size_t Root, const wchar_t *Name)
  {
    FarSettingsItem item = {sizeof(FarSettingsItem), Root, Name, FST_DATA};
    if (SettingsControl(handle, SCTL_GET, 0, &item) != FALSE)
    {
       return (int)item.Data.Size;
    }
    return 0;
  }
};

#endif
