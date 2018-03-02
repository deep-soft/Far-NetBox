#pragma once

#pragma warning(push, 1)
#include <vcl.h>
#include <Sysutils.hpp>
#include <plugin.hpp>
#pragma warning(pop)

#include <Common.h>

class TCustomFarFileSystem;
class TFarPanelModes;
class TFarKeyBarTitles;
class TFarPanelInfo;
class TFarDialog;
class TWinSCPFileSystem;
class TFarDialogItem;
class TFarMessageDialog;
class TFarEditorInfo;
class TFarPluginGuard;

const int MaxMessageWidth = 64;

enum TFarShiftStatus
{
  fsNone,
  fsCtrl,
  fsAlt,
  fsShift,
  fsCtrlShift,
  fsAltShift,
  fsCtrlAlt
};

enum THandlesFunction
{
  hfProcessKey,
  hfProcessHostFile,
  hfProcessEvent
};

#if 0
typedef void (__closure *TFarInputBoxValidateEvent)
(AnsiString &Text);
#endif // #if 0
typedef nb::FastDelegate1<void, UnicodeString & /*Text*/> TFarInputBoxValidateEvent;
#if 0
typedef void (__closure *TFarMessageTimerEvent)(unsigned int &Result);
#endif // #if 0
typedef nb::FastDelegate1<void, uint32_t & /*Result*/> TFarMessageTimerEvent;
#if 0
typedef void (__closure *TFarMessageClickEvent)(void *Token, int Result, bool &Close);
#endif // #if 0
typedef nb::FastDelegate3<void, void * /*Token*/,
        uintptr_t /*Result*/, bool & /*Close*/> TFarMessageClickEvent;

struct TFarMessageParams : public TObject
{
  NB_DISABLE_COPY(TFarMessageParams)
public:
  TFarMessageParams();

  TStrings *MoreMessages;
  UnicodeString CheckBoxLabel;
  bool CheckBox;
  uintptr_t Timer;
  uint32_t TimerAnswer;
  TFarMessageTimerEvent TimerEvent;
  uintptr_t Timeout;
  uintptr_t TimeoutButton;
  uintptr_t DefaultButton;
  UnicodeString TimeoutStr;
  TFarMessageClickEvent ClickEvent;
  void *Token;
};

NB_DEFINE_CLASS_ID(TCustomFarPlugin);
class TCustomFarPlugin : public TObject
{
  friend class TCustomFarFileSystem;
  friend class TFarDialog;
  friend class TWinSCPFileSystem;
  friend class TFarDialogItem;
  friend class TFarMessageDialog;
  friend class TFarPluginGuard;
  NB_DISABLE_COPY(TCustomFarPlugin)
public:
  static inline bool classof(const TObject *Obj) { return Obj->is(OBJECT_CLASS_TCustomFarPlugin); }
  virtual bool is(TObjectClassId Kind) const override { return (Kind == OBJECT_CLASS_TCustomFarPlugin) || TObject::is(Kind); }
public:
  explicit TCustomFarPlugin(TObjectClassId Kind, HINSTANCE HInst);
  virtual ~TCustomFarPlugin();
  virtual void Initialize();
  virtual void Finalize();

  virtual intptr_t GetMinFarVersion() const;
  virtual void SetStartupInfo(const struct PluginStartupInfo *Info);
  virtual const struct PluginStartupInfo *GetPluginStartupInfo() const { return &FStartupInfo; }
  virtual void ExitFAR();
  virtual void GetPluginInfo(struct PluginInfo *Info);
  virtual intptr_t Configure(intptr_t Item);
  virtual void *OpenPlugin(int OpenFrom, intptr_t Item);
  virtual void ClosePlugin(void *Plugin);
  virtual void GetOpenPluginInfo(HANDLE Plugin, struct OpenPluginInfo *Info);
  virtual intptr_t GetFindData(HANDLE Plugin,
    struct PluginPanelItem **PanelItem, int *ItemsNumber, int OpMode);
  virtual void FreeFindData(HANDLE Plugin, struct PluginPanelItem *PanelItem,
    intptr_t ItemsNumber);
  virtual intptr_t ProcessHostFile(HANDLE Plugin,
    struct PluginPanelItem *PanelItem, intptr_t ItemsNumber, int OpMode);
  virtual intptr_t ProcessKey(HANDLE Plugin, int Key, DWORD ControlState);
  virtual intptr_t ProcessEvent(HANDLE Plugin, int Event, void *Param);
  virtual intptr_t SetDirectory(HANDLE Plugin, const wchar_t *Dir, int OpMode);
  virtual intptr_t MakeDirectory(HANDLE Plugin, const wchar_t **Name, int OpMode);
  virtual intptr_t DeleteFiles(HANDLE Plugin, struct PluginPanelItem *PanelItem,
    intptr_t ItemsNumber, int OpMode);
  virtual intptr_t GetFiles(HANDLE Plugin, struct PluginPanelItem *PanelItem,
    intptr_t ItemsNumber, int Move, const wchar_t **DestPath, int OpMode);
  virtual intptr_t PutFiles(HANDLE Plugin, struct PluginPanelItem *PanelItem,
    intptr_t ItemsNumber, int Move, const wchar_t *srcPath, int OpMode);
  virtual intptr_t ProcessEditorEvent(int Event, void *Param);
  virtual intptr_t ProcessEditorInput(const INPUT_RECORD *Rec);

  virtual void HandleException(Exception *E, int OpMode = 0);

  static wchar_t *DuplicateStr(const UnicodeString Str, bool AllowEmpty = false);
  uint32_t Message(DWORD Flags, const UnicodeString Title,
    const UnicodeString Message, TStrings *Buttons = nullptr,
    TFarMessageParams *Params = nullptr);
  intptr_t MaxMessageLines() const;
  intptr_t MaxMenuItemLength() const;
  intptr_t Menu(DWORD Flags, const UnicodeString Title,
    UnicodeString Bottom, TStrings *Items, const int *BreakKeys,
    int &BreakCode);
  intptr_t Menu(DWORD Flags, const UnicodeString Title,
    UnicodeString Bottom, TStrings *Items);
  intptr_t Menu(DWORD Flags, const UnicodeString Title,
    UnicodeString Bottom, const FarMenuItem *Items, intptr_t Count,
    const int *BreakKeys, int &BreakCode);
  bool InputBox(const UnicodeString Title, const UnicodeString Prompt,
    UnicodeString &Text, DWORD Flags, const UnicodeString HistoryName = UnicodeString(),
    intptr_t MaxLen = 255, TFarInputBoxValidateEvent OnValidate = nullptr);
  virtual UnicodeString GetMsg(intptr_t MsgId) const;
  void SaveScreen(HANDLE &Screen);
  void RestoreScreen(HANDLE &Screen);
  bool CheckForEsc() const;
  bool Viewer(const UnicodeString AFileName, const UnicodeString Title, DWORD Flags);
  bool Editor(const UnicodeString AFileName, const UnicodeString Title, DWORD Flags);

  intptr_t FarControl(uintptr_t Command, intptr_t Param1, intptr_t Param2, HANDLE Plugin = INVALID_HANDLE_VALUE);
  intptr_t FarAdvControl(uintptr_t Command, void *Param = nullptr) const;
  intptr_t FarEditorControl(uintptr_t Command, void *Param);
  intptr_t GetFarSystemSettings() const;
  void Text(int X, int Y, int Color, const UnicodeString Str);
  void FlushText();
  void FarWriteConsole(const UnicodeString Str);
  void FarCopyToClipboard(const UnicodeString Str);
  void FarCopyToClipboard(const TStrings *Strings);
  intptr_t GetFarVersion() const;
  UnicodeString FormatFarVersion(intptr_t Version) const;
  UnicodeString GetTemporaryDir() const;
  intptr_t InputRecordToKey(const INPUT_RECORD *Rec) const;
  TFarEditorInfo *EditorInfo();

  void ShowConsoleTitle(const UnicodeString Title);
  void ClearConsoleTitle();
  void UpdateConsoleTitle(const UnicodeString Title);
  void UpdateConsoleTitleProgress(short Progress);
  void ShowTerminalScreen();
  void SaveTerminalScreen();
  void ScrollTerminalScreen(int Rows);
  TPoint TerminalInfo(TPoint *Size = nullptr, TPoint *Cursor = nullptr) const;
  uintptr_t ConsoleWindowState() const;
  void ToggleVideoMode();

  TCustomFarFileSystem *GetPanelFileSystem(bool Another = false,
    HANDLE Plugin = INVALID_HANDLE_VALUE);

  virtual UnicodeString GetModuleName() const;
  TFarDialog *GetTopDialog() const { return FTopDialog; }
  HINSTANCE GetHandle() const { return FHandle; }
  uintptr_t GetFarThreadId() const { return FFarThreadId; }
  const FarStandardFunctions &GetFarStandardFunctions() const { return FFarStandardFunctions; }

protected:
  PluginStartupInfo FStartupInfo;
  FarStandardFunctions FFarStandardFunctions;
  HINSTANCE FHandle;
  TList *FOpenedPlugins;
  TFarDialog *FTopDialog;
  HANDLE FConsoleInput;
  HANDLE FConsoleOutput;
  mutable intptr_t FFarVersion;
  bool FTerminalScreenShowing;
  TCriticalSection FCriticalSection;
  uintptr_t FFarThreadId;
  mutable bool FValidFarSystemSettings;
  mutable intptr_t FFarSystemSettings;
  TPoint FNormalConsoleSize;

  virtual bool HandlesFunction(THandlesFunction Function) const;
  virtual void GetPluginInfoEx(DWORD &Flags,
    TStrings *DiskMenuStrings, TStrings *PluginMenuStrings,
    TStrings *PluginConfigStrings, TStrings *CommandPrefixes) = 0;
  virtual TCustomFarFileSystem *OpenPluginEx(intptr_t OpenFrom, intptr_t Item) = 0;
  virtual bool ConfigureEx(intptr_t Item) = 0;
  virtual intptr_t ProcessEditorEventEx(intptr_t Event, void *Param) = 0;
  virtual intptr_t ProcessEditorInputEx(const INPUT_RECORD *Rec) = 0;
  virtual void HandleFileSystemException(TCustomFarFileSystem *FarFileSystem,
    Exception *E, int OpMode = 0);
  void ResetCachedInfo();
  intptr_t MaxLength(TStrings *Strings) const;
  uint32_t FarMessage(DWORD Flags,
    const UnicodeString Title, const UnicodeString Message, TStrings *Buttons,
    TFarMessageParams *Params);
  uint32_t DialogMessage(DWORD Flags,
    const UnicodeString Title, const UnicodeString Message, TStrings *Buttons,
    TFarMessageParams *Params);
  void InvalidateOpenPluginInfo();

  const TCriticalSection &GetCriticalSection() const { return FCriticalSection; }

#ifdef NETBOX_DEBUG
public:
  void RunTests();
#endif
private:
  void UpdateProgress(intptr_t State, intptr_t Progress) const;

private:
  PluginInfo FPluginInfo;
  TStringList *FSavedTitles;
  UnicodeString FCurrentTitle;
  short FCurrentProgress;

  void ClearPluginInfo(PluginInfo &Info) const;
  void UpdateCurrentConsoleTitle();
  UnicodeString FormatConsoleTitle() const;
  HWND GetConsoleWindow() const;
  RECT GetPanelBounds(HANDLE PanelHandle);
  bool CompareRects(const RECT &lhs, const RECT &rhs) const
  {
    return
      lhs.left == rhs.left &&
      lhs.top == rhs.top &&
      lhs.right == rhs.right &&
      lhs.bottom == rhs.bottom;
  }
  void CloseFileSystem(TCustomFarFileSystem *FileSystem);
};

NB_DEFINE_CLASS_ID(TCustomFarFileSystem);
class TCustomFarFileSystem : public TObject
{
  friend class TFarPanelInfo;
  friend class TCustomFarPlugin;
  NB_DISABLE_COPY(TCustomFarFileSystem)
public:
  static inline bool classof(const TObject *Obj) { return Obj->is(OBJECT_CLASS_TCustomFarFileSystem); }
  virtual bool is(TObjectClassId Kind) const override { return (Kind == OBJECT_CLASS_TCustomFarFileSystem) || TObject::is(Kind); }
public:
  explicit TCustomFarFileSystem(TObjectClassId Kind, TCustomFarPlugin *APlugin);
  void Init();
  virtual ~TCustomFarFileSystem();

  void GetOpenPluginInfo(struct OpenPluginInfo *Info);
  intptr_t GetFindData(struct PluginPanelItem **PanelItem,
    int *ItemsNumber, int OpMode);
  void FreeFindData(struct PluginPanelItem *PanelItem, intptr_t ItemsNumber);
  intptr_t ProcessHostFile(struct PluginPanelItem *PanelItem,
    intptr_t ItemsNumber, int OpMode);
  intptr_t ProcessKey(intptr_t Key, uintptr_t ControlState);
  intptr_t ProcessEvent(intptr_t Event, void *Param);
  intptr_t SetDirectory(const wchar_t *Dir, int OpMode);
  intptr_t MakeDirectory(const wchar_t **Name, int OpMode);
  intptr_t DeleteFiles(struct PluginPanelItem *PanelItem,
    intptr_t ItemsNumber, int OpMode);
  intptr_t GetFiles(struct PluginPanelItem *PanelItem,
    intptr_t ItemsNumber, int Move, const wchar_t **DestPath, int OpMode);
  intptr_t PutFiles(struct PluginPanelItem *PanelItem,
    intptr_t ItemsNumber, int Move, const wchar_t *srcPath, int OpMode);
  virtual void Close();

protected:
  virtual UnicodeString GetCurrDirectory() const = 0;

protected:
  TCustomFarPlugin *FPlugin;
  bool FClosed;

  virtual void GetOpenPluginInfoEx(DWORD &Flags,
    UnicodeString &HostFile, UnicodeString &CurDir, UnicodeString &Format,
    UnicodeString &PanelTitle, TFarPanelModes *PanelModes, int &StartPanelMode,
    int &StartSortMode, bool &StartSortOrder, TFarKeyBarTitles *KeyBarTitles,
    UnicodeString &ShortcutData) = 0;
  virtual bool GetFindDataEx(TObjectList *PanelItems, int OpMode) = 0;
  virtual bool ProcessHostFileEx(TObjectList *PanelItems, int OpMode);
  virtual bool ProcessKeyEx(intptr_t Key, uintptr_t ControlState);
  virtual bool ProcessEventEx(intptr_t Event, void *Param);
  virtual bool SetDirectoryEx(const UnicodeString Dir, int OpMode);
  virtual intptr_t MakeDirectoryEx(UnicodeString &Name, int OpMode);
  virtual bool DeleteFilesEx(TObjectList *PanelItems, int OpMode);
  virtual intptr_t GetFilesEx(TObjectList *PanelItems, bool Move,
    UnicodeString &DestPath, int OpMode);
  virtual intptr_t PutFilesEx(TObjectList *PanelItems, bool Move, int OpMode);

  void ResetCachedInfo();
  intptr_t FarControl(uintptr_t Command, intptr_t Param1, intptr_t Param2);
  intptr_t FarControl(uintptr_t Command, intptr_t Param1, intptr_t Param2, HANDLE Plugin);
  bool UpdatePanel(bool ClearSelection = false, bool Another = false);
  void RedrawPanel(bool Another = false);
  void ClosePlugin();
  UnicodeString GetMsg(intptr_t MsgId) const;
  TCustomFarFileSystem *GetOppositeFileSystem();
  bool IsActiveFileSystem() const;
  bool IsLeft() const;
  bool IsRight() const;

  virtual void HandleException(Exception *E, int OpMode = 0);

  TFarPanelInfo *const *GetPanelInfo() const { return GetPanelInfo(0); }
  TFarPanelInfo **GetPanelInfo() { return GetPanelInfo(0); }
  TFarPanelInfo *const *GetAnotherPanelInfo() const { return GetPanelInfo(1); }
  TFarPanelInfo **GetAnotherPanelInfo() { return GetPanelInfo(1); }
  const TCriticalSection &GetCriticalSection() const { return FCriticalSection; }
  TCriticalSection &GetCriticalSection() { return FCriticalSection; }
  bool GetOpenPluginInfoValid() const { return FOpenPluginInfoValid; }

protected:
  TCriticalSection FCriticalSection;
  void InvalidateOpenPluginInfo();
  TCustomFarFileSystem *GetOwnerFileSystem() { return FOwnerFileSystem; }
  void SetOwnerFileSystem(TCustomFarFileSystem *Value) { FOwnerFileSystem = Value; }

private:
  UnicodeString FNameStr;
  UnicodeString FDestPathStr;
  OpenPluginInfo FOpenPluginInfo;
  TCustomFarFileSystem *FOwnerFileSystem;
  bool FOpenPluginInfoValid;
  TFarPanelInfo *FPanelInfo[2];
  static uintptr_t FInstances;

  void ClearOpenPluginInfo(OpenPluginInfo &Info);
  TObjectList *CreatePanelItemList(struct PluginPanelItem *PanelItem, intptr_t ItemsNumber);
  TFarPanelInfo *const *GetPanelInfo(int Another) const;
  TFarPanelInfo **GetPanelInfo(int Another);
};

#define PANEL_MODES_COUNT 10
class TFarPanelModes : public TObject
{
  friend class TCustomFarFileSystem;
public:
  TFarPanelModes();
  virtual ~TFarPanelModes();

  void SetPanelMode(size_t Mode, const UnicodeString ColumnTypes = UnicodeString(),
    const UnicodeString ColumnWidths = UnicodeString(), TStrings *ColumnTitles = nullptr,
    bool FullScreen = false, bool DetailedStatus = true, bool AlignExtensions = true,
    bool CaseConversion = true, const UnicodeString StatusColumnTypes = UnicodeString(),
    const UnicodeString StatusColumnWidths = UnicodeString());

private:
  PanelMode FPanelModes[PANEL_MODES_COUNT];
  bool FReferenced;

  void FillOpenPluginInfo(struct OpenPluginInfo *Info);
  static void ClearPanelMode(PanelMode &Mode);
  static intptr_t CommaCount(const UnicodeString ColumnTypes);
};

class TFarKeyBarTitles : public TObject
{
  friend class TCustomFarFileSystem;
public:
  TFarKeyBarTitles();
  virtual ~TFarKeyBarTitles();

  void ClearFileKeyBarTitles();
  void ClearKeyBarTitle(TFarShiftStatus ShiftStatus,
    intptr_t FunctionKeyStart, intptr_t FunctionKeyEnd = 0);
  void SetKeyBarTitle(TFarShiftStatus ShiftStatus, intptr_t FunctionKey,
    UnicodeString Title);

private:
  KeyBarTitles FKeyBarTitles;
  bool FReferenced;

  void FillOpenPluginInfo(struct OpenPluginInfo *Info);
  static void ClearKeyBarTitles(KeyBarTitles &Titles);
};

NB_DEFINE_CLASS_ID(TCustomFarPanelItem);
class TCustomFarPanelItem : public TObject
{
  friend class TCustomFarFileSystem;
public:
  static inline bool classof(const TObject *Obj) { return Obj->is(OBJECT_CLASS_TCustomFarPanelItem); }
  virtual bool is(TObjectClassId Kind) const override { return (Kind == OBJECT_CLASS_TCustomFarPanelItem) || TObject::is(Kind); }
protected:
  explicit TCustomFarPanelItem(TObjectClassId Kind) : TObject(Kind) {}
  virtual ~TCustomFarPanelItem()
  {
  }
  virtual void GetData(
    DWORD &Flags, UnicodeString &AFileName, int64_t &Size,
    DWORD &FileAttributes,
    TDateTime &LastWriteTime, TDateTime &LastAccess,
    DWORD &NumberOfLinks, UnicodeString &Description,
    UnicodeString &Owner, void *&UserData, int &CustomColumnNumber) = 0;
  virtual UnicodeString GetCustomColumnData(size_t Column);

  void FillPanelItem(struct PluginPanelItem *PanelItem);
};

NB_DEFINE_CLASS_ID(TFarPanelItem);
class TFarPanelItem : public TCustomFarPanelItem
{
  NB_DISABLE_COPY(TFarPanelItem)
public:
  static inline bool classof(const TObject *Obj) { return Obj->is(OBJECT_CLASS_TFarPanelItem); }
  virtual bool is(TObjectClassId Kind) const override { return (Kind == OBJECT_CLASS_TFarPanelItem) || TCustomFarPanelItem::is(Kind); }
public:
  explicit TFarPanelItem(PluginPanelItem *APanelItem, bool OwnsItem);
  virtual ~TFarPanelItem();

  uintptr_t GetFlags() const;
  uintptr_t GetFileAttrs() const;
  UnicodeString GetFileName() const;
  void *GetUserData() const;
  bool GetSelected() const;
  void SetSelected(bool Value);
  bool GetIsParentDirectory() const;
  bool GetIsFile() const;

protected:
  PluginPanelItem *FPanelItem;
  bool FOwnsItem;

  virtual void GetData(
    DWORD &Flags, UnicodeString &AFileName, int64_t &Size,
    DWORD &FileAttributes,
    TDateTime &LastWriteTime, TDateTime &LastAccess,
    DWORD &NumberOfLinks, UnicodeString &Description,
    UnicodeString &Owner, void *&UserData, int &CustomColumnNumber) override;
  virtual UnicodeString GetCustomColumnData(size_t Column) override;
};

NB_DEFINE_CLASS_ID(THintPanelItem);
class THintPanelItem : public TCustomFarPanelItem
{
public:
  explicit THintPanelItem(const UnicodeString AHint);
  virtual ~THintPanelItem() {}

protected:
  virtual void GetData(
    DWORD &Flags, UnicodeString &AFileName, int64_t &Size,
    DWORD &FileAttributes,
    TDateTime &LastWriteTime, TDateTime &LastAccess,
    DWORD &NumberOfLinks, UnicodeString &Description,
    UnicodeString &Owner, void *&UserData, int &CustomColumnNumber) override;

private:
  UnicodeString FHint;
};

enum TFarPanelType
{
  ptFile,
  ptTree,
  ptQuickView,
  ptInfo
};

class TFarPanelInfo : public TObject
{
  NB_DISABLE_COPY(TFarPanelInfo)
public:
  explicit TFarPanelInfo(PanelInfo *APanelInfo, TCustomFarFileSystem *AOwner);
  virtual ~TFarPanelInfo();

  const TObjectList *GetItems() const { return const_cast<TFarPanelInfo *>(this)->GetItems(); }
  TObjectList *GetItems();
  intptr_t GetItemCount() const;
  TFarPanelItem *GetFocusedItem() const;
  void SetFocusedItem(const TFarPanelItem *Value);
  intptr_t GetFocusedIndex() const;
  void SetFocusedIndex(intptr_t Value);
  intptr_t GetSelectedCount(bool CountCurrentItem = false) const;
  TRect GetBounds() const;
  TFarPanelType GetType() const;
  bool GetIsPlugin() const;
  UnicodeString GetCurrDirectory() const;

  void ApplySelection();
  TFarPanelItem *FindFileName(const UnicodeString AFileName) const;
  const TFarPanelItem *FindUserData(const void *UserData) const;
  TFarPanelItem *FindUserData(const void *UserData);

private:
  PanelInfo *FPanelInfo;
  TObjectList *FItems;
  TCustomFarFileSystem *FOwner;
};

NB_DEFINE_CLASS_ID(TFarMenuItems);
class TFarMenuItems : public TStringList
{
public:
  static inline bool classof(const TObject *Obj) { return Obj->is(OBJECT_CLASS_TFarMenuItems); }
  virtual bool is(TObjectClassId Kind) const override { return (Kind == OBJECT_CLASS_TFarMenuItems) || TStringList::is(Kind); }
public:
  explicit TFarMenuItems();
  virtual ~TFarMenuItems() {}
  void AddSeparator(bool Visible = true);
  virtual intptr_t Add(const UnicodeString Text, bool Visible = true);

  virtual void Clear() override;
  virtual void Delete(intptr_t Index) override;

  intptr_t GetItemFocused() const { return FItemFocused; }
  void SetItemFocused(intptr_t Value);
  bool GetDisabled(intptr_t Index) const { return GetFlag(Index, MIF_DISABLE); }
  void SetDisabled(intptr_t Index, bool Value) { SetFlag(Index, MIF_DISABLE, Value); }
  bool GetChecked(intptr_t Index) const { return GetFlag(Index, MIF_CHECKED); }
  void SetChecked(intptr_t Index, bool Value) { SetFlag(Index, MIF_CHECKED, Value); }

  void SetFlag(intptr_t Index, uintptr_t Flag, bool Value);
  bool GetFlag(intptr_t Index, uintptr_t Flag) const;

protected:
  virtual void SetObj(intptr_t Index, TObject *AObject) override;

private:
  intptr_t FItemFocused;
};

class TFarEditorInfo : public TObject
{
  NB_DISABLE_COPY(TFarEditorInfo)
public:
  explicit TFarEditorInfo(EditorInfo *Info);
  ~TFarEditorInfo();

  intptr_t GetEditorID() const;
  static UnicodeString GetFileName();

private:
  EditorInfo *FEditorInfo;
};

class TFarEnvGuard // : public TObject
{
  CUSTOM_MEM_ALLOCATION_IMPL
  NB_DISABLE_COPY(TFarEnvGuard)
public:
  TFarEnvGuard();
  ~TFarEnvGuard();
};

class TFarPluginEnvGuard // : public TObject
{
  CUSTOM_MEM_ALLOCATION_IMPL
  NB_DISABLE_COPY(TFarPluginEnvGuard)
public:
  TFarPluginEnvGuard();
  ~TFarPluginEnvGuard();
};

extern TCustomFarPlugin *FarPlugin;

class TGlobalFunctions : public TGlobals
{
public:
  virtual HINSTANCE GetInstanceHandle() const override;
  virtual UnicodeString GetMsg(intptr_t Id) const override;
  virtual UnicodeString GetCurrDirectory() const override;
  virtual UnicodeString GetStrVersionNumber() const override;
  virtual bool InputDialog(const UnicodeString ACaption,
    const UnicodeString APrompt, UnicodeString &Value, const UnicodeString HelpKeyword,
    TStrings *History, bool PathInput,
    TInputDialogInitializeEvent OnInitialize, bool Echo) override;
  virtual uintptr_t MoreMessageDialog(const UnicodeString AMessage,
    TStrings *MoreMessages, TQueryType Type, uint32_t Answers,
    const TMessageParams *Params) override;
};

