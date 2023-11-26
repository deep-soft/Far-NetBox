﻿
#include <vcl.h>
#pragma hdrstop

// #include "ScpCommander.h"
// #include "ScpExplorer.h"

#include <CoreMain.h>
#include <Common.h>
#include <Exceptions.h>
#include <Cryptography.h>
#include "ProgParams.h"
#include "VCLCommon.h"
#include "WinConfiguration.h"
// #include "TerminalManager.h"
#include "TextsWin.h"
#include "WinInterface.h"
// #include "PasswordEdit.hpp"
#include "ProgParams.h"
#include "Tools.h"
// #include "Custom.h"
#include "HelpWin.h"
#include <Math.hpp>
#include <PasTools.hpp>
#include <GUITools.h>

#if 0

#pragma package(smart_init)

const UnicodeString AppName = L"WinSCP";

TConfiguration * CreateConfiguration()
{
  WinConfiguration = new TWinConfiguration();
  CustomWinConfiguration = WinConfiguration;
  GUIConfiguration = CustomWinConfiguration;

  TProgramParams * Params = TProgramParams::Instance();
  UnicodeString IniFileName = Params->SwitchValue(INI_SWITCH);
  if (!IniFileName.IsEmpty())
  {
    if (SameText(IniFileName, INI_NUL))
    {
      WinConfiguration->SetNulStorage();
    }
    else if (CheckSafe(Params))
    {
      IniFileName = ExpandFileName(ExpandEnvironmentVariables(IniFileName));
      WinConfiguration->SetExplicitIniFileStorageName(IniFileName);
    }
  }

  if (CheckSafe(Params))
  {
    std::unique_ptr<TStrings> RawConfig(std::make_unique<TStringList>());
    if (Params->FindSwitch(RAW_CONFIG_SWITCH, RawConfig.get()))
    {
      WinConfiguration->OptionsStorage = RawConfig.get();
    }
  }

  return WinConfiguration;
}
#endif // #if 0

TOptions * GetGlobalOptions()
{
  static TOptions Options;
  return &Options; // TProgramParams::Instance();
}

#if 0

TCustomScpExplorerForm * CreateScpExplorer()
{
  TCustomScpExplorerForm * ScpExplorer;
  if (WinConfiguration->Interface == ifExplorer)
  {
    ScpExplorer = SafeFormCreate<TScpExplorerForm>();
  }
  else
  {
    ScpExplorer = SafeFormCreate<TScpCommanderForm>();
  }
  return ScpExplorer;
}

UnicodeString SshVersionString()
{
  return FORMAT("WinSCP-release-%s", Configuration->Version);
}

UnicodeString AppNameString()
{
  return L"WinSCP";
}

UnicodeString GetCompanyRegistryKey()
{
  return L"Software\\Martin Prikryl";
}

UnicodeString GetRegistryKey()
{
  return GetCompanyRegistryKey() + L"\\WinSCP 2";
}

static bool ForcedOnForeground = false;
void SetOnForeground(bool OnForeground)
{
  ForcedOnForeground = OnForeground;
}

void FlashOnBackground()
{
  DebugAssert(Application);
  if ((WinConfiguration != nullptr) && WinConfiguration->FlashTaskbar && !ForcedOnForeground && !ForegroundTask())
  {
    FlashWindow(Application->MainFormHandle, true);
  }
}

void LocalSystemSettings(TCustomForm * /*Control*/)
{
  // noop
}

void ShowExtendedException(Exception * E)
{
  ShowExtendedExceptionEx(nullptr, E);
}

void TerminateApplication()
{
  Application->Terminate();
}

struct TOpenLocalPathHandler
{
  UnicodeString LocalPath;
  UnicodeString LocalFileName;

  void Open(TObject * Sender, unsigned int & /*Answer*/)
  {
    TButton * Button = DebugNotNull(dynamic_cast<TButton *>(Sender));
    // Reason for separate AMenu variable is given in TPreferencesDialog::EditorFontColorButtonClick
    TPopupMenu * AMenu = new TPopupMenu(Application);
    // Popup menu has to survive the popup as TBX calls click handler asynchronously (post).
    Menu.reset(AMenu);
    TMenuItem * Item;

    Item = new TMenuItem(Menu.get());
    Menu->Items->Add(Item);
    Item->Caption = LoadStr(OPEN_TARGET_FOLDER);
    Item->OnClick = OpenFolderClick;

    if (!LocalFileName.IsEmpty())
    {
      Item = new TMenuItem(Menu.get());
      Menu->Items->Add(Item);
      Item->Caption = LoadStr(OPEN_DOWNLOADED_FILE);
      Item->OnClick = OpenFileClick;
    }

    MenuPopup(Menu.get(), Button);
  }

private:
  std::unique_ptr<TPopupMenu> Menu;

  void OpenFolderClick(TObject * /*Sender*/)
  {
    if (LocalFileName.IsEmpty())
    {
      OpenFolderInExplorer(LocalPath);
    }
    else
    {
      OpenFileInExplorer(LocalFileName);
    }
  }

  void OpenFileClick(TObject * /*Sender*/)
  {
    ExecuteShellChecked(LocalFileName, L"");
  }
};

void ShowExtendedExceptionEx(TTerminal * Terminal,
  Exception * E)
{
  bool Show = ShouldDisplayException(E);
  bool DoNotDisplay = false;

  try
  {
    // This is special case used particularly when called from .NET assembly
    // (which always uses /nointeractiveinput),
    // but can be useful for other console runs too
    TProgramParams * Params = TProgramParams::Instance();
    if (Params->FindSwitch(NOINTERACTIVEINPUT_SWITCH))
    {
      DoNotDisplay = true;
      if (Show && CheckXmlLogParam(Params))
      {
        // The Started argument won't be used with .NET assembly, as it never uses patterns in XML log file name.
        // But it theoretically can be used, when started manually.
        std::unique_ptr<TActionLog> ActionLog(std::make_unique<TActionLog>(Now(), Configuration));
        ActionLog->AddFailure(E);
        // unnecessary explicit release
        ActionLog.reset(nullptr);
      }
    }
  }
  catch (Exception & E)
  {
    // swallow
  }

  TTerminalManager * Manager = TTerminalManager::Instance(false);
  bool HookedDialog = false;
  try
  {
    if (!DoNotDisplay)
    {
      ESshTerminate * Terminate = dynamic_cast<ESshTerminate*>(E);
      bool CloseOnCompletion = (Terminate != nullptr);

      bool ForActiveTerminal =
        E->InheritsFrom(__classid(EFatal)) && (Terminal != nullptr) &&
        (Manager != nullptr) && (Manager->ActiveTerminal == Terminal);

      unsigned int Result;
      if (CloseOnCompletion)
      {
        if (ForActiveTerminal)
        {
          DebugAssert(!Terminal->Active);
          Manager->DisconnectActiveTerminal();
        }

        if (Terminate->Operation == odoSuspend)
        {
          // suspend, so that exit prompt is shown only after windows resume
          SuspendWindows();
        }

        DebugAssert(Show);
        bool ConfirmExitOnCompletion =
          CloseOnCompletion &&
          ((Terminate->Operation == odoDisconnect) || (Terminate->Operation == odoSuspend)) &&
          WinConfiguration->ConfirmExitOnCompletion;

        if (ConfirmExitOnCompletion)
        {
          TMessageParams Params(mpNeverAskAgainCheck);
          unsigned int Answers = 0;
          TQueryButtonAlias Aliases[1];
          TOpenLocalPathHandler OpenLocalPathHandler;
          if (!Terminate->TargetLocalPath.IsEmpty() && !ForActiveTerminal)
          {
            OpenLocalPathHandler.LocalPath = Terminate->TargetLocalPath;
            OpenLocalPathHandler.LocalFileName = Terminate->DestLocalFileName;

            Aliases[0].Button = qaIgnore;
            Aliases[0].Alias = LoadStr(OPEN_BUTTON);
            Aliases[0].OnSubmit = OpenLocalPathHandler.Open;
            Aliases[0].MenuButton = true;
            Answers |= Aliases[0].Button;
            Params.Aliases = Aliases;
            Params.AliasesCount = LENOF(Aliases);
          }

          if (ForActiveTerminal)
          {
            UnicodeString MessageFormat =
              (Manager->Count > 1) ?
                FMTLOAD(DISCONNECT_ON_COMPLETION, Manager->Count - 1) :
                LoadStr(EXIT_ON_COMPLETION);
            // Remove the leading "%s\n\n" (not to change the translation originals - previously the error message was prepended)
            MessageFormat = FORMAT(MessageFormat, UnicodeString()).Trim();
            MessageFormat = MainInstructions(MessageFormat) + L"\n\n%s";
            Result = FatalExceptionMessageDialog(E, qtInformation,
              MessageFormat,
              Answers | qaYes | qaNo, HELP_NONE, &Params);
          }
          else
          {
            Result =
              ExceptionMessageDialog(E, qtInformation, L"", Answers | qaOK, HELP_NONE, &Params);
          }
        }
        else
        {
          Result = qaYes;
        }
      }
      else
      {
        if (Show)
        {
          if (ForActiveTerminal)
          {
            TMessageParams Params;
            if (DebugAlwaysTrue(Manager->ActiveTerminal != nullptr) &&
                ((Configuration->SessionReopenTimeout == 0) ||
                 ((double)Manager->ActiveTerminal->ReopenStart == 0) ||
                 (nb::ToInt32(double(Now() - Manager->ActiveTerminal->ReopenStart) * MSecsPerDay) < Configuration->SessionReopenTimeout)))
            {
              Params.Timeout = GUIConfiguration->SessionReopenAutoIdle;
              Params.TimeoutAnswer = qaRetry;
              Params.TimeoutResponse = Params.TimeoutAnswer;
              HookedDialog = Manager->HookFatalExceptionMessageDialog(Params);
            }

            Result = FatalExceptionMessageDialog(E, qtError, EmptyStr, qaOK, EmptyStr, &Params);
          }
          else
          {
            Result = ExceptionMessageDialog(E, qtError);
          }
        }
        else
        {
          Result = qaOK;
        }
      }

      if (Result == qaNeverAskAgain)
      {
        DebugAssert(CloseOnCompletion);
        Result = qaYes;
        WinConfiguration->ConfirmExitOnCompletion = false;
      }

      if (Result == qaYes)
      {
        DebugAssert(CloseOnCompletion);
        DebugAssert(Terminate != nullptr);
        DebugAssert(Terminate->Operation != odoIdle);
        TerminateApplication();

        switch (Terminate->Operation)
        {
          case odoDisconnect:
            break;

          case odoSuspend:
            // suspended before already
            break;

          case odoShutDown:
            ShutDownWindows();
            break;

          default:
            DebugFail();
        }
      }
      else if (Result == qaRetry)
      {
        // qaRetry is used by FatalExceptionMessageDialog
        if (DebugAlwaysTrue(ForActiveTerminal))
        {
          Manager->ReconnectActiveTerminal();
        }
      }
      else
      {
        if (ForActiveTerminal)
        {
          Manager->DisconnectActiveTerminalIfPermanentFreeOtherwise();
        }
      }
    }
  }
  __finally
  {
    if (HookedDialog)
    {
      Manager->UnhookFatalExceptionMessageDialog();
    }
  }
}

void ShowNotification(TTerminal * Terminal, const UnicodeString & Str,
  TQueryType Type)
{
  TTerminalManager * Manager = TTerminalManager::Instance(false);
  DebugAssert(Manager != nullptr);

  Manager->ScpExplorer->PopupTrayBalloon(Terminal, Str, Type);
}

UnicodeString GetThemeName(bool Dark)
{
  return Dark ? L"DarkOfficeXP" : L"OfficeXP";
}

void ConfigureInterface()
{
  DebugAssert(WinConfiguration != nullptr);
  int BidiModeFlag =
    AdjustLocaleFlag(LoadStr(BIDI_MODE), WinConfiguration->BidiModeOverride, false, bdRightToLeft, bdLeftToRight);
  Application->BiDiMode = static_cast<TBiDiMode>(BidiModeFlag);
  SetTBXSysParam(TSP_XPVISUALSTYLE, XPVS_AUTOMATIC);
  UnicodeString Theme = GetThemeName(WinConfiguration->UseDarkTheme());
  if (!SameText(TBXCurrentTheme(), Theme))
  {
    TBXSetTheme(Theme);
  }
  // Has any effect on Wine only
  // (otherwise initial UserDocumentDirectory is equivalent to GetPersonalFolder())
  UserDocumentDirectory = GetPersonalFolder();
}

void DoAboutDialog(TConfiguration *Configuration)
{
  DoAboutDialog(Configuration, true, nullptr);
}

void DoProductLicense()
{
  DoLicenseDialog(lcWinScp);
}

const UnicodeString PixelsPerInchKey = L"PixelsPerInch";

int GetToolbarLayoutPixelsPerInch(TStrings * Storage, TControl * Control)
{
  int Result;
  if (Storage->IndexOfName(PixelsPerInchKey))
  {
    Result = LoadPixelsPerInch(Storage->Values[PixelsPerInchKey], Control);
  }
  else
  {
    Result = -1;
  }
  return Result;
}

UnicodeString GetToolbarKey(const UnicodeString & ToolbarName)
{
  UnicodeString Result = ToolbarName;
  Result = RemoveSuffix(Result, L"Toolbar", true);
  return Result;
}

static inline void GetToolbarKey(const UnicodeString & ToolbarName,
  const UnicodeString & Value, UnicodeString & ToolbarKey)
{
  ToolbarKey = GetToolbarKey(ToolbarName);
  if (!Value.IsEmpty())
  {
    ToolbarKey += L"_" + Value;
  }
}

static int ToolbarReadInt(const UnicodeString & ToolbarName,
  const UnicodeString & Value, const int Default, const void * ExtraData)
{
  int Result;
  if (Value == L"Rev")
  {
    Result = 2000;
  }
  else
  {
    TStrings * Storage = static_cast<TStrings *>(const_cast<void*>(ExtraData));
    UnicodeString ToolbarKey;
    GetToolbarKey(ToolbarName, Value, ToolbarKey);
    if (Storage->IndexOfName(ToolbarKey) >= 0)
    {
      Result = StrToIntDef(Storage->Values[ToolbarKey], Default);
      #if 0
      // this does not work well, as it scales down the stretched
      // toolbars (path toolbars) too much, it has to be reimplemented smarter
      if (Value == L"DockPos")
      {
        int PixelsPerInch = GetToolbarLayoutPixelsPerInch(Storage);
        // When DPI has decreased since the last time, scale down
        // toolbar position to get rid of gaps caused by smaller labels.
        // Do not do this when DPI has increased as it would introduce gaps,
        // as toolbars consists mostly of icons only, that do not scale.
        // The toolbars shift themselves anyway, when other toolbars to the left
        // get wider. We also risk a bit that toolbar order changes,
        // as with very small toolbars (History) we can get scaled down position
        // of the following toolbar to the left of it.
        // There's special handling (also for scaling-up) stretched toolbars
        // in LoadToolbarsLayoutStr
        if ((PixelsPerInch > 0) && (Screen->PixelsPerInch < PixelsPerInch))
        {
          Result = LoadDimension(Result, PixelsPerInch);
        }
      }
      #endif
    }
    else
    {
      Result = Default;
    }
  }
  return Result;
}

static UnicodeString ToolbarReadString(const UnicodeString & ToolbarName,
  const UnicodeString Value, const UnicodeString Default, const void * ExtraData)
{
  UnicodeString Result;
  TStrings * Storage = static_cast<TStrings *>(const_cast<void*>(ExtraData));
  UnicodeString ToolbarKey;
  GetToolbarKey(ToolbarName, Value, ToolbarKey);
  if (Storage->IndexOfName(ToolbarKey) >= 0)
  {
    Result = Storage->Values[ToolbarKey];
  }
  else
  {
    Result = Default;
  }
  return Result;
}

static void ToolbarWriteInt(const UnicodeString ToolbarName,
  const UnicodeString Value, const int Data, const void * ExtraData)
{
  DebugFail();
  if (Value != L"Rev")
  {
    TStrings * Storage = static_cast<TStrings *>(const_cast<void*>(ExtraData));
    UnicodeString ToolbarKey;
    GetToolbarKey(ToolbarName, Value, ToolbarKey);
    DebugAssert(Storage->IndexOfName(ToolbarKey) < 0);
    Storage->Values[ToolbarKey] = IntToStr(Data);
  }
}

static void ToolbarWriteString(const UnicodeString ToolbarName,
  const UnicodeString Value, const UnicodeString Data, const void * ExtraData)
{
  DebugAssert(Value.IsEmpty());
  TStrings * Storage = static_cast<TStrings *>(const_cast<void*>(ExtraData));
  UnicodeString ToolbarKey;
  GetToolbarKey(ToolbarName, Value, ToolbarKey);
  DebugAssert(Storage->IndexOfName(ToolbarKey) < 0);
  Storage->Values[ToolbarKey] = Data;
}

UnicodeString GetToolbarsLayoutStr(TControl * OwnerControl)
{
  UnicodeString Result;
  TStrings * Storage = new TStringList();
  try
  {
    TBCustomSavePositions(OwnerControl, ToolbarWriteInt, ToolbarWriteString,
      Storage);
    Storage->Values[PixelsPerInchKey] = SavePixelsPerInch(OwnerControl);
    Result = Storage->CommaText;
  }
  __finally
  {
    delete Storage;
  }
  return Result;
}

void LoadToolbarsLayoutStr(TControl * OwnerControl, const UnicodeString & LayoutStr)
{
  TStrings * Storage = CommaTextToStringList(LayoutStr);
  try
  {
    TBCustomLoadPositions(OwnerControl, ToolbarReadInt, ToolbarReadString,
      Storage);
    int PixelsPerInch = GetToolbarLayoutPixelsPerInch(Storage, OwnerControl);
    // Scale toolbars stretched to the first other toolbar to the right
    if ((PixelsPerInch > 0) && (PixelsPerInch != GetControlPixelsPerInch(OwnerControl))) // optimization
    {
      for (int Index = 0; Index < OwnerControl->ComponentCount; Index++)
      {
        TTBXToolbar * Toolbar =
          dynamic_cast<TTBXToolbar *>(OwnerControl->Components[Index]);
        if ((Toolbar != nullptr) && Toolbar->Stretch &&
            (Toolbar->OnGetBaseSize != nullptr) &&
            // we do not support floating of stretched toolbars
            DebugAlwaysTrue(!Toolbar->Floating))
        {
          TTBXToolbar * FollowingToolbar = nullptr;
          for (int Index2 = 0; Index2 < OwnerControl->ComponentCount; Index2++)
          {
            TTBXToolbar * Toolbar2 =
              dynamic_cast<TTBXToolbar *>(OwnerControl->Components[Index2]);
            if ((Toolbar2 != nullptr) && !Toolbar2->Floating &&
                (Toolbar2->Parent == Toolbar->Parent) &&
                (Toolbar2->DockRow == Toolbar->DockRow) &&
                (Toolbar2->DockPos > Toolbar->DockPos) &&
                ((FollowingToolbar == nullptr) || (FollowingToolbar->DockPos > Toolbar2->DockPos)))
            {
              FollowingToolbar = Toolbar2;
            }
          }

          if (FollowingToolbar != nullptr)
          {
            int NewWidth = LoadDimension(Toolbar->Width, PixelsPerInch, Toolbar);
            FollowingToolbar->DockPos += NewWidth - Toolbar->Width;
          }
        }
      }
    }
  }
  __finally
  {
    delete Storage;
  }
}

TTBXSeparatorItem * AddMenuSeparator(TTBCustomItem * Menu)
{
  TTBXSeparatorItem * Item = new TTBXSeparatorItem(Menu);
  Menu->Add(Item);
  return Item;
}

static TComponent * LastPopupComponent = nullptr;
static TRect LastPopupRect(-1, -1, -1, -1);
static TDateTime LastCloseUp;

static void ConvertMenu(TMenuItem * AItems, TTBCustomItem * Items)
{
  for (int Index = 0; Index < AItems->Count; Index++)
  {
    TMenuItem * AItem = AItems->Items[Index];
    TTBCustomItem * Item;

    if (!AItem->Enabled && !AItem->Visible && (AItem->Action == nullptr) &&
        (AItem->OnClick == nullptr) && DebugAlwaysTrue(AItem->Count == 0))
    {
      TTBXLabelItem * LabelItem = new TTBXLabelItem(Items->Owner);
      // TTBXLabelItem has it's own Caption
      LabelItem->Caption = AItem->Caption;
      LabelItem->SectionHeader = true;
      Item = LabelItem;
    }
    else
    {
      // see TB2DsgnConverter.pas DoConvert
      if (AItem->Caption == L"-")
      {
        Item = new TTBXSeparatorItem(Items->Owner);
      }
      else
      {
        if (AItem->Count > 0)
        {
          Item = new TTBXSubmenuItem(Items->Owner);
        }
        else
        {
          Item = new TTBXItem(Items->Owner);
        }
        Item->Action = AItem->Action;
        Item->AutoCheck = AItem->AutoCheck;
        Item->Caption = AItem->Caption;
        Item->Checked = AItem->Checked;
        if (AItem->Default)
        {
          Item->Options = Item->Options << tboDefault;
        }
        Item->Enabled = AItem->Enabled;
        Item->GroupIndex = AItem->GroupIndex;
        Item->HelpContext = AItem->HelpContext;
        Item->ImageIndex = AItem->ImageIndex;
        Item->RadioItem = AItem->RadioItem;
        Item->ShortCut = AItem->ShortCut;
        Item->SubMenuImages = AItem->SubMenuImages;
        Item->OnClick = AItem->OnClick;
      }
      Item->Hint = AItem->Hint;
      Item->Tag = AItem->Tag;
      Item->Visible = AItem->Visible;

      // recurse is supported only for empty submenus (as used for custom commands)
      if (AItem->Count > 0)
      {
        ConvertMenu(AItem, Item);
      }
    }

    Items->Add(Item);
  }
}

void MenuPopup(TPopupMenu * AMenu, TRect Rect,
  TComponent * PopupComponent)
{
  // Pressing the same button within 200ms after closing its popup menu
  // does nothing.
  // It is to immitate close-by-click behavior. Note that menu closes itself
  // before onclick handler of button occurs.
  // To support content menu popups, we have to check for the popup location too,
  // to allow poping menu on different location (such as different node of TTreeView),
  // even if there's another popup opened already (so that the time interval
  // below does not elapse).
  TDateTime N = Now();
  TDateTime Diff = N - LastCloseUp;
  if ((PopupComponent == LastPopupComponent) &&
      (Rect == LastPopupRect) &&
      (Diff < TDateTime(0, 0, 0, 200)))
  {
    LastPopupComponent = nullptr;
  }
  else
  {
    TTBXPopupMenu * Menu = dynamic_cast<TTBXPopupMenu *>(AMenu);
    if (Menu == nullptr)
    {
      Menu = CreateTBXPopupMenu(AMenu->Owner);
      Menu->OnPopup = AMenu->OnPopup;
      Menu->Items->SubMenuImages = AMenu->Images;

      ConvertMenu(AMenu->Items, Menu->Items);
    }

    Menu->PopupComponent = PopupComponent;
    Menu->PopupEx(Rect);

    LastPopupComponent = PopupComponent;
    LastPopupRect = Rect;
    LastCloseUp = Now();
  }
}

const int ColorCols = 8;
const int StandardColorRows = 2;
const int StandardColorCount = ColorCols * StandardColorRows;
const int UserColorRows = 1;
const int UserColorCount = UserColorRows * ColorCols;
const wchar_t ColorSeparator = L',';

static void GetStandardSessionColorInfo(
  int Col, int Row, TColor & Color, UnicodeString & Name)
{
  #define COLOR_INFO(COL, ROW, NAME, COLOR) \
    if ((Col == COL) && (Row == ROW)) { Name = NAME; Color = TColor(COLOR); } else
  // bottom row of default TBX color set
  COLOR_INFO(0, 0, L"Rose",              0xCC99FF)
  COLOR_INFO(1, 0, L"Tan",               0x99CCFF)
  COLOR_INFO(2, 0, L"Light Yellow",      0x99FFFF)
  COLOR_INFO(3, 0, L"Light Green",       0xCCFFCC)
  COLOR_INFO(4, 0, L"Light Turquoise",   0xFFFFCC)
  COLOR_INFO(5, 0, L"Pale Blue",         0xFFCC99)
  COLOR_INFO(6, 0, L"Lavender",          0xFF99CC)

  // second row of Excel 2010 palette with second color (Lighter Black) skipped
  COLOR_INFO(7, 0, L"Light Orange",      0xB5D5FB)

  COLOR_INFO(0, 1, L"Darker White",      0xD8D8D8)
  COLOR_INFO(1, 1, L"Darker Tan",        0x97BDC4)
  COLOR_INFO(2, 1, L"Lighter Blue",      0xE2B38D)
  COLOR_INFO(3, 1, L"Light Blue",        0xE4CCB8)
  COLOR_INFO(4, 1, L"Lighter Red",       0xB7B9E5)
  COLOR_INFO(5, 1, L"Light Olive Green", 0xBCE3D7)
  COLOR_INFO(6, 1, L"Light Purple",      0xD9C1CC)
  COLOR_INFO(7, 1, L"Light Aqua",        0xE8DDB7)

  DebugFail();
  #undef COLOR_INFO
}

static void SessionColorSetGetColorInfo(
  void * /*Data*/, TTBXCustomColorSet * /*Sender*/, int Col, int Row, TColor & Color, UnicodeString & Name)
{
  GetStandardSessionColorInfo(Col, Row, Color, Name);
}

TColor RestoreColor(const UnicodeString & CStr)
{
  return TColor(StrToInt(UnicodeString(L"$") + CStr));
}

UnicodeString StoreColor(TColor Color)
{
  return IntToHex(Color, 6);
}

static UnicodeString ExtractColorStr(UnicodeString & Colors)
{
  return CutToChar(Colors, ColorSeparator, true);
}

static bool IsStandardColor(bool SessionColors, TColor Color)
{
  if (SessionColors)
  {
    for (int Row = 0; Row < StandardColorRows; Row++)
    {
      for (int Col = 0; Col < ColorCols; Col++)
      {
        TColor StandardColor;
        UnicodeString Name; // unused
        GetStandardSessionColorInfo(Col, Row, StandardColor, Name);
        if (StandardColor == Color)
        {
          return true;
        }
      }
    }
    return false;
  }
  else
  {
    std::unique_ptr<TTBXColorPalette> DefaultColorPalette(new TTBXColorPalette(nullptr));
    return (DefaultColorPalette->FindCell(Color).X >= 0);
  }
}

class TColorChangeData : public TComponent
{
public:
  TColorChangeData(TColorChangeEvent OnColorChange, TColor Color, bool SessionColors);

  static TColorChangeData * Retrieve(TObject * Object);

  void ColorChange(TColor Color);

  __property TColor Color = { read = FColor };

  __property bool SessionColors = { read = FSessionColors };

private:
  TColorChangeEvent FOnColorChange;
  TColor FColor;
  bool FSessionColors;
};

TColorChangeData::TColorChangeData(
    TColorChangeEvent OnColorChange, TColor Color, bool SessionColors) :
  TComponent(nullptr)
{
  Name = QualifiedClassName();
  FOnColorChange = OnColorChange;
  FColor = Color;
  FSessionColors = SessionColors;
}

TColorChangeData * TColorChangeData::Retrieve(TObject * Object)
{
  TComponent * Component = DebugNotNull(dynamic_cast<TComponent *>(Object));
  TComponent * ColorChangeDataComponent = Component->FindComponent(QualifiedClassName());
  return DebugNotNull(dynamic_cast<TColorChangeData *>(ColorChangeDataComponent));
}

static void SaveCustomColors(bool SessionColors, const UnicodeString & Colors)
{
  if (SessionColors)
  {
    CustomWinConfiguration->SessionColors = Colors;
  }
  else
  {
    CustomWinConfiguration->FontColors = Colors;
  }
}

static UnicodeString LoadCustomColors(bool SessionColors)
{
  return SessionColors ? CustomWinConfiguration->SessionColors : CustomWinConfiguration->FontColors;
}

void TColorChangeData::ColorChange(TColor Color)
{
  // Color palette returns clNone when no color is selected,
  // though it should not really happen.
  // See also CreateColorPalette
  if (Color == Vcl::Graphics::clNone)
  {
    Color = TColor(0);
  }

  if ((Color != TColor(0)) &&
      !IsStandardColor(SessionColors, Color))
  {
    UnicodeString Colors = StoreColor(Color);
    UnicodeString Temp = LoadCustomColors(SessionColors);
    while (!Temp.IsEmpty())
    {
      UnicodeString CStr = ExtractColorStr(Temp);
      if (RestoreColor(CStr) != Color)
      {
        Colors += UnicodeString(ColorSeparator) + CStr;
      }
    }
    SaveCustomColors(SessionColors, Colors);
  }

  FOnColorChange(Color);
}

static void ColorDefaultClick(void * /*Data*/, TObject * Sender)
{
  TColorChangeData::Retrieve(Sender)->ColorChange(TColor(0));
}

static void ColorPaletteChange(void * /*Data*/, TObject * Sender)
{
  TTBXColorPalette * ColorPalette = DebugNotNull(dynamic_cast<TTBXColorPalette *>(Sender));
  TColorChangeData::Retrieve(Sender)->ColorChange(GetNonZeroColor(ColorPalette->Color));
}

static UnicodeString CustomColorName(int Index)
{
  return UnicodeString(L"Color") + wchar_t(L'A' + Index);
}

static void ColorPickClick(void * /*Data*/, TObject * Sender)
{
  TColorChangeData * ColorChangeData = TColorChangeData::Retrieve(Sender);

  std::unique_ptr<TColorDialog> Dialog(new TColorDialog(Application));
  Dialog->Options = Dialog->Options << cdFullOpen << cdAnyColor;
  Dialog->Color = (ColorChangeData->Color != 0 ? ColorChangeData->Color : clSkyBlue);

  UnicodeString Temp = LoadCustomColors(ColorChangeData->SessionColors);
  int StandardColorIndex = 0;
  for (int Index = 0; Index < MaxCustomColors; Index++)
  {
    TColor CustomColor;
    if (!Temp.IsEmpty())
    {
      CustomColor = RestoreColor(ExtractColorStr(Temp));
    }
    else
    {
      if (ColorChangeData->SessionColors)
      {
        if (StandardColorIndex < StandardColorCount)
        {
          UnicodeString Name; // not used
          GetStandardSessionColorInfo(
            StandardColorIndex % ColorCols, StandardColorIndex / ColorCols,
            CustomColor, Name);
          StandardColorIndex++;
        }
        else
        {
          break;
        }
      }
      else
      {
        // no standard font colors
        break;
      }
    }
    Dialog->CustomColors->Values[CustomColorName(Index)] = StoreColor(CustomColor);
  }

  if (Dialog->Execute())
  {
    // so that we do not have to try to preserve the excess colors
    DebugAssert(UserColorCount <= MaxCustomColors);
    UnicodeString Colors;
    for (int Index = 0; Index < MaxCustomColors; Index++)
    {
      UnicodeString CStr = Dialog->CustomColors->Values[CustomColorName(Index)];
      if (!CStr.IsEmpty())
      {
        TColor CustomColor = RestoreColor(CStr);
        if ((CustomColor != static_cast<TColor>(-1)) &&
            !IsStandardColor(ColorChangeData->SessionColors, CustomColor))
        {
          AddToList(Colors, StoreColor(CustomColor), ColorSeparator);
        }
      }
    }
    SaveCustomColors(ColorChangeData->SessionColors, Colors);

    // call color change only after copying custom colors back,
    // so that it can add selected color to the user list
    ColorChangeData->ColorChange(GetNonZeroColor(Dialog->Color));
  }
}

TPopupMenu * CreateSessionColorPopupMenu(TColor Color,
  TColorChangeEvent OnColorChange)
{
  std::unique_ptr<TTBXPopupMenu> PopupMenu(new TTBXPopupMenu(Application));
  CreateSessionColorMenu(PopupMenu->Items, Color, OnColorChange);
  return PopupMenu.release();
}

static void UserCustomColorSetGetColorInfo(
  void * /*Data*/, TTBXCustomColorSet * Sender, int Col, int Row, TColor & Color, UnicodeString & /*Name*/)
{
  int Index = (Row * Sender->ColCount) + Col;
  bool SessionColors = static_cast<bool>(Sender->Tag);
  UnicodeString Temp = LoadCustomColors(SessionColors);
  while ((Index > 0) && !Temp.IsEmpty())
  {
    ExtractColorStr(Temp);
    Index--;
  }

  if (!Temp.IsEmpty())
  {
    Color = RestoreColor(ExtractColorStr(Temp));
  }
  else
  {
    // hide the trailing cells
    Color = Vcl::Graphics::clNone;
  }
}

void CreateColorPalette(TTBCustomItem * Owner, TColor Color, int Rows,
  TCSGetColorInfo OnGetColorInfo, TColorChangeEvent OnColorChange, bool SessionColors)
{
  TTBXColorPalette * ColorPalette = new TTBXColorPalette(Owner);

  if (OnGetColorInfo != nullptr)
  {
    TTBXCustomColorSet * ColorSet = new TTBXCustomColorSet(Owner);
    ColorPalette->InsertComponent(ColorSet);
    ColorPalette->ColorSet = ColorSet;

    // has to be set only after it's assigned to color palette
    ColorSet->ColCount = ColorCols;
    ColorSet->RowCount = Rows;
    ColorSet->OnGetColorInfo = OnGetColorInfo;
    ColorSet->Tag = static_cast<int>(SessionColors);
  }

  // clNone = no selection, see also ColorChange
  ColorPalette->Color = (Color != 0) ? Color : Vcl::Graphics::clNone;
  ColorPalette->OnChange = MakeMethod<TNotifyEvent>(nullptr, ColorPaletteChange);
  ColorPalette->InsertComponent(new TColorChangeData(OnColorChange, Color, SessionColors));
  Owner->Add(ColorPalette);

  Owner->Add(new TTBXSeparatorItem(Owner));
}

static void CreateColorMenu(TComponent * AOwner, TColor Color,
  TColorChangeEvent OnColorChange, bool SessionColors,
  const UnicodeString & DefaultColorCaption, const UnicodeString & DefaultColorHint,
  const UnicodeString & HelpKeyword,
  const UnicodeString & ColorPickHint)
{
  TTBCustomItem * Owner = dynamic_cast<TTBCustomItem *>(AOwner);
  if (DebugAlwaysTrue(Owner != nullptr))
  {
    Owner->Clear();

    TTBCustomItem * Item;

    Item = new TTBXItem(Owner);
    Item->Caption = DefaultColorCaption;
    Item->Hint = DefaultColorHint;
    Item->HelpKeyword = HelpKeyword;
    Item->OnClick = MakeMethod<TNotifyEvent>(nullptr, ColorDefaultClick);
    Item->Checked = (Color == TColor(0));
    Item->InsertComponent(new TColorChangeData(OnColorChange, Color, SessionColors));
    Owner->Add(Item);

    Owner->Add(new TTBXSeparatorItem(Owner));

    int CustomColorCount = 0;
    UnicodeString Temp = LoadCustomColors(SessionColors);
    while (!Temp.IsEmpty())
    {
      CustomColorCount++;
      ExtractColorStr(Temp);
    }

    if (CustomColorCount > 0)
    {
      CustomColorCount = Min(CustomColorCount, UserColorCount);
      int RowCount = ((CustomColorCount + ColorCols - 1) / ColorCols);
      DebugAssert(RowCount <= UserColorRows);

      CreateColorPalette(Owner, Color, RowCount,
        MakeMethod<TCSGetColorInfo>(nullptr, UserCustomColorSetGetColorInfo),
        OnColorChange, SessionColors);
    }

    if (SessionColors)
    {
      CreateColorPalette(Owner, Color, StandardColorRows,
        MakeMethod<TCSGetColorInfo>(nullptr, SessionColorSetGetColorInfo),
        OnColorChange, SessionColors);
    }
    else
    {
      CreateColorPalette(Owner, Color, -1, nullptr, OnColorChange, SessionColors);
    }

    Owner->Add(new TTBXSeparatorItem(Owner));

    Item = new TTBXItem(Owner);
    Item->Caption = LoadStr(COLOR_PICK_CAPTION);
    Item->Hint = ColorPickHint;
    Item->HelpKeyword = HelpKeyword;
    Item->OnClick = MakeMethod<TNotifyEvent>(nullptr, ColorPickClick);
    Item->InsertComponent(new TColorChangeData(OnColorChange, Color, SessionColors));
    Owner->Add(Item);
  }
}

void CreateSessionColorMenu(TComponent * AOwner, TColor Color,
  TColorChangeEvent OnColorChange)
{
  CreateColorMenu(
    AOwner, Color, OnColorChange, true,
    LoadStr(COLOR_TRUE_DEFAULT_CAPTION), LoadStr(COLOR_DEFAULT_HINT),
    HELP_COLOR, LoadStr(COLOR_PICK_HINT));
}

void CreateEditorBackgroundColorMenu(TComponent * AOwner, TColor Color,
  TColorChangeEvent OnColorChange)
{
  CreateColorMenu(
    AOwner, Color, OnColorChange, true,
    LoadStr(COLOR_TRUE_DEFAULT_CAPTION), LoadStr(EDITOR_BACKGROUND_COLOR_HINT),
    HELP_COLOR, LoadStr(EDITOR_BACKGROUND_COLOR_PICK_HINT));
}

TPopupMenu * CreateColorPopupMenu(TColor Color,
  TColorChangeEvent OnColorChange)
{
  std::unique_ptr<TTBXPopupMenu> PopupMenu(new TTBXPopupMenu(Application));
  CreateColorMenu(
    PopupMenu->Items, Color, OnColorChange, false,
    LoadStr(COLOR_TRUE_DEFAULT_CAPTION), UnicodeString(),
    HELP_NONE, UnicodeString());
  return PopupMenu.release();
}

void UpgradeSpeedButton(TSpeedButton * /*Button*/)
{
  // no-op yet
}

struct TThreadParam
{
  TThreadFunc ThreadFunc;
  void * Parameter;
};

static int ThreadProc(void * AParam)
{
  TThreadParam * Param = reinterpret_cast<TThreadParam *>(AParam);
  unsigned int Result = Param->ThreadFunc(Param->Parameter);
  delete Param;
  EndThread(Result);
  return Result;
}

int StartThread(void * SecurityAttributes, unsigned StackSize,
  TThreadFunc ThreadFunc, void * Parameter, unsigned CreationFlags,
  TThreadID & ThreadId)
{
  TThreadParam * Param = new TThreadParam;
  Param->ThreadFunc = ThreadFunc;
  Param->Parameter = Parameter;
  return BeginThread(SecurityAttributes, StackSize, ThreadProc, Param,
    CreationFlags, ThreadId);
}

static TShortCut FirstCtrlNumberShortCut = ShortCut(L'0', TShiftState() << ssCtrl);
static TShortCut LastCtrlNumberShortCut = ShortCut(L'9', TShiftState() << ssCtrl);
static TShortCut FirstCtrlKeyPadShortCut = ShortCut(VK_NUMPAD0, TShiftState() << ssCtrl);
static TShortCut LastCtrlKeyPadShortCut = ShortCut(VK_NUMPAD9, TShiftState() << ssCtrl);
static TShortCut FirstShiftCtrlAltLetterShortCut = ShortCut(L'A', TShiftState() << ssShift << ssCtrl << ssAlt);
static TShortCut LastShiftCtrlAltLetterShortCut = ShortCut(L'Z', TShiftState() << ssShift << ssCtrl << ssAlt);

void InitializeShortCutCombo(TComboBox * ComboBox,
  const TShortCuts & ShortCuts)
{
  ComboBox->Items->BeginUpdate();
  try
  {
    ComboBox->Items->Clear();

    ComboBox->Items->AddObject(LoadStr(SHORTCUT_NONE), reinterpret_cast<TObject* >(0));

    for (TShortCut AShortCut = FirstCtrlNumberShortCut; AShortCut <= LastCtrlNumberShortCut; AShortCut++)
    {
      if (!ShortCuts.Has(AShortCut))
      {
        ComboBox->Items->AddObject(ShortCutToText(AShortCut), reinterpret_cast<TObject* >(AShortCut));
      }
    }

    for (TShortCut AShortCut = FirstShiftCtrlAltLetterShortCut; AShortCut <= LastShiftCtrlAltLetterShortCut; AShortCut++)
    {
      if (!ShortCuts.Has(AShortCut))
      {
        ComboBox->Items->AddObject(ShortCutToText(AShortCut), reinterpret_cast<TObject* >(AShortCut));
      }
    }
  }
  __finally
  {
    ComboBox->Items->EndUpdate();
  }

  ComboBox->Style = csDropDownList;
  ComboBox->DropDownCount = Max(ComboBox->DropDownCount, 16);
}

void SetShortCutCombo(TComboBox * ComboBox, TShortCut Value)
{
  for (int Index = ComboBox->Items->Count - 1; Index >= 0; Index--)
  {
    TShortCut AShortCut = TShortCut(ComboBox->Items->Objects[Index]);
    if (AShortCut == Value)
    {
      ComboBox->ItemIndex = Index;
      break;
    }
    else if (AShortCut < Value)
    {
      DebugAssert(Value != 0);
      ComboBox->Items->InsertObject(Index + 1, ShortCutToText(Value),
        reinterpret_cast<TObject* >(Value));
      ComboBox->ItemIndex = Index + 1;
      break;
    }
    DebugAssert(Index > 0);
  }
}

TShortCut GetShortCutCombo(TComboBox * ComboBox)
{
  return TShortCut(ComboBox->Items->Objects[ComboBox->ItemIndex]);
}

TShortCut NormalizeCustomShortCut(TShortCut ShortCut)
{
  if ((FirstCtrlKeyPadShortCut <= ShortCut) && (ShortCut <= LastCtrlKeyPadShortCut))
  {
    ShortCut = FirstCtrlNumberShortCut + (ShortCut - FirstCtrlKeyPadShortCut);
  }
  return ShortCut;
}

bool IsCustomShortCut(TShortCut ShortCut)
{
  return
    ((FirstCtrlNumberShortCut <= ShortCut) && (ShortCut <= LastCtrlNumberShortCut)) ||
    ((FirstShiftCtrlAltLetterShortCut <= ShortCut) && (ShortCut <= LastShiftCtrlAltLetterShortCut));
}


class TMasterPasswordDialog : public TCustomDialog
{
public:
  TMasterPasswordDialog(TComponent * AOwner);

  void Init(bool Current);
  bool Execute(UnicodeString & CurrentPassword, UnicodeString & NewPassword);

protected:
  virtual void DoValidate();
  virtual void DoChange(bool & CanSubmit);

private:
  TPasswordEdit * CurrentEdit;
  TPasswordEdit * NewEdit;
  TPasswordEdit * ConfirmEdit;
};

// Need to have an Owner argument for SafeFormCreate
TMasterPasswordDialog::TMasterPasswordDialog(TComponent *) :
  TCustomDialog(EmptyStr)
{
}

void TMasterPasswordDialog::Init(bool Current)
{
  HelpKeyword = Current ? HELP_MASTER_PASSWORD_CURRENT : HELP_MASTER_PASSWORD_CHANGE;
  Caption = LoadStr(MASTER_PASSWORD_CAPTION);

  CurrentEdit = new TPasswordEdit(this);
  AddEdit(CurrentEdit, CreateLabel(LoadStr(MASTER_PASSWORD_CURRENT)));
  EnableControl(CurrentEdit, Current || WinConfiguration->UseMasterPassword);
  CurrentEdit->MaxLength = PasswordMaxLength();

  if (!Current)
  {
    NewEdit = new TPasswordEdit(this);
    AddEdit(NewEdit, CreateLabel(LoadStr(MASTER_PASSWORD_NEW)));
    NewEdit->MaxLength = CurrentEdit->MaxLength;

    if (!WinConfiguration->UseMasterPassword)
    {
      ActiveControl = NewEdit;
    }

    ConfirmEdit = new TPasswordEdit(this);
    AddEdit(ConfirmEdit, CreateLabel(LoadStr(MASTER_PASSWORD_CONFIRM)));
    ConfirmEdit->MaxLength = CurrentEdit->MaxLength;
  }
  else
  {
    NewEdit = nullptr;
    ConfirmEdit = nullptr;
  }
}

bool TMasterPasswordDialog::Execute(
  UnicodeString & CurrentPassword, UnicodeString & NewPassword)
{
  bool Result = TCustomDialog::Execute();
  if (Result)
  {
    if (CurrentEdit->Enabled)
    {
      CurrentPassword = CurrentEdit->Text;
    }
    if (NewEdit != nullptr)
    {
      NewPassword = NewEdit->Text;
    }
  }
  return Result;
}

void TMasterPasswordDialog::DoChange(bool & CanSubmit)
{
  CanSubmit =
    (!WinConfiguration->UseMasterPassword || (IsValidPassword(CurrentEdit->Text) >= 0)) &&
    ((NewEdit == nullptr) || (IsValidPassword(NewEdit->Text) >= 0)) &&
    ((ConfirmEdit == nullptr) || (IsValidPassword(ConfirmEdit->Text) >= 0));
  TCustomDialog::DoChange(CanSubmit);
}

void TMasterPasswordDialog::DoValidate()
{
  TCustomDialog::DoValidate();

  if (WinConfiguration->UseMasterPassword &&
      !WinConfiguration->ValidateMasterPassword(CurrentEdit->Text))
  {
    CurrentEdit->SetFocus();
    CurrentEdit->SelectAll();
    throw Exception(MainInstructions(LoadStr(MASTER_PASSWORD_INCORRECT)));
  }

  if (NewEdit != nullptr)
  {
    if (NewEdit->Text != ConfirmEdit->Text)
    {
      ConfirmEdit->SetFocus();
      ConfirmEdit->SelectAll();
      throw Exception(MainInstructions(LoadStr(MASTER_PASSWORD_DIFFERENT)));
    }

    int Valid = IsValidPassword(NewEdit->Text);
    if (Valid <= 0)
    {
      DebugAssert(Valid == 0);
      if (MessageDialog(LoadStr(MASTER_PASSWORD_SIMPLE2), qtWarning,
            qaOK | qaCancel, HELP_MASTER_PASSWORD_SIMPLE) == qaCancel)
      {
        NewEdit->SetFocus();
        NewEdit->SelectAll();
        Abort();
      }
    }
  }
}

static bool DoMasterPasswordDialog(bool Current,
  UnicodeString & NewPassword)
{
  bool Result;
  // This can be a standalone dialog when opening session from commandline
  TMasterPasswordDialog * Dialog = SafeFormCreate<TMasterPasswordDialog>();
  try
  {
    Dialog->Init(Current);
    UnicodeString CurrentPassword;
    Result = Dialog->Execute(CurrentPassword, NewPassword);
    if (Result)
    {
      if ((Current || WinConfiguration->UseMasterPassword) &&
          DebugAlwaysTrue(!CurrentPassword.IsEmpty()))
      {
        WinConfiguration->SetMasterPassword(CurrentPassword);
      }
    }
  }
  __finally
  {
    delete Dialog;
  }
  return Result;
}

bool DoMasterPasswordDialog()
{
  UnicodeString NewPassword;
  bool Result = DoMasterPasswordDialog(true, NewPassword);
  DebugAssert(NewPassword.IsEmpty());
  return Result;
}

bool DoChangeMasterPasswordDialog(UnicodeString & NewPassword)
{
  bool Result = DoMasterPasswordDialog(false, NewPassword);
  return Result;
}

void MessageWithNoHelp(const UnicodeString & Message)
{
  TMessageParams Params;
  Params.AllowHelp = false; // to avoid recursion
  if (MessageDialog(LoadStr(HELP_SEND_MESSAGE2), qtConfirmation,
        qaOK | qaCancel, HELP_NONE, &Params) == qaOK)
  {
    SearchHelp(Message);
  }
}

void CheckLogParam(TProgramParams * Params)
{
  UnicodeString LogFile;
  if (Params->FindSwitch(LOG_SWITCH, LogFile) && CheckSafe(Params))
  {
    Configuration->Usage->Inc(L"ScriptLog");
    Configuration->TemporaryLogging(LogFile);
  }
}

bool CheckXmlLogParam(TProgramParams * Params)
{
  UnicodeString LogFile;
  bool Result =
    Params->FindSwitch(L"XmlLog", LogFile) &&
    CheckSafe(Params);
  if (Result)
  {
    Configuration->Usage->Inc(L"ScriptXmlLog");
    Configuration->TemporaryActionsLogging(LogFile);

    if (Params->FindSwitch(L"XmlLogRequired"))
    {
      Configuration->LogActionsRequired = true;
    }
  }
  return Result;
}

bool CheckSafe(TProgramParams * Params)
{
  // Originally we warned when the test didn't pass,
  // but it would actually be helping hackers, so let's be silent.

  // Earlier we tested presence of any URL on command-line.
  // That was added to prevent abusing URL handler in 3.8.2 (2006).
  // Later in 4.0.4 (2007) an /Unsafe switch was added to URL handler registration.
  // So by now, we can check for the switch only and
  // do not limit user from combining URL with say
  // /rawconfig

  return !Params->FindSwitch(UNSAFE_SWITCH);
}

#endif // #if 0
