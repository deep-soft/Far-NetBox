#include <Queue.h>
#include <Interface.h>
#include <System.IOUtils.hpp>

#include "WinInterface.h"
//---------------------------------------------------------------------------
__removed #pragma package(smart_init)
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TMessageParams::TMessageParams(uintptr_t AParams) :
  Aliases(nullptr),
  AliasesCount(0),
  Flags(0),
  Params(AParams),
  Timer(0),
  TimerEvent(nullptr),
  TimerAnswers(0),
  TimerQueryType(qtConfirmation),
  Timeout(0),
  TimeoutAnswer(0),
  NeverAskAgainAnswer(0),
  NeverAskAgainCheckedInitially(false),
  AllowHelp(false),
  MoreMessagesSize(0)
{
}
//---------------------------------------------------------------------------
void TMessageParams::Assign(const TMessageParams *AParams)
{
  Reset();

  if (AParams != nullptr)
  {
    Aliases = AParams->Aliases;
    AliasesCount = AParams->AliasesCount;
    Flags = AParams->Flags;
    // Params = AParams->Params;
    Timer = AParams->Timer;
    TimerEvent = AParams->TimerEvent;
    TimerMessage = AParams->TimerMessage;
    TimerAnswers = AParams->TimerAnswers;
    TimerQueryType = AParams->TimerQueryType;
    Timeout = AParams->Timeout;
    TimeoutAnswer = AParams->TimeoutAnswer;

    if (FLAGSET(AParams->Params, qpNeverAskAgainCheck))
    {
      Params |= mpNeverAskAgainCheck;
    }
    if (FLAGSET(AParams->Params, qpAllowContinueOnError))
    {
      Params |= mpAllowContinueOnError;
    }
  }
}
//---------------------------------------------------------------------------
inline void TMessageParams::Reset()
{
  Params = 0;
  Aliases = nullptr;
  AliasesCount = 0;
  Timer = 0;
  TimerEvent = nullptr;
  TimerMessage = L"";
  TimerAnswers = 0;
  TimerQueryType = static_cast<TQueryType>(-1);
  Timeout = 0;
  TimeoutAnswer = 0;
  NeverAskAgainTitle = L"";
  NeverAskAgainAnswer = 0;
  NeverAskAgainCheckedInitially = false;
  AllowHelp = true;
  ImageName = L"";
  MoreMessagesUrl = L"";
  MoreMessagesSize = 0;
  CustomCaption = L"";
}
//---------------------------------------------------------------------------
static bool IsPositiveAnswer(uint32_t Answer)
{
  return (Answer == qaYes) || (Answer == qaOK) || (Answer == qaYesToAll);
}
#if 0
//---------------------------------------------------------------------------
static void NeverAskAgainCheckClick(void * /*Data*/, TObject *Sender)
{
  TFarCheckBox *CheckBox = dyn_cast<TFarCheckBox>(Sender);
  DebugAssert(CheckBox != nullptr);
  TFarDialog *Dialog = dyn_cast<TFarDialog>(CheckBox->GetOwner());
  DebugAssert(Dialog != nullptr);

  uintptr_t PositiveAnswer = 0;

  if (CheckBox->GetChecked())
  {
    if (CheckBox->GetTag() > 0)
    {
      PositiveAnswer = CheckBox->GetTag();
    }
    else
    {
      for (int ii = 0; ii < Dialog->GetControlCount(); ii++)
      {
        TFarButton *Button = dyn_cast<TFarButton>(Dialog->GetControl(ii));
        if (Button != nullptr)
        {
          if (IsPositiveAnswer(Button->GetModalResult()))
          {
            PositiveAnswer = Button->GetModalResult();
            break;
          }
        }
      }
    }

    DebugAssert(PositiveAnswer != 0);
  }

  for (int ii = 0; ii < Dialog->GetControlCount(); ii++)
  {
    TFarButton *Button = dyn_cast<TFarButton>(Dialog->GetControl(ii));
    if (Button != nullptr)
    {
      if ((Button->GetModalResult() != 0) && (Button->GetModalResult() != ToIntPtr(qaCancel)))
      {
        Button->SetEnabled(!CheckBox->GetChecked() || (Button->GetModalResult() == ToIntPtr(PositiveAnswer)));
      }

#if 0
      if (Button->DropDownMenu != nullptr)
      {
        for (int iii = 0; iii < Button->DropDownMenu->Items->Count; iii++)
        {
          TMenuItem *Item = Button->DropDownMenu->Items->Items[iii];
          Item->Enabled = Item->Default || !CheckBox->Checked;
        }
      }
#endif
    }
  }
}
#endif

#if 0
//---------------------------------------------------------------------------
static TFarCheckBox *FindNeverAskAgainCheck(TFarDialog *Dialog)
{
  return nullptr; // DebugNotNull(dyn_cast<TFarCheckBox>(Dialog->FindComponent(L"NeverAskAgainCheck")));
}
//---------------------------------------------------------------------------
TFarDialog *CreateMessageDialogEx(const UnicodeString Msg,
  TStrings *MoreMessages, TQueryType Type, uint32_t Answers, const UnicodeString AHelpKeyword,
  const TMessageParams *Params, TFarButton *&TimeoutButton)
{
  TMsgDlgType DlgType;
  switch (Type)
  {
  case qtConfirmation: DlgType = mtConfirmation; break;
  case qtInformation: DlgType = mtInformation; break;
  case qtError: DlgType = mtError; break;
  case qtWarning: DlgType = mtWarning; break;
  default: DebugFail();
  }

  uintptr_t TimeoutAnswer = (Params != nullptr) ? Params->TimeoutAnswer : 0;

  uintptr_t ActualAnswers = Answers;
  if ((Params == nullptr) || Params->AllowHelp)
  {
    Answers = Answers | qaHelp;
  }

  if (IsInternalErrorHelpKeyword(HelpKeyword))
  {
    Answers = Answers | qaReport;
  }

  if ((MoreMessages != nullptr) && (MoreMessages->GetCount() == 0))
  {
    MoreMessages = nullptr;
  }

  UnicodeString ImageName;
  UnicodeString MoreMessagesUrl;
  TSize MoreMessagesSize;
  UnicodeString CustomCaption;
  if (Params != nullptr)
  {
    ImageName = Params->ImageName;
    MoreMessagesUrl = Params->MoreMessagesUrl;
    MoreMessagesSize = Params->MoreMessagesSize;
    CustomCaption = Params->CustomCaption;
  }

  const TQueryButtonAlias *Aliases = (Params != nullptr) ? Params->Aliases : nullptr;
  uintptr_t AliasesCount = (Params != nullptr) ? Params->AliasesCount : 0;

  UnicodeString NeverAskAgainCaption;
  bool HasNeverAskAgain = (Params != nullptr) && FLAGSET(Params->Params, mpNeverAskAgainCheck);
  if (HasNeverAskAgain)
  {
    NeverAskAgainCaption =
      !Params->NeverAskAgainTitle.IsEmpty() ?
        (UnicodeString)Params->NeverAskAgainTitle :
        // qaOK | qaIgnore is used, when custom "non-answer" button is required
        LoadStr(((ActualAnswers == qaOK) || (ActualAnswers == (qaOK | qaIgnore))) ?
          MSG_CHECK_NEVER_SHOW_AGAIN : MSG_CHECK_NEVER_ASK_AGAIN);
  }

  TFarDialog *Dialog = CreateMoreMessageDialog(Msg, MoreMessages, DlgType, Answers,
      Aliases, AliasesCount, TimeoutAnswer, &TimeoutButton, ImageName, NeverAskAgainCaption,
      MoreMessagesUrl, MoreMessagesSize, CustomCaption);

  try
  {
    if (HasNeverAskAgain && DebugAlwaysTrue(Params != nullptr))
    {
      TFarCheckBox *NeverAskAgainCheck = FindNeverAskAgainCheck(Dialog);
      NeverAskAgainCheck->SetChecked(Params->NeverAskAgainCheckedInitially;
      if (Params->NeverAskAgainAnswer > 0)
      {
        NeverAskAgainCheck->Tag = Params->NeverAskAgainAnswer;
      }
      TNotifyEvent OnClick;
      ((TMethod *)&OnClick)->Code = NeverAskAgainCheckClick;
      NeverAskAgainCheck->OnClick = OnClick;
    }

    Dialog->HelpKeyword = HelpKeyword;
    if (FLAGSET(Answers, qaHelp))
    {
      Dialog->BorderIcons = Dialog->BorderIcons << biHelp;
    }
    ResetSystemSettings(Dialog);
  }
  catch (...)
  {
    delete Dialog;
    throw;
  }
  return Dialog;
}
//---------------------------------------------------------------------------
uintptr_t ExecuteMessageDialog(TForm *Dialog, uint32_t Answers, const TMessageParams *Params)
{
  FlashOnBackground();
  uint32_t Answer = Dialog->ShowModal();
  // mrCancel is returned always when X button is pressed, despite
  // no Cancel button was on the dialog. Find valid "cancel" answer.
  // mrNone is returned when Windows session is closing (log off)
  if ((Answer == mrCancel) || (Answer == mrNone))
  {
    Answer = CancelAnswer(Answers);
  }

  if ((Params != nullptr) && (Params->Params & mpNeverAskAgainCheck))
  {
    TCheckBox *NeverAskAgainCheck = FindNeverAskAgainCheck(Dialog);

    if (NeverAskAgainCheck->Checked)
    {
      bool PositiveAnswer =
        (Params->NeverAskAgainAnswer > 0) ?
        (Answer == Params->NeverAskAgainAnswer) :
        IsPositiveAnswer(Answer);
      if (PositiveAnswer)
      {
        Answer = qaNeverAskAgain;
      }
    }
  }

  return Answer;
}
//---------------------------------------------------------------------------
class TMessageTimer : public TTimer
{
public:
  TQueryParamsTimerEvent Event;
  TForm *Dialog;

  TMessageTimer(TComponent *AOwner);

protected:
  void DoTimer(TObject *Sender);
};
//---------------------------------------------------------------------------
TMessageTimer::TMessageTimer(TComponent *AOwner) : TTimer(AOwner)
{
  Event = nullptr;
  OnTimer = DoTimer;
  Dialog = nullptr;
}
//---------------------------------------------------------------------------
void TMessageTimer::DoTimer(TObject * /*Sender*/)
{
  if (Event != nullptr)
  {
    uintptr_t Result = 0;
    Event(Result);
    if (Result != 0)
    {
      Dialog->ModalResult = Result;
    }
  }
}
//---------------------------------------------------------------------------
class TMessageTimeout : public TTimer
{
public:
  TMessageTimeout(TComponent *AOwner, uintptr_t Timeout,
    TButton *Button);

  void MouseMove();
  void Cancel();

protected:
  uintptr_t FOrigTimeout;
  uintptr_t FTimeout;
  TButton *FButton;
  UnicodeString FOrigCaption;
  TPoint FOrigCursorPos;

  void DoTimer(TObject *Sender);
  void UpdateButton();
};
//---------------------------------------------------------------------------
TMessageTimeout::TMessageTimeout(TComponent *AOwner,
  uintptr_t Timeout, TButton *Button) :
  TTimer(AOwner), FOrigTimeout(Timeout), FTimeout(Timeout), FButton(Button)
{
  OnTimer = DoTimer;
  Interval = MSecsPerSec;
  FOrigCaption = FButton->Caption;
  FOrigCursorPos = Mouse->CursorPos;
  UpdateButton();
}
//---------------------------------------------------------------------------
void TMessageTimeout::MouseMove()
{
  TPoint CursorPos = Mouse->CursorPos;
  int Delta = std::max(std::abs(FOrigCursorPos.X - CursorPos.X), std::abs(FOrigCursorPos.Y - CursorPos.Y));

  int Threshold = 8;
  if (DebugAlwaysTrue(FButton != nullptr))
  {
    Threshold = ScaleByTextHeight(FButton, Threshold);
  }

  if (Delta > Threshold)
  {
    FOrigCursorPos = CursorPos;
    const uintptr_t SuspendTime = 30 * MSecsPerSec;
    FTimeout = std::max(FOrigTimeout, SuspendTime);
    UpdateButton();
  }
}
//---------------------------------------------------------------------------
void TMessageTimeout::Cancel()
{
  Enabled = false;
  UpdateButton();
}
//---------------------------------------------------------------------------
void TMessageTimeout::UpdateButton()
{
  DebugAssert(FButton != nullptr);
  FButton->Caption =
    !Enabled ? FOrigCaption : FMTLOAD(TIMEOUT_BUTTON, FOrigCaption, int(FTimeout / MSecsPerSec));
}
//---------------------------------------------------------------------------
void TMessageTimeout::DoTimer(TObject * /*Sender*/)
{
  if (FTimeout <= Interval)
  {
    DebugAssert(FButton != nullptr);
    TForm *Dialog = dynamic_cast<TForm *>(FButton->Parent);
    DebugAssert(Dialog != nullptr);

    Dialog->ModalResult = FButton->ModalResult;
  }
  else
  {
    FTimeout -= Interval;
    UpdateButton();
  }
}
//---------------------------------------------------------------------
class TPublicControl : public TControl
{
  friend void MenuPopup(TObject *Sender, const TPoint &MousePos, bool &Handled);
  friend void SetTimeoutEvents(TControl *Control, TMessageTimeout *Timeout);
};
//---------------------------------------------------------------------
class TPublicWinControl : public TWinControl
{
  friend void SetTimeoutEvents(TControl *Control, TMessageTimeout *Timeout);
};
//---------------------------------------------------------------------------
static void MessageDialogMouseMove(void *Data, TObject * /*Sender*/,
  TShiftState /*Shift*/, int /*X*/, int /*Y*/)
{
  DebugAssert(Data != nullptr);
  TMessageTimeout *Timeout = static_cast<TMessageTimeout *>(Data);
  Timeout->MouseMove();
}
//---------------------------------------------------------------------------
static void MessageDialogMouseDown(void *Data, TObject * /*Sender*/,
  TMouseButton /*Button*/, TShiftState /*Shift*/, int /*X*/, int /*Y*/)
{
  DebugAssert(Data != nullptr);
  TMessageTimeout *Timeout = static_cast<TMessageTimeout *>(Data);
  Timeout->Cancel();
}
//---------------------------------------------------------------------------
static void MessageDialogKeyDownUp(void *Data, TObject * /*Sender*/,
  Word & /*Key*/, TShiftState /*Shift*/)
{
  DebugAssert(Data != nullptr);
  TMessageTimeout *Timeout = static_cast<TMessageTimeout *>(Data);
  Timeout->Cancel();
}
//---------------------------------------------------------------------------
void SetTimeoutEvents(TControl *Control, TMessageTimeout *Timeout)
{
  TPublicControl *PublicControl = reinterpret_cast<TPublicControl *>(Control);
  DebugAssert(PublicControl->OnMouseMove == nullptr);
  PublicControl->OnMouseMove = MakeMethod<TMouseMoveEvent>(Timeout, MessageDialogMouseMove);
  DebugAssert(PublicControl->OnMouseDown == nullptr);
  PublicControl->OnMouseDown = MakeMethod<TMouseEvent>(Timeout, MessageDialogMouseDown);

  TWinControl *WinControl = dynamic_cast<TWinControl *>(Control);
  if (WinControl != nullptr)
  {
    TPublicWinControl *PublicWinControl = reinterpret_cast<TPublicWinControl *>(Control);
    DebugAssert(PublicWinControl->OnKeyDown == nullptr);
    PublicWinControl->OnKeyDown = MakeMethod<TKeyEvent>(Timeout, MessageDialogKeyDownUp);
    DebugAssert(PublicWinControl->OnKeyUp == nullptr);
    PublicWinControl->OnKeyUp = MakeMethod<TKeyEvent>(Timeout, MessageDialogKeyDownUp);

    for (int Index = 0; Index < WinControl->ControlCount; Index++)
    {
      SetTimeoutEvents(WinControl->Controls[Index], Timeout);
    }
  }
}
//---------------------------------------------------------------------------
// Merge with CreateMessageDialogEx
TForm *CreateMoreMessageDialogEx(const UnicodeString Message, TStrings *MoreMessages,
  TQueryType Type, uint32_t Answers, const UnicodeString HelpKeyword, const TMessageParams *Params)
{
  std::unique_ptr<TForm> Dialog;
  UnicodeString AMessage = Message;
  TMessageTimer *Timer = nullptr;

  if ((Params != nullptr) && (Params->Timer > 0))
  {
    Timer = new TMessageTimer(Application);
    Timer->Interval = Params->Timer;
    Timer->Event = Params->TimerEvent;
    if (Params->TimerAnswers > 0)
    {
      Answers = Params->TimerAnswers;
    }
    if (Params->TimerQueryType >= 0)
    {
      Type = Params->TimerQueryType;
    }
    if (!Params->TimerMessage.IsEmpty())
    {
      AMessage = Params->TimerMessage;
    }
    Timer->Name = L"MessageTimer";
  }

  TButton *TimeoutButton = nullptr;
  Dialog.reset(
    CreateMessageDialogEx(
      AMessage, MoreMessages, Type, Answers, HelpKeyword, Params, TimeoutButton));

  if (Timer != nullptr)
  {
    Timer->Dialog = Dialog.get();
    Dialog->InsertComponent(Timer);
  }

  if (Params != nullptr)
  {
    if (Params->Timeout > 0)
    {
      TMessageTimeout *Timeout = new TMessageTimeout(Application, Params->Timeout, TimeoutButton);
      SetTimeoutEvents(Dialog.get(), Timeout);
      Timeout->Name = L"MessageTimeout";
      Dialog->InsertComponent(Timeout);
    }
  }

  return Dialog.release();
}
//---------------------------------------------------------------------------
uintptr_t MoreMessageDialog(const UnicodeString Message, TStrings *MoreMessages,
  TQueryType Type, uint32_t Answers, const UnicodeString HelpKeyword, const TMessageParams *Params)
{
  std::unique_ptr<TForm> Dialog(CreateMoreMessageDialogEx(Message, MoreMessages, Type, Answers, HelpKeyword, Params));
  uintptr_t Result = ExecuteMessageDialog(Dialog.get(), Answers, Params);
  return Result;
}
//---------------------------------------------------------------------------
uintptr_t MessageDialog(const UnicodeString Msg, TQueryType Type,
  uint32_t Answers, const UnicodeString HelpKeyword, const TMessageParams *Params)
{
  return MoreMessageDialog(Msg, nullptr, Type, Answers, HelpKeyword, Params);
}
//---------------------------------------------------------------------------
uintptr_t SimpleErrorDialog(const UnicodeString Msg, const UnicodeString MoreMessages)
{
  uintptr_t Result;
  TStrings *More = nullptr;
  try
  {
    if (!MoreMessages.IsEmpty())
    {
      More = TextToStringList(MoreMessages);
    }
    Result = MoreMessageDialog(Msg, More, qtError, qaOK, HELP_NONE);
  }
  __finally
  {
    delete More;
  }
  return Result;
}
#endif

#if 0
//---------------------------------------------------------------------------
static TStrings *StackInfoListToStrings(
  TJclStackInfoList *StackInfoList)
{
  std::unique_ptr<TStrings> StackTrace(new TStringList());
  StackInfoList->AddToStrings(StackTrace.get(), true, false, true, true);
  for (int Index = 0; Index < StackTrace->Count; Index++)
  {
    UnicodeString Frame = StackTrace->Strings[Index];
    // get rid of declarations "flags" that are included in .map
    Frame = ReplaceStr(Frame, L"", L"");
    Frame = ReplaceStr(Frame, L"__linkproc__ ", L"");
    if (DebugAlwaysTrue(!Frame.IsEmpty() && (Frame[1] == L'(')))
    {
      int Start = Frame.Pos(L"[");
      int End = Frame.Pos(L"]");
      if (DebugAlwaysTrue((Start > 1) && (End > Start) && (Frame[Start - 1] == L' ')))
      {
        // remove absolute address
        Frame.Delete(Start - 1, End - Start + 2);
      }
    }
    StackTrace->Strings[Index] = Frame;
  }
  return StackTrace.release();
}
#endif
//---------------------------------------------------------------------------
static TCriticalSection StackTraceCriticalSection;
typedef rde::map<DWORD, TStrings *> TStackTraceMap;
static TStackTraceMap StackTraceMap;
//---------------------------------------------------------------------------
bool AppendExceptionStackTraceAndForget(TStrings *&MoreMessages)
{
  bool Result = false;

  volatile TGuard Guard(StackTraceCriticalSection);

  DWORD Id = ::GetCurrentThreadId();
  TStackTraceMap::iterator Iterator = StackTraceMap.find(Id);
  if (Iterator != StackTraceMap.end())
  {
    std::unique_ptr<TStrings> OwnedMoreMessages;
    if (MoreMessages == nullptr)
    {
      OwnedMoreMessages.reset(new TStringList());
      MoreMessages = OwnedMoreMessages.release();
      Result = true;
    }
    if (!MoreMessages->GetText().IsEmpty())
    {
      MoreMessages->SetText(MoreMessages->GetText() + "\n");
    }
    MoreMessages->SetText(MoreMessages->GetText() + LoadStr(MSG_STACK_TRACE) + "\n");
    MoreMessages->AddStrings(Iterator->second);

    delete Iterator->second;
    StackTraceMap.erase(Id);

    OwnedMoreMessages.reset();
  }
  return Result;
}
//---------------------------------------------------------------------------
uintptr_t ExceptionMessageDialog(Exception * /*E*/, TQueryType /*Type*/,
  const UnicodeString /*AMessageFormat*/, uint32_t /*Answers*/, const UnicodeString /*AHelpKeyword*/,
  const TMessageParams * /*Params*/)
{
#if 0
  TStrings *MoreMessages = nullptr;
  ExtException *EE = dynamic_cast<ExtException *>(E);
  if (EE != nullptr)
  {
    MoreMessages = EE->MoreMessages;
  }

  UnicodeString Message;
  // this is always called from within ExceptionMessage check,
  // so it should never fail here
  DebugCheck(ExceptionMessageFormatted(E, Message));

  HelpKeyword = ""; // MergeHelpKeyword(HelpKeyword, GetExceptionHelpKeyword(E));

  std::unique_ptr<TStrings> OwnedMoreMessages;
  if (AppendExceptionStackTraceAndForget(MoreMessages))
  {
    OwnedMoreMessages.reset(MoreMessages);
  }

  return MoreMessageDialog(
      FORMAT(UnicodeString(MessageFormat.IsEmpty() ? UnicodeString(L"%s") : MessageFormat), Message),
      MoreMessages, Type, Answers, HelpKeyword, Params);
#endif
  ThrowNotImplemented(3018);
  return 0;
}
//---------------------------------------------------------------------------
uintptr_t FatalExceptionMessageDialog(Exception * /*E*/, TQueryType /*Type*/,
  intptr_t /*SessionReopenTimeout*/, const UnicodeString /*MessageFormat*/, uint32_t /*Answers*/,
  const UnicodeString /*HelpKeyword*/, const TMessageParams * /*Params*/)
{
#if 0
  DebugAssert(FLAGCLEAR(Answers, qaRetry));
  Answers |= qaRetry;

  TQueryButtonAlias Aliases[1];
  Aliases[0].Button = qaRetry;
  Aliases[0].Alias = LoadStr(RECONNECT_BUTTON);

  TMessageParams AParams;
  if (Params != nullptr)
  {
    AParams = *Params;
  }
  DebugAssert(AParams.Timeout == 0);
  // the condition is de facto excess
  if (SessionReopenTimeout > 0)
  {
    AParams.Timeout = SessionReopenTimeout;
    AParams.TimeoutAnswer = qaRetry;
  }
  DebugAssert(AParams.Aliases == nullptr);
  AParams.Aliases = Aliases;
  AParams.AliasesCount = LENOF(Aliases);

  return ExceptionMessageDialog(E, Type, MessageFormat, Answers, HelpKeyword, &AParams);
#endif
  ThrowNotImplemented(3017);
  return 0;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static void DoExceptNotify(TObject * /*ExceptObj*/, void * /*ExceptAddr*/,
  bool /*OSException*/, void * /*BaseOfStack*/)
{
#if 0
  if (ExceptObj != nullptr)
  {
    Exception *E = dynamic_cast<Exception *>(ExceptObj);
    if ((E != nullptr) && IsInternalException(E)) // optimization
    {
      DoExceptionStackTrace(ExceptObj, ExceptAddr, OSException, BaseOfStack);

      TJclStackInfoList *StackInfoList = JclLastExceptStackList();

      if (DebugAlwaysTrue(StackInfoList != nullptr))
      {
        std::unique_ptr<TStrings> StackTrace(StackInfoListToStrings(StackInfoList));

        DWORD ThreadID = GetCurrentThreadId();
        volatile TGuard Guard(StackTraceCriticalSection.get());

        TStackTraceMap::iterator Iterator = StackTraceMap.find(ThreadID);
        if (Iterator != StackTraceMap.end())
        {
          Iterator->second->Add(L"");
          Iterator->second->AddStrings(StackTrace.get());
        }
        else
        {
          StackTraceMap.insert(std::make_pair(ThreadID, StackTrace.release()));
        }

        // this chains so that JclLastExceptStackList() returns nullptr the next time
        // for the current thread
        delete StackInfoList;
      }
    }
  }
#endif
  ThrowNotImplemented(3016);
}
//---------------------------------------------------------------------------
void *BusyStart()
{
  void *Token = nullptr;  // ToPtr(Screen->Cursor);
//  Screen->Cursor = crHourGlass;
  return Token;
}
//---------------------------------------------------------------------------
void BusyEnd(void * /*Token*/)
{
//  Screen->Cursor = reinterpret_cast<TCursor>(Token);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static DWORD MainThread = 0;
static TDateTime LastGUIUpdate(0.0);
static double GUIUpdateIntervalFrac = ToDouble(OneSecond / 1000 * GUIUpdateInterval); // 1/5 sec
static bool NoGUI = false;
//---------------------------------------------------------------------------
void SetNoGUI()
{
  NoGUI = true;
}
//---------------------------------------------------------------------------
bool ProcessGUI(bool Force)
{
  DebugAssert(MainThread != 0);
  bool Result = false;
  if (MainThread == ::GetCurrentThreadId() && !NoGUI)
  {
    TDateTime N = Now();
    if (Force ||
      (double(N) - double(LastGUIUpdate) > GUIUpdateIntervalFrac))
    {
      LastGUIUpdate = N;
      TODO("GetGlobalFunctions()->ProcessMessages()");
//      Application->ProcessMessages();
      Result = true;
    }
  }
  return Result;
}
//---------------------------------------------------------------------------
#if 0
void CopyParamListButton(TButton *Button)
{
  if (!SupportsSplitButton())
  {
    MenuButton(Button);
  }
}
//---------------------------------------------------------------------------
const int cpiDefault = -1;
const int cpiConfigure = -2;
const int cpiCustom = -3;
const int cpiSaveSettings = -4;
//---------------------------------------------------------------------------
void CopyParamListPopup(TRect Rect, TPopupMenu *Menu,
  const TCopyParamType &Param, const UnicodeString Preset, TNotifyEvent OnClick,
  int Options, int CopyParamAttrs, bool SaveSettings)
{
  Menu->Items->Clear();

  TMenuItem *CustomizeItem = nullptr;
  TMenuItem *Item;

  if (FLAGSET(Options, cplCustomize))
  {
    Item = new TMenuItem(Menu);
    Item->Caption = LoadStr(COPY_PARAM_CUSTOM);
    Item->Tag = cpiCustom;
    Item->Default = FLAGSET(Options, cplCustomizeDefault);
    Item->OnClick = OnClick;
    Menu->Items->Add(Item);
    CustomizeItem = Item;
  }

  if (FLAGSET(Options, cplSaveSettings))
  {
    Item = new TMenuItem(Menu);
    Item->Caption = LoadStr(COPY_PARAM_SAVE_SETTINGS);
    Item->Tag = cpiSaveSettings;
    Item->Checked = SaveSettings;
    Item->OnClick = OnClick;
    Menu->Items->Add(Item);
  }

  Item = new TMenuItem(Menu);
  Item->Caption = LoadStr(COPY_PARAM_PRESET_HEADER);
  Item->Visible = false;
  Item->Enabled = false;
  Menu->Items->Add(Item);

  bool AnyChecked = false;
  Item = new TMenuItem(Menu);
  Item->Caption = LoadStr(COPY_PARAM_DEFAULT);
  Item->Tag = cpiDefault;
  Item->Checked =
    Preset.IsEmpty() && (GUIConfiguration->CopyParamPreset[L""] == Param);
  AnyChecked = AnyChecked || Item->Checked;
  Item->OnClick = OnClick;
  Menu->Items->Add(Item);

  TCopyParamType DefaultParam;
  const TCopyParamList *CopyParamList = GUIConfiguration->CopyParamList;
  for (int i = 0; i < CopyParamList->Count; i++)
  {
    UnicodeString Name = CopyParamList->Names[i];
    TCopyParamType AParam = GUIConfiguration->CopyParamPreset[Name];
    if (AParam.AnyUsableCopyParam(CopyParamAttrs) ||
      // This makes "Binary" preset visible,
      // as long as we care about transfer mode
      ((AParam == DefaultParam) &&
        FLAGCLEAR(CopyParamAttrs, cpaIncludeMaskOnly) &&
        FLAGCLEAR(CopyParamAttrs, cpaNoTransferMode)))
    {
      Item = new TMenuItem(Menu);
      Item->Caption = Name;
      Item->Tag = i;
      Item->Checked =
        (Preset == Name) && (AParam == Param);
      AnyChecked = AnyChecked || Item->Checked;
      Item->OnClick = OnClick;
      Menu->Items->Add(Item);
    }
  }

  if (CustomizeItem != nullptr)
  {
    CustomizeItem->Checked = !AnyChecked;
  }

  Item = new TMenuItem(Menu);
  Item->Caption = L"-";
  Menu->Items->Add(Item);

  Item = new TMenuItem(Menu);
  Item->Caption = LoadStr(COPY_PARAM_CONFIGURE);
  Item->Tag = cpiConfigure;
  Item->OnClick = OnClick;
  Menu->Items->Add(Item);

  MenuPopup(Menu, Rect, nullptr);
}
//---------------------------------------------------------------------------
int CopyParamListPopupClick(TObject *Sender,
  TCopyParamType &Param, UnicodeString &Preset, int CopyParamAttrs,
  bool *SaveSettings)
{
  TComponent *Item = dynamic_cast<TComponent *>(Sender);
  DebugAssert(Item != nullptr);
  DebugAssert((Item->Tag >= cpiSaveSettings) && (Item->Tag < GUIConfiguration->CopyParamList->Count));

  int Result = 0;
  if (Item->Tag == cpiConfigure)
  {
    bool MatchedPreset = (GUIConfiguration->CopyParamPreset[Preset] == Param);
    DoPreferencesDialog(pmPresets);
    Result = (MatchedPreset && GUIConfiguration->HasCopyParamPreset[Preset]);
    if (Result > 0)
    {
      // For cast, see a comment below
      Param = TCopyParamType(GUIConfiguration->CopyParamPreset[Preset]);
    }
  }
  else if (Item->Tag == cpiCustom)
  {
    Result = DoCopyParamCustomDialog(Param, CopyParamAttrs) ? 1 : 0;
  }
  else if (Item->Tag == cpiSaveSettings)
  {
    if (DebugAlwaysTrue(SaveSettings != nullptr))
    {
      *SaveSettings = !*SaveSettings;
    }
    Result = false;
  }
  else
  {
    Preset = (Item->Tag >= 0) ?
      GUIConfiguration->CopyParamList->Names[Item->Tag] : UnicodeString();
    // The cast strips away the "queue" properties of the TGUICopyParamType
    // that are not configurable in presets
    Param = TCopyParamType(GUIConfiguration->CopyParamPreset[Preset]);
    Result = true;
  }
  return Result;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class TCustomCommandPromptsDialog : public TCustomDialog
{
public:
  TCustomCommandPromptsDialog(
    const UnicodeString CustomCommandName, const UnicodeString HelpKeyword,
    const TUnicodeStringVector &Prompts, const TUnicodeStringVector &Defaults);

  bool Execute(TUnicodeStringVector &Values);

private:
  UnicodeString HistoryKey(int Index);

  std::vector<THistoryComboBox *> FEdits;
  TUnicodeStringVector FPrompts;
  UnicodeString FCustomCommandName;
};
//---------------------------------------------------------------------------
TCustomCommandPromptsDialog::TCustomCommandPromptsDialog(
  const UnicodeString CustomCommandName, const UnicodeString HelpKeyword,
  const TUnicodeStringVector &Prompts, const TUnicodeStringVector &Defaults) :
  TCustomDialog(HelpKeyword)
{

  FCustomCommandName = CustomCommandName;
  Caption = FMTLOAD(CUSTOM_COMMANDS_PARAMS_TITLE, FCustomCommandName);

  FPrompts = Prompts;
  DebugAssert(FPrompts.size() == Defaults.size());
  for (size_t Index = 0; Index < FPrompts.size(); Index++)
  {
    UnicodeString Prompt = FPrompts[Index];
    if (Prompt.IsEmpty())
    {
      Prompt = LoadStr(CUSTOM_COMMANDS_PARAM_PROMPT2);
    }
    THistoryComboBox *ComboBox = new THistoryComboBox(this);
    ComboBox->AutoComplete = false;
    AddComboBox(ComboBox, CreateLabel(Prompt));
    ComboBox->Items = CustomWinConfiguration->History[HistoryKey(Index)];
    ComboBox->Text = Defaults[Index];
    FEdits.push_back(ComboBox);
  }
}

#endif // #if 0
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TWinInteractiveCustomCommand::TWinInteractiveCustomCommand(
  TCustomCommand *ChildCustomCommand, const UnicodeString CustomCommandName, const UnicodeString HelpKeyword) :
  TInteractiveCustomCommand(ChildCustomCommand)
{
  FCustomCommandName = StripEllipsis(StripHotkey(CustomCommandName));
  FHelpKeyword = HelpKeyword;
}
//---------------------------------------------------------------------------
void TWinInteractiveCustomCommand::PatternHint(intptr_t /*AIndex*/, const UnicodeString Pattern)
{
  if (IsPromptPattern(Pattern))
  {
    UnicodeString Prompt;
    UnicodeString Default;
    bool Delimit = false;
    ParsePromptPattern(Pattern, Prompt, Default, Delimit);
#if 0
    FIndexes.insert(std::make_pair(Index, FPrompts.size()));
    FPrompts.push_back(Prompt);
    FDefaults.push_back(Default);
#endif // #if 0
  }
}
//---------------------------------------------------------------------------
void TWinInteractiveCustomCommand::Prompt(
  intptr_t /*Index*/, const UnicodeString Prompt, UnicodeString & /*Value*/) const
{
  UnicodeString APrompt = Prompt;
#if 0
  if (APrompt.IsEmpty())
  {
    APrompt = FMTLOAD(CUSTOM_COMMANDS_PARAM_PROMPT, FCustomCommandName);
  }
  std::unique_ptr<TStrings> History(CloneStrings(CustomWinConfiguration->History[L"CustomCommandParam"]));
  if (InputDialog(FMTLOAD(CUSTOM_COMMANDS_PARAM_TITLE, FCustomCommandName),
      APrompt, Value, HELP_CUSTOM_COMMAND_PARAM, History.get()))
  {
    CustomWinConfiguration->History[L"CustomCommandParam"] = History.get();
  }
  else
  {
    Abort();
  }
#endif // #if 0
}
//---------------------------------------------------------------------------
void TWinInteractiveCustomCommand::Execute(
  const UnicodeString /*ACommand*/, UnicodeString & /*AValue*/) const
{
#if 0
  // inspired by
  // http://forum.codecall.net/topic/72472-execute-a-console-program-and-capture-its-output/
  HANDLE StdOutOutput;
  HANDLE StdOutInput;
  HANDLE StdInOutput;
  HANDLE StdInInput;
  SECURITY_ATTRIBUTES SecurityAttributes;
  SecurityAttributes.nLength = sizeof(SecurityAttributes);
  SecurityAttributes.lpSecurityDescriptor = nullptr;
  SecurityAttributes.bInheritHandle = TRUE;
  try__finally
  {
    SCOPE_EXIT
    {
      if (StdOutOutput != INVALID_HANDLE_VALUE)
      {
        SAFE_CLOSE_HANDLE(StdOutOutput);
      }
      if (StdOutInput != INVALID_HANDLE_VALUE)
      {
        SAFE_CLOSE_HANDLE(StdOutInput);
      }
      if (StdInOutput != INVALID_HANDLE_VALUE)
      {
        SAFE_CLOSE_HANDLE(StdInOutput);
      }
      if (StdInInput != INVALID_HANDLE_VALUE)
      {
        SAFE_CLOSE_HANDLE(StdInInput);
      }
    };
    if (!::CreatePipe(&StdOutOutput, &StdOutInput, &SecurityAttributes, 0))
    {
      throw Exception(FMTLOAD(SHELL_PATTERN_ERROR, Command, L"out"));
    }
    else if (!::CreatePipe(&StdInOutput, &StdInInput, &SecurityAttributes, 0))
    {
      throw Exception(FMTLOAD(SHELL_PATTERN_ERROR, Command, L"in"));
    }
    else
    {
      STARTUPINFO StartupInfo;
      PROCESS_INFORMATION ProcessInformation;

      FillMemory(&StartupInfo, sizeof(StartupInfo), 0);
      StartupInfo.cb = sizeof(StartupInfo);
      StartupInfo.wShowWindow = SW_HIDE;
      StartupInfo.hStdInput = StdInOutput;
      StartupInfo.hStdOutput = StdOutInput;
      StartupInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

      if (!::CreateProcess(nullptr, Command.c_str(), &SecurityAttributes, &SecurityAttributes,
          TRUE, NORMAL_PRIORITY_CLASS, nullptr, nullptr, &StartupInfo, &ProcessInformation))
      {
        throw Exception(FMTLOAD(SHELL_PATTERN_ERROR, Command, L"process"));
      }
      else
      {
        try__finally
        {
          SCOPE_EXIT
          {
            SAFE_CLOSE_HANDLE(ProcessInformation.hProcess);
            SAFE_CLOSE_HANDLE(ProcessInformation.hThread);
          };
          // wait until the console program terminated
          bool Running = true;
          while (Running)
          {
            switch (WaitForSingleObject(ProcessInformation.hProcess, 200))
            {
            case WAIT_TIMEOUT:
              Application->ProcessMessages();
              break;

            case WAIT_OBJECT_0:
              Running = false;
              break;

            default:
              throw Exception(FMTLOAD(SHELL_PATTERN_ERROR, Command, L"wait"));
            }
          }

          char Buffer[1024];
          unsigned long Read;
          while (PeekNamedPipe(StdOutOutput, nullptr, 0, nullptr, &Read, nullptr) &&
            (Read > 0))

          {
            if (!ReadFile(StdOutOutput, &Buffer, Read, &Read, nullptr))
            {
              throw Exception(FMTLOAD(SHELL_PATTERN_ERROR, Command, L"read"));
            }
            else if (Read > 0)
            {
              Value += AnsiToString(Buffer, Read);
            }
          }

          // trim trailing cr/lf
          Value = TrimRight(Value);
        }
        __finally
        {
          CloseHandle(ProcessInformation.hProcess);
          CloseHandle(ProcessInformation.hThread);
        }
      }
    }
  }
  __finally
  ({
    if (StdOutOutput != INVALID_HANDLE_VALUE)
    {
      CloseHandle(StdOutOutput);
    }
    if (StdOutInput != INVALID_HANDLE_VALUE)
    {
      CloseHandle(StdOutInput);
    }
    if (StdInOutput != INVALID_HANDLE_VALUE)
    {
      CloseHandle(StdInOutput);
    }
    if (StdInInput != INVALID_HANDLE_VALUE)
    {
      CloseHandle(StdInInput);
    }
  })
#endif // #if 0
}

#if 0
//---------------------------------------------------------------------------
void MenuPopup(TPopupMenu *Menu, TButton *Button)
{
  MenuPopup(Menu, CalculatePopupRect(Button), Button);
}
//---------------------------------------------------------------------------
void MenuPopup(TObject *Sender, const TPoint &MousePos, bool &Handled)
{
  TControl *Control = dynamic_cast<TControl *>(Sender);
  DebugAssert(Control != nullptr);
  TPoint Point;
  if ((MousePos.x == -1) && (MousePos.y == -1))
  {
    Point = Control->ClientToScreen(TPoint(0, 0));
  }
  else
  {
    Point = Control->ClientToScreen(MousePos);
  }
  TPopupMenu *PopupMenu = (reinterpret_cast<TPublicControl *>(Control))->PopupMenu;
  DebugAssert(PopupMenu != nullptr);
  TRect Rect(Point, Point);
  MenuPopup(PopupMenu, Rect, Control);
  Handled = true;
}
//---------------------------------------------------------------------------
TComponent *GetPopupComponent(TObject *Sender)
{
  TComponent *Item = dynamic_cast<TComponent *>(Sender);
  DebugAssert(Item != nullptr);
  TPopupMenu *PopupMenu = dynamic_cast<TPopupMenu *>(Item->Owner);
  DebugAssert(PopupMenu != nullptr);
  DebugAssert(PopupMenu->PopupComponent != nullptr);
  return PopupMenu->PopupComponent;
}
//---------------------------------------------------------------------------
static void SetMenuButtonImages(TButton * Button)
{
  Button->Images = GetButtonImages(Button);
}
//---------------------------------------------------------------------------
static void MenuButtonRescale(TComponent *Sender, TObject * /*Token*/)
{
  TButton *Button = DebugNotNull(dynamic_cast<TButton *>(Sender));
  SetMenuButtonImages(Button);
}
//---------------------------------------------------------------------------
void MenuButton(TButton * Button)
{
  SetMenuButtonImages(Button);
  Button->ImageIndex = 0;
  Button->DisabledImageIndex = 1;
  Button->ImageAlignment = iaRight;
  SetRescaleFunction(Button, MenuButtonRescale);
}
//---------------------------------------------------------------------------
TRect CalculatePopupRect(TButton *Button)
{
  TPoint UpPoint = Button->ClientToScreen(TPoint(0, 0));
  TPoint DownPoint = Button->ClientToScreen(TPoint(Button->Width, Button->Height));
  TRect Rect(UpPoint, DownPoint);
  // With themes enabled, button are rendered 1 pixel smaller than their actual size
  int Offset = UseThemes() ? -1 : 0;
  Rect.Inflate(Offset, Offset);
  return Rect;
}
//---------------------------------------------------------------------------
TRect CalculatePopupRect(TControl *Control, TPoint MousePos)
{
  MousePos = Control->ClientToScreen(MousePos);
  TRect Rect(MousePos, MousePos);
  return Rect;
}
//---------------------------------------------------------------------------
void FixButtonImage(TButton *Button)
{
  // with themes enabled, button image is by default drawn too high
  if (UseThemes())
  {
    Button->ImageMargins->Top = 1;
  }
}
//---------------------------------------------------------------------------
void CenterButtonImage(TButton *Button)
{
  // with themes disabled, the text seems to be drawn over the icon,
  // so that the padding spaces hide away most of the icon
  if (UseThemes())
  {
    Button->ImageAlignment = iaCenter;
    int ImageWidth = Button->Images->Width;

    std::unique_ptr<TControlCanvas> Canvas(new TControlCanvas());
    Canvas->Control = Button;
    Canvas->Font = Button->Font;

    UnicodeString Caption = Button->Caption.Trim();
    UnicodeString Padding;
    while (Canvas->TextWidth(Padding) < ImageWidth)
    {
      Padding += L" ";
    }
    if (Button->IsRightToLeft())
    {
      Caption = Caption + Padding;
    }
    else
    {
      Caption = Padding + Caption;
    }
    Button->Caption = Caption;

    int CaptionWidth = Canvas->TextWidth(Caption);
    // The margins seem to extend the area over which the image is centered,
    // so we have to set it to a double of desired padding.
    // The original formula is - 2 * ((CaptionWidth / 2) - (ImageWidth / 2) + ScaleByTextHeight(Button, 2))
    // the one below is equivalent, but with reduced rouding.
    // Without the change, the rouding caused the space between icon and caption too
    // small on 200% zoom.
    // Note that (CaptionWidth / 2) - (ImageWidth / 2)
    // is approximatelly same as half of caption width before padding.
    Button->ImageMargins->Left = -(CaptionWidth - ImageWidth + ScaleByTextHeight(Button, 4));
  }
  else
  {
    // at least do not draw it so near to the edge
    Button->ImageMargins->Left = 1;
  }
}
//---------------------------------------------------------------------------
int AdjustLocaleFlag(const UnicodeString S, TLocaleFlagOverride LocaleFlagOverride, bool Recommended, int On, int Off)
{
  int Result = !S.IsEmpty() && StrToInt64(S);
  switch (LocaleFlagOverride)
  {
  default:
  case lfoLanguageIfRecommended:
    if (!Recommended)
    {
      Result = Off;
    }
    break;

  case lfoLanguage:
    // noop = as configured in locale
    break;

  case lfoAlways:
    Result = On;
    break;

  case lfoNever:
    Result = Off;
    break;
  }
  return Result;
}
//---------------------------------------------------------------------------
void SetGlobalMinimizeHandler(TCustomForm * /*Form*/, TNotifyEvent OnMinimize)
{
  if (GlobalOnMinimize == nullptr)
  {
    GlobalOnMinimize = OnMinimize;
  }
}
//---------------------------------------------------------------------------
void ClearGlobalMinimizeHandler(TNotifyEvent OnMinimize)
{
  if (GlobalOnMinimize == OnMinimize)
  {
    GlobalOnMinimize = nullptr;
  }
}
//---------------------------------------------------------------------------
void CallGlobalMinimizeHandler(TObject *Sender)
{
  Configuration->Usage->Inc(L"OperationMinimizations");
  if (DebugAlwaysTrue(GlobalOnMinimize != nullptr))
  {
    GlobalOnMinimize(Sender);
  }
}
//---------------------------------------------------------------------------
static void DoApplicationMinimizeRestore(bool Minimize)
{
  // WORKAROUND
  // When main window is hidden (command-line operation),
  // we do not want it to be shown by TApplication.Restore,
  // so we temporarily detach it from an application.
  // Probably not really necessary for minimizing phase,
  // but we do it for consistency anyway.
  TForm *MainForm = Application->MainForm;
  bool RestoreMainForm = false;
  if (DebugAlwaysTrue(MainForm != nullptr) &&
      !MainForm->Visible)
  {
    SetAppMainForm(nullptr);
    RestoreMainForm = true;
  }
  try
  {
    if (Minimize)
    {
      Application->Minimize();
    }
    else
    {
      Application->Restore();
    }
  }
  __finally
  {
    if (RestoreMainForm)
    {
      SetAppMainForm(MainForm);
    }
  }
}
//---------------------------------------------------------------------------
void ApplicationMinimize()
{
  DoApplicationMinimizeRestore(true);
}
//---------------------------------------------------------------------------
void ApplicationRestore()
{
  DoApplicationMinimizeRestore(false);
}
//---------------------------------------------------------------------------
bool IsApplicationMinimized()
{
  // VCL help recommends handling Application->OnMinimize/OnRestore
  // for tracking state, but OnRestore is actually not called
  // (OnMinimize is), when app is minimized from e.g. Progress window
  bool AppMinimized = IsIconic(Application->Handle);
  bool MainFormMinimized = IsIconic(Application->MainFormHandle);
  return AppMinimized || MainFormMinimized;
}
//---------------------------------------------------------------------------
bool HandleMinimizeSysCommand(TMessage &Message)
{
  TWMSysCommand &SysCommand = reinterpret_cast<TWMSysCommand &>(Message);
  uintptr_t Cmd = (SysCommand.CmdType & 0xFFF0);
  bool Result = (Cmd == SC_MINIMIZE);
  if (Result)
  {
    ApplicationMinimize();
    SysCommand.Result = 1;
  }
  return Result;
}
#endif
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class TCallstackThread : public TSignalThread
{
public:
  explicit TCallstackThread();
  void InitCallstackThread(bool LowPriority);

protected:
  virtual void ProcessEvent();

private:
  static UnicodeString DoGetName();
  static HANDLE DoCreateEvent();
};
//---------------------------------------------------------------------------
TCallstackThread::TCallstackThread() :
  TSignalThread(OBJECT_CLASS_TCallstackThread)
{
}

void TCallstackThread::InitCallstackThread(bool LowPriority)
{
  TSignalThread::InitSignalThread(LowPriority, DoCreateEvent());
}
//---------------------------------------------------------------------------
void TCallstackThread::ProcessEvent()
{
#if 0
  try
  {
    UnicodeString FileName = FORMAT(L"%s.txt", DoGetName());
    UnicodeString Path = TPath::Combine(::GetSystemTemporaryDirectory(), FileName);
    std::unique_ptr<TStrings> StackStrings;
    HANDLE MainThreadHandle = reinterpret_cast<HANDLE>(MainThreadID);
    if (SuspendThread(MainThreadHandle) < 0)
    {
      RaiseLastOSError();
    }
    try
    {
      TJclStackInfoList * StackInfoList = JclCreateThreadStackTraceFromID(true, MainThreadID);
      if (StackInfoList == nullptr)
      {
        RaiseLastOSError();
      }
      StackStrings.reset(StackInfoListToStrings(StackInfoList));
    }
    __finally
    {
      if (ResumeThread(MainThreadHandle) < 0)
      {
        RaiseLastOSError();
      }
    }
    TFile::WriteAllText(Path, StackStrings->Text);
  }
  catch (...)
  {
  }
#endif // if 0
}
//---------------------------------------------------------------------------
UnicodeString TCallstackThread::DoGetName()
{
  return FORMAT("WinSCPCallstack%d", GetCurrentProcessId());
}
//---------------------------------------------------------------------------
HANDLE TCallstackThread::DoCreateEvent()
{
  UnicodeString Name = DoGetName();
  return ::CreateEventW(nullptr, false, false, Name.c_str());
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
std::unique_ptr<TCallstackThread> CallstackThread;
//---------------------------------------------------------------------------
void WinInitialize()
{
#if 0
  if (JclHookExceptions())
  {
    JclStackTrackingOptions << stAllModules;
    JclAddExceptNotifier(DoExceptNotify, npFirstChain);
    CallstackThread.reset(new TCallstackThread());
    CallstackThread->Start();
  }
#endif // #if 0

  SetErrorMode(SEM_FAILCRITICALERRORS);
#if 0
  OnApiPath = ::ApiPath;
#endif // #if 0
  MainThread = ::GetCurrentThreadId();
  CallstackThread.reset(nullptr);
}
//---------------------------------------------------------------------------
void WinFinalize()
{
//  JclRemoveExceptNotifier(DoExceptNotify);
}
//---------------------------------------------------------------------------
bool InputDialog(const UnicodeString ACaption,
  const UnicodeString APrompt, UnicodeString &Value, const UnicodeString AHelpKeyword,
  TStrings *History, bool PathInput,
  TInputDialogInitializeEvent OnInitialize, bool Echo)
{
  bool Result = GetGlobals()->InputDialog(ACaption, APrompt, Value, AHelpKeyword,
      History, PathInput, OnInitialize, Echo);
  return Result;
}

uintptr_t MessageDialog(const UnicodeString AMsg, TQueryType Type,
  uint32_t Answers, const UnicodeString AHelpKeyword, const TMessageParams *Params)
{
  DebugUsedParam(AHelpKeyword);
  uintptr_t Result = GetGlobals()->MoreMessageDialog(AMsg, nullptr, Type, Answers, Params);
  return Result;
}

uintptr_t MessageDialog(intptr_t Ident, TQueryType Type,
  uint32_t Answers, const UnicodeString AHelpKeyword, const TMessageParams *Params)
{
  DebugUsedParam(AHelpKeyword);
  UnicodeString Msg = LoadStr(Ident);
  uintptr_t Result = GetGlobals()->MoreMessageDialog(Msg, nullptr, Type, Answers, Params);
  return Result;
}

uintptr_t SimpleErrorDialog(const UnicodeString AMsg, const UnicodeString /*AMoreMessages*/)
{
  uint32_t Answers = qaOK;
  uintptr_t Result = GetGlobals()->MoreMessageDialog(AMsg, nullptr, qtError, Answers, nullptr);
  return Result;
}

uintptr_t MoreMessageDialog(const UnicodeString AMessage,
  TStrings *MoreMessages, TQueryType Type, uint32_t Answers,
  const UnicodeString AHelpKeyword, const TMessageParams *Params)
{
  DebugUsedParam(AHelpKeyword);
  uintptr_t Result = GetGlobals()->MoreMessageDialog(AMessage, MoreMessages, Type, Answers, Params);
  return Result;
}

