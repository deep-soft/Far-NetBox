
#include <vcl.h>
// #pragma hdrstop

#include <Common.h>
#include <StrUtils.hpp>

#include "TextsCore.h"
#if defined(FARPLUGIN)
#include "HelpCore.h"
#endif // FARPLUGIN
#include "rtlconsts.h"

// #pragma package(smart_init)

#if 0

static std::unique_ptr<TCriticalSection> IgnoredExceptionsCriticalSection(std::make_unique<TCriticalSection>());
using TIgnoredExceptions = nb::set_t<UnicodeString>;
static TIgnoredExceptions IgnoredExceptions;

static UnicodeString NormalizeClassName(const UnicodeString & ClassName)
{
  return ReplaceStr(ClassName, L".", L"::").LowerCase();
}

void IgnoreException(const std::type_info & ExceptionType)
{
  TGuard Guard(*IgnoredExceptionsCriticalSection.get()); nb::used(Guard);
  // We should better use type_index as a key, instead of a class name,
  // but type_index is not available in 32-bit version of STL in XE6.
  IgnoredExceptions.insert(NormalizeClassName(UnicodeString(AnsiString(ExceptionType.name()))));
}

#endif // #if 0

static bool WellKnownException(
  const Exception * E, UnicodeString * AMessage, const wchar_t ** ACounterName, Exception ** AClone, bool Rethrow)
{
  UnicodeString Message;
  const wchar_t *CounterName = nullptr;
  std::unique_ptr<Exception> Clone;


  bool Result = true;
  const bool IgnoreException = false;

#if 0
  if (!IgnoredExceptions.empty())
  {
    TGuard Guard(*IgnoredExceptionsCriticalSection.get()); nb::used(Guard);
    UnicodeString ClassName = ""; // NormalizeClassName(E->QualifiedClassName());
    IgnoreException = (IgnoredExceptions.find(ClassName) != IgnoredExceptions.end());
  }
#endif // #if 0

  if (IgnoreException)
  {
    Result = false;
  }
  // EAccessViolation is EExternal
  else if (isa<EAccessViolation>(E))
  {
    if (Rethrow)
    {
      throw EAccessViolation(E->Message);
    }
    UnicodeString S = E->Message;
    UnicodeString Dummy;
    if (!ExtractMainInstructions(S, Dummy))
    {
      S = UnicodeString();
    }
    Message = MainInstructions(LoadStr(ACCESS_VIOLATION_ERROR3)) + S;
    CounterName = L"AccessViolations";
    Clone = std::make_unique<EAccessViolation>(E->Message);
  }
#if 0
  // EIntError and EMathError are EExternal
  // EClassNotFound is EFilerError
  else if ((dyn_cast<EListError>(E) != nullptr) ||
           (dyn_cast<EStringListError>(E) != nullptr) ||
           (dyn_cast<EIntError>(E) != nullptr) ||
           (dyn_cast<EMathError>(E) != nullptr) ||
           (dyn_cast<EVariantError>(E) != nullptr) ||
           (dyn_cast<EInvalidOperation>(E) != nullptr))
           (dyn_cast<EFilerError *>(E) != nullptr))
  {
    if (Rethrow)
    {
      throw EIntError(E->Message);
    }
    Message = MainInstructions(E->Message);
    CounterName = L"InternalExceptions";
    Clone.reset(new EIntError(E->Message));
  }
  else if (dyn_cast<EExternal>(E) != nullptr)
  {
    if (Rethrow)
    {
      throw EExternal(E->Message);
    }
    Message = MainInstructions(E->Message);
    CounterName = L"ExternalExceptions";
    Clone.reset(new EExternal(E->Message));
  }
  else if (dyn_cast<EHeapException>(E) != nullptr)
  {
    if (Rethrow)
    {
      throw EHeapException(E->Message);
    }
    Message = MainInstructions(E->Message);
    CounterName = L"HeapExceptions";
    Clone.reset(new EHeapException(E->Message));
  }
#endif // #if 0
  else
  {
    Result = false;
  }

  if (Result)
  {
    if (AMessage != nullptr)
    {
      (*AMessage) = Message;
    }
    if (ACounterName != nullptr)
    {
      (*ACounterName) = CounterName;
    }
    if (AClone != nullptr)
    {
      (*AClone) = DebugNotNull(Clone.release());
    }
  }

  return Result;
}

static bool ExceptionMessage(const Exception * E, bool /*Count*/,
  bool Formatted, UnicodeString & Message, bool & InternalError)
{
  bool Result = true;
  const wchar_t * CounterName = nullptr;
  InternalError = false; // see also IsInternalException

  // this list has to be in sync with CloneException
  if (isa<EAbort>(E))
  {
    Result = false;
  }
  else if (WellKnownException(E, &Message, &CounterName, nullptr, false))
  {
    InternalError = true;
  }
  else if (E && E->Message.IsEmpty())
  {
    Result = false;
  }
  else if (E)
  {
    Message = E->Message;
  }

  if (!Formatted)
  {
    Message = UnformatMessage(Message);
  }

  if (InternalError)
  {
    Message = FMTLOAD(REPORT_ERROR, Message);
  }
#if 0
  if (Count && (CounterName != nullptr) && (Configuration->Usage != nullptr))
  {
    Configuration->Usage->Inc(CounterName);
    UnicodeString ExceptionDebugInfo =
      E->ClassName() + L":" + GetExceptionDebugInfo();
    Configuration->Usage->Set(LastInternalExceptionCounter, ExceptionDebugInfo);
  }
#endif // #if 0
  return Result;
}

bool IsInternalException(const Exception * E)
{
  // see also InternalError in ExceptionMessage
  return WellKnownException(E, nullptr, nullptr, nullptr, false);
}

bool ExceptionMessage(const Exception * E, UnicodeString & Message)
{
  bool InternalError;
  return ExceptionMessage(E, true, false, Message, InternalError);
}

bool ExceptionMessageFormatted(const Exception * E, UnicodeString & Message)
{
  bool InternalError;
  return ExceptionMessage(E, true, true, Message, InternalError);
}

bool ShouldDisplayException(Exception * E)
{
  UnicodeString Message;
  return ExceptionMessageFormatted(E, Message);
}

TStrings * ExceptionToMoreMessages(Exception * E)
{
  TStrings * Result = nullptr;
  UnicodeString Message;
  if (ExceptionMessage(E, Message))
  {
    Result = new TStringList();
    Result->Add(Message);
    const ExtException * ExtE = dyn_cast<ExtException>(E);
    if ((ExtE != nullptr) && (ExtE->GetMoreMessages() != nullptr))
    {
      Result->AddStrings(ExtE->GetMoreMessages());
    }
  }
  return Result;
}

bool ExceptionFullMessage(Exception * E, UnicodeString & Message)
{
  const bool Result = ExceptionMessage(E, Message);
  if (Result)
  {
    Message += L"\n";
    const ExtException * EE = dyn_cast<ExtException>(E);
    if ((EE != nullptr) && (EE->GetMoreMessages() != nullptr))
    {
      Message += EE->GetMoreMessages()->GetText() + L"\n";
    }
  }
  return Result;
}

UnicodeString GetExceptionHelpKeyword(const Exception * E)
{
  UnicodeString HelpKeyword;
  const ExtException * ExtE = dyn_cast<ExtException>(E);
  UnicodeString Message; // not used
  bool InternalError = false;
  if (ExtE != nullptr)
  {
    HelpKeyword = ExtE->GetHelpKeyword();
  }
  else if ((E != nullptr) && ExceptionMessage(E, false, false, Message, InternalError) &&
           InternalError)
  {
#if defined(FARPLUGIN)
    HelpKeyword = HELP_INTERNAL_ERROR;
#endif // FARPLUGIN
  }
  return HelpKeyword;
}

UnicodeString MergeHelpKeyword(const UnicodeString & PrimaryHelpKeyword, const UnicodeString & SecondaryHelpKeyword)
{
  if (!PrimaryHelpKeyword.IsEmpty() &&
      !IsInternalErrorHelpKeyword(SecondaryHelpKeyword))
  {
    // we have to yet decide what we have both
    // PrimaryHelpKeyword and SecondaryHelpKeyword
    return PrimaryHelpKeyword;
  }
  return SecondaryHelpKeyword;
}

bool IsInternalErrorHelpKeyword(const UnicodeString & HelpKeyword)
{
#if defined(FARPLUGIN)
  return
    (HelpKeyword == HELP_INTERNAL_ERROR);
#else
  return false;
#endif // FARPLUGIN
}

ExtException::ExtException(const Exception * E) :
  Exception(OBJECT_CLASS_ExtException, L"")
{
  AddMoreMessages(E);
  FHelpKeyword = GetExceptionHelpKeyword(E);
}

ExtException::ExtException(TObjectClassId Kind, const Exception * E) :
  Exception(Kind, L"")
{
  AddMoreMessages(E);
  FHelpKeyword = GetExceptionHelpKeyword(E);
}

ExtException::ExtException(const Exception * E, const UnicodeString & Msg, const UnicodeString & HelpKeyword) :
  Exception(OBJECT_CLASS_ExtException, Msg)
{
  AddMoreMessages(E);
  FHelpKeyword = MergeHelpKeyword(HelpKeyword, GetExceptionHelpKeyword(E));
}

ExtException::ExtException(TObjectClassId Kind, const Exception * E, const UnicodeString & Msg, const UnicodeString & HelpKeyword) :
  Exception(Kind, Msg)
{
  AddMoreMessages(E);
  FHelpKeyword = MergeHelpKeyword(HelpKeyword, GetExceptionHelpKeyword(E));
}

ExtException::ExtException(TObjectClassId Kind, Exception * E, int32_t Ident, const UnicodeString & HelpKeyword) :
  Exception(Kind, E, Ident),
  FHelpKeyword(HelpKeyword)
{
}

ExtException::ExtException(TObjectClassId Kind, const UnicodeString & Msg, const Exception * E, const UnicodeString & HelpKeyword) :
  Exception(Kind, L"")
{
  // "copy exception"
  AddMoreMessages(E);
  // and append message to the end to more messages
  if (!Msg.IsEmpty())
  {
    if (Message.IsEmpty())
    {
      Message = Msg;
    }
    else
    {
      if (FMoreMessages == nullptr)
      {
        FMoreMessages = new TStringList();
      }
      FMoreMessages->Append(UnformatMessage(Msg));
    }
  }
  FHelpKeyword = MergeHelpKeyword(GetExceptionHelpKeyword(E), HelpKeyword);
}

ExtException::ExtException(const UnicodeString & Msg, const UnicodeString & MoreMessages,
  const UnicodeString & HelpKeyword) :
  Exception(OBJECT_CLASS_ExtException, Msg),
  FHelpKeyword(HelpKeyword)
{
  if (!MoreMessages.IsEmpty())
  {
    FMoreMessages = TextToStringList(MoreMessages);
  }
}

ExtException::ExtException(TObjectClassId Kind, const UnicodeString & Msg, const UnicodeString & MoreMessages,
  const UnicodeString & HelpKeyword) :
  Exception(Kind, Msg),
  FHelpKeyword(HelpKeyword)
{
  if (!MoreMessages.IsEmpty())
  {
    FMoreMessages = TextToStringList(MoreMessages);
  }
}

ExtException::ExtException(const UnicodeString & Msg, TStrings * MoreMessages,
  bool Own, const UnicodeString & HelpKeyword) :
  Exception(OBJECT_CLASS_ExtException, Msg),
  FHelpKeyword(HelpKeyword)
{
  if (Own)
  {
    FMoreMessages = MoreMessages;
  }
  else
  {
    FMoreMessages = new TStringList();
    FMoreMessages->Assign(MoreMessages);
  }
}

void ExtException::AddMoreMessages(const Exception * E)
{
  if (E != nullptr)
  {
    if (FMoreMessages == nullptr)
    {
      FMoreMessages = new TStringList();
    }

    const ExtException * ExtE = dyn_cast<ExtException>(E);
    if (ExtE != nullptr)
    {
      if (ExtE->GetMoreMessages() != nullptr)
      {
        FMoreMessages->Assign(ExtE->GetMoreMessages());
      }
    }

    UnicodeString Msg;
    ExceptionMessageFormatted(E, Msg);

    // new exception does not have own message, this is in fact duplication of
    // the exception data, but the exception class may be changed
    if (Message.IsEmpty())
    {
      Message = Msg;
    }
    else if (!Msg.IsEmpty())
    {
      FMoreMessages->Insert(0, UnformatMessage(Msg));
    }

    if (IsInternalException(E))
    {
      AppendExceptionStackTraceAndForget(FMoreMessages);
    }

    if (FMoreMessages->GetCount() == 0)
    {
      SAFE_DESTROY(FMoreMessages);
    }
  }
}

ExtException::~ExtException() noexcept
{
  SAFE_DESTROY(FMoreMessages);
}

ExtException * ExtException::CloneFrom(const Exception * E)
{
  return new ExtException(E, L"");
}

ExtException * ExtException::Clone() const
{
  return CloneFrom(this);
}

void ExtException::Rethrow()
{
  throw ExtException(this, L"");
}

UnicodeString SysErrorMessageForError(int32_t LastError)
{
  UnicodeString Result;
  if (LastError != 0)
  {
    //Result = FORMAT("System Error. Code: %d.\r\n%s", LastError, SysErrorMessage(LastError));
    Result = FMTLOAD(SOSError, LastError, ::SysErrorMessage(LastError), L"");
  }
  return Result;
}

UnicodeString LastSysErrorMessage()
{
  return SysErrorMessageForError(nb::ToInt32(GetLastError()));
}

EOSExtException::EOSExtException() : EOSExtException(LastSysErrorMessage())
{
}

EOSExtException::EOSExtException(const UnicodeString & Msg) :
  EOSExtException(Msg, nb::ToInt32(GetLastError()))
{
}

EOSExtException::EOSExtException(const UnicodeString & Msg, int32_t LastError) :
  EOSExtException(OBJECT_CLASS_EOSExtException, Msg, LastError)
{
}

EOSExtException::EOSExtException(TObjectClassId Kind, const UnicodeString & Msg, int32_t LastError) :
  ExtException(Kind, Msg, LastError)
{
}

EFatal::EFatal(TObjectClassId Kind, const Exception * E, const UnicodeString & Msg, const UnicodeString & HelpKeyword) :
  ExtException(Kind, Msg, E, HelpKeyword)
{
  const EFatal * F = dyn_cast<EFatal>(E);
  if (F != nullptr)
  {
    FReopenQueried = F->GetReopenQueried();
  }
}

ECRTExtException::ECRTExtException(const UnicodeString & Msg) :
  EOSExtException(OBJECT_CLASS_ECRTExtException, Msg, errno)
{
}

ExtException * EFatal::Clone() const
{
  return new EFatal(OBJECT_CLASS_EFatal, this, L"");
}

void EFatal::Rethrow()
{
  throw EFatal(this, L"");
}

ExtException * ESshTerminate::Clone() const
{
  return new ESshTerminate(this, L"", Operation, TargetLocalPath, DestLocalFileName);
}

void ESshTerminate::Rethrow()
{
  throw ESshTerminate(this, L"", Operation, TargetLocalPath, DestLocalFileName);
}

ECallbackGuardAbort::ECallbackGuardAbort() : EAbort(OBJECT_CLASS_ECallbackGuardAbort, L"callback abort")
{
}

Exception * CloneException(Exception * E)
{
  Exception * Result{nullptr};
  // this list has to be in sync with ExceptionMessage
  const ExtException * Ext = dyn_cast<ExtException>(E);
  if (Ext != nullptr)
  {
    Result = Ext->Clone();
  }
  else if (isa<ECallbackGuardAbort>(E))
  {
    Result = new ECallbackGuardAbort();
  }
  else if (isa<EAbort>(E))
  {
    Result = new EAbort(E->Message);
  }
  else if (WellKnownException(E, nullptr, nullptr, &Result, false))
  {
    // noop
  }
  else
  {
    // we do not expect this to happen
    if (DebugAlwaysFalse(IsInternalException(E)))
    {
      // to save exception stack trace
      Result = ExtException::CloneFrom(E);
    }
    else
    {
      Result = new Exception(E->Message);
    }
  }
  return Result;
}

void RethrowException(Exception * E)
{
  // this list has to be in sync with ExceptionMessage
  if (isa<ExtException>(E))
  {
    dyn_cast<ExtException>(E)->Rethrow();
  }
  else if (isa<ECallbackGuardAbort>(E))
  {
    throw ECallbackGuardAbort();
  }
  else if (isa<EAbort>(E))
  {
    throw EAbort(E->Message);
  }
  else if (WellKnownException(E, nullptr, nullptr, nullptr, true))
  {
    // noop, should never get here
  }
  else
  {
    throw ExtException(E, L"");
  }
}

UnicodeString AddContextToExceptionMessage(const Exception & E, const UnicodeString & NewContext)
{
  UnicodeString MainMessage;
  UnicodeString Context = E.Message;
  if (!ExtractMainInstructions(Context, MainMessage))
  {
    MainMessage = E.Message;
    Context = NewContext;
  }
  else
  {
    Context += L"\n" + NewContext;
  }
  UnicodeString Result = MainInstructions(MainMessage) + L"\n" + Context;
  return Result;
}
