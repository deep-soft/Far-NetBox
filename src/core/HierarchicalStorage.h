//---------------------------------------------------------------------------
#ifndef HierarchicalStorageH
#define HierarchicalStorageH

#include <registry.hpp>
//---------------------------------------------------------------------------
enum TStorage { stDetect, stRegistry, stIniFile, stNul, stXmlFile };
enum TStorageAccessMode { smRead, smReadWrite };
//---------------------------------------------------------------------------
class THierarchicalStorage
{
public:
  explicit /* __fastcall */ THierarchicalStorage(const UnicodeString & AStorage);
  virtual void __fastcall Init() {}
  virtual /* __fastcall */ ~THierarchicalStorage();
  bool __fastcall OpenRootKey(bool CanCreate);
  bool __fastcall OpenSubKey(const UnicodeString & SubKey, bool CanCreate, bool Path = false);
  virtual void __fastcall CloseSubKey();
  virtual bool __fastcall DeleteSubKey(const UnicodeString & SubKey) = 0;
  virtual void __fastcall GetSubKeyNames(Classes::TStrings* Strings) = 0;
  virtual void __fastcall GetValueNames(Classes::TStrings* Strings) = 0;
  bool __fastcall HasSubKeys();
  bool __fastcall HasSubKey(const UnicodeString & SubKey);
  bool __fastcall KeyExists(const UnicodeString & SubKey);
  virtual bool __fastcall ValueExists(const UnicodeString & Value) = 0;
  virtual void __fastcall RecursiveDeleteSubKey(const UnicodeString & Key);
  virtual void __fastcall ClearSubKeys();
  virtual void __fastcall ReadValues(Classes::TStrings* Strings, bool MaintainKeys = false);
  virtual void __fastcall WriteValues(Classes::TStrings* Strings, bool MaintainKeys = false);
  virtual void __fastcall ClearValues();
  virtual bool __fastcall DeleteValue(const UnicodeString & Name) = 0;

  virtual size_t __fastcall BinaryDataSize(const UnicodeString & Name) = 0;

  virtual bool __fastcall ReadBool(const UnicodeString & Name, bool Default) = 0;
  virtual int __fastcall ReadInteger(const UnicodeString & Name, int Default) = 0;
  virtual __int64 __fastcall ReadInt64(const UnicodeString & Name, __int64 Default) = 0;
  virtual TDateTime __fastcall ReadDateTime(const UnicodeString & Name, TDateTime Default) = 0;
  virtual double __fastcall ReadFloat(const UnicodeString & Name, double Default) = 0;
  virtual UnicodeString __fastcall ReadStringRaw(const UnicodeString & Name, const UnicodeString & Default) = 0;
  virtual size_t __fastcall ReadBinaryData(const UnicodeString & Name, void * Buffer, size_t Size) = 0;

  virtual UnicodeString __fastcall ReadString(const UnicodeString & Name, const UnicodeString & Default);
  RawByteString __fastcall ReadBinaryData(const UnicodeString & Name);
  RawByteString __fastcall ReadStringAsBinaryData(const UnicodeString & Name, const RawByteString & Default);

  virtual void __fastcall WriteBool(const UnicodeString & Name, bool Value) = 0;
  virtual void __fastcall WriteStringRaw(const UnicodeString & Name, const UnicodeString & Value) = 0;
  virtual void __fastcall WriteInteger(const UnicodeString & Name, int Value) = 0;
  virtual void __fastcall WriteInt64(const UnicodeString & Name, __int64 Value) = 0;
  virtual void __fastcall WriteDateTime(const UnicodeString & Name, TDateTime Value) = 0;
  virtual void __fastcall WriteFloat(const UnicodeString & Name, double Value) = 0;
  virtual void __fastcall WriteBinaryData(const UnicodeString & Name, const void * Buffer, size_t Size) = 0;

  virtual void __fastcall WriteString(const UnicodeString & Name, const UnicodeString & Value);
  void __fastcall WriteBinaryData(const UnicodeString & Name, const RawByteString & Value);
  void __fastcall WriteBinaryDataAsString(const UnicodeString & Name, const RawByteString & Value);

  virtual void __fastcall Flush();

  UnicodeString __fastcall GetStorage() const { return FStorage; }
  TStorageAccessMode __fastcall GetAccessMode() const { return FAccessMode; }
  bool __fastcall GetExplicit() const { return FExplicit; }
  void __fastcall SetExplicit(bool value) { FExplicit = value; }
  bool __fastcall GetForceAnsi() const { return FForceAnsi; }
  void __fastcall SetForceAnsi(bool value) { FForceAnsi = value; }
  bool __fastcall GetMungeStringValues() const { return FMungeStringValues; }
  void __fastcall SetMungeStringValues(bool value) { FMungeStringValues = value; }

  virtual void __fastcall SetAccessMode(TStorageAccessMode value);
  virtual UnicodeString __fastcall GetSource() = 0;

protected:
  UnicodeString FStorage;
  TStrings * FKeyHistory;
  TStorageAccessMode FAccessMode;
  bool FExplicit;
  bool FMungeStringValues;
  bool FForceAnsi;

  UnicodeString __fastcall GetCurrentSubKey() const;
  UnicodeString __fastcall GetCurrentSubKeyMunged() const;
  virtual bool __fastcall DoKeyExists(const UnicodeString & SubKey, bool ForceAnsi) = 0;
  static UnicodeString __fastcall IncludeTrailingBackslash(const UnicodeString & S);
  static UnicodeString __fastcall ExcludeTrailingBackslash(const UnicodeString & S);
  virtual bool __fastcall DoOpenSubKey(const UnicodeString & SubKey, bool CanCreate) = 0;
  UnicodeString __fastcall MungeKeyName(const UnicodeString & Key);

private:
  THierarchicalStorage(const THierarchicalStorage &);
  THierarchicalStorage & operator=(const THierarchicalStorage &);
};
//---------------------------------------------------------------------------
class TRegistryStorage : public THierarchicalStorage
{
public:
  explicit /* __fastcall */ TRegistryStorage(const UnicodeString & AStorage, HKEY ARootKey);
  explicit /* __fastcall */ TRegistryStorage(const UnicodeString & AStorage);
  virtual void __fastcall Init();
  virtual /* __fastcall */ ~TRegistryStorage();

  bool __fastcall Copy(TRegistryStorage * Storage);

  virtual void __fastcall CloseSubKey();
  virtual bool __fastcall DeleteSubKey(const UnicodeString & SubKey);
  virtual bool __fastcall DeleteValue(const UnicodeString & Name);
  virtual void __fastcall GetSubKeyNames(Classes::TStrings* Strings);
  virtual bool __fastcall ValueExists(const UnicodeString & Value);

  virtual size_t __fastcall BinaryDataSize(const UnicodeString & Name);

  virtual bool __fastcall ReadBool(const UnicodeString & Name, bool Default);
  virtual int __fastcall ReadInteger(const UnicodeString & Name, int Default);
  virtual __int64 __fastcall ReadInt64(const UnicodeString & Name, __int64 Default);
  virtual TDateTime __fastcall ReadDateTime(const UnicodeString & Name, TDateTime Default);
  virtual double __fastcall ReadFloat(const UnicodeString & Name, double Default);
  virtual UnicodeString __fastcall ReadStringRaw(const UnicodeString & Name, const UnicodeString & Default);
  virtual size_t __fastcall ReadBinaryData(const UnicodeString & Name, void * Buffer, size_t Size);

  virtual void __fastcall WriteBool(const UnicodeString & Name, bool Value);
  virtual void __fastcall WriteInteger(const UnicodeString & Name, int Value);
  virtual void __fastcall WriteInt64(const UnicodeString & Name, __int64 Value);
  virtual void __fastcall WriteDateTime(const UnicodeString & Name, TDateTime Value);
  virtual void __fastcall WriteFloat(const UnicodeString & Name, double Value);
  virtual void __fastcall WriteStringRaw(const UnicodeString & Name, const UnicodeString & Value);
  virtual void __fastcall WriteBinaryData(const UnicodeString & Name, const void * Buffer, size_t Size);

  virtual void __fastcall GetValueNames(Classes::TStrings* Strings);

  int __fastcall GetFailed();
  void __fastcall SetFailed(int value) { FFailed = value; }
  virtual void __fastcall SetAccessMode(TStorageAccessMode value);

protected:
  virtual bool __fastcall DoKeyExists(const UnicodeString & SubKey, bool ForceAnsi);
  virtual bool __fastcall DoOpenSubKey(const UnicodeString & SubKey, bool CanCreate);
  virtual UnicodeString __fastcall GetSource();

private:
  TRegistry * FRegistry;
  int FFailed;

  // void __fastcall Init();
};
#ifndef _MSC_VER
//---------------------------------------------------------------------------
class TCustomIniFileStorage : public THierarchicalStorage
{
public:
  explicit /* __fastcall */ TCustomIniFileStorage(const UnicodeString & Storage, TCustomIniFile * IniFile);
  virtual void __fastcall Init() {}
  virtual /* __fastcall */ ~TCustomIniFileStorage();

  virtual bool __fastcall DeleteSubKey(const UnicodeString & SubKey);
  virtual bool __fastcall DeleteValue(const UnicodeString & Name);
  virtual void __fastcall GetSubKeyNames(Classes::TStrings* Strings);
  virtual bool __fastcall ValueExists(const UnicodeString & Value);

  virtual size_t __fastcall BinaryDataSize(const UnicodeString & Name);

  virtual bool __fastcall ReadBool(const UnicodeString & Name, bool Default);
  virtual int __fastcall ReadInteger(const UnicodeString & Name, int Default);
  virtual __int64 __fastcall ReadInt64(const UnicodeString & Name, __int64 Default);
  virtual TDateTime __fastcall ReadDateTime(const UnicodeString & Name, TDateTime Default);
  virtual double __fastcall ReadFloat(const UnicodeString & Name, double Default);
  virtual UnicodeString __fastcall ReadStringRaw(const UnicodeString & Name, const UnicodeString & Default);
  virtual size_t __fastcall ReadBinaryData(const UnicodeString & Name, void * Buffer, size_t Size);

  virtual void __fastcall WriteBool(const UnicodeString & Name, bool Value);
  virtual void __fastcall WriteInteger(const UnicodeString & Name, int Value);
  virtual void __fastcall WriteInt64(const UnicodeString & Name, __int64 Value);
  virtual void __fastcall WriteDateTime(const UnicodeString & Name, TDateTime Value);
  virtual void __fastcall WriteFloat(const UnicodeString & Name, double Value);
  virtual void __fastcall WriteStringRaw(const UnicodeString & Name, const UnicodeString & Value);
  virtual void __fastcall WriteBinaryData(const UnicodeString & Name, const void * Buffer, size_t Size);

  virtual void __fastcall GetValueNames(Classes::TStrings* Strings);

private:
  UnicodeString __fastcall GetCurrentSection() const;

protected:
  TCustomIniFile * FIniFile;

  virtual bool __fastcall DoKeyExists(const UnicodeString & SubKey, bool ForceAnsi);
  virtual bool __fastcall DoOpenSubKey(const UnicodeString & SubKey, bool CanCreate);
  virtual UnicodeString __fastcall GetSource();
};
//---------------------------------------------------------------------------
class TIniFileStorage : public TCustomIniFileStorage
{
public:
  explicit /* __fastcall */ TIniFileStorage(const UnicodeString & FileName);
  virtual /* __fastcall */ ~TIniFileStorage();

  virtual void __fastcall Flush();

private:
  TStrings * FOriginal;
  void __fastcall ApplyOverrides();
};
//---------------------------------------------------------------------------
class TOptionsStorage : public TCustomIniFileStorage
{
public:
  explicit /* __fastcall */ TOptionsStorage(TStrings * Options);
};
#endif
//---------------------------------------------------------------------------
UnicodeString __fastcall PuttyMungeStr(const UnicodeString & Str);
UnicodeString __fastcall PuttyUnMungeStr(const UnicodeString & Str);
//---------------------------------------------------------------------------
#endif
