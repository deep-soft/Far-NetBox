
#pragma once

#include <rdestl/vector.h>
#include "PuttyIntf.h"
#include "Configuration.h"
#include "SessionData.h"
#include "SessionInfo.h"
//---------------------------------------------------------------------------
#ifndef PuttyIntfH
__removed struct Backend;
__removed struct Conf;
#endif
//---------------------------------------------------------------------------
struct _WSANETWORKEVENTS;
typedef struct _WSANETWORKEVENTS WSANETWORKEVENTS;
typedef UINT_PTR SOCKET;
typedef rde::vector<SOCKET> TSockets;
struct TPuttyTranslation;

enum TSshImplementation
{
  sshiUnknown,
  sshiOpenSSH,
  sshiProFTPD,
  sshiBitvise,
  sshiTitan,
  sshiOpenVMS,
  sshiCerberus,
};
//---------------------------------------------------------------------------
NB_DEFINE_CLASS_ID(TSecureShell);
class TSecureShell : public TObject
{
  friend class TPoolForDataEvent;
  NB_DISABLE_COPY(TSecureShell)
public:
  static inline bool classof(const TObject *Obj) { return Obj->is(OBJECT_CLASS_TSecureShell); }
  virtual bool is(TObjectClassId Kind) const override { return (Kind == OBJECT_CLASS_TSecureShell) || TObject::is(Kind); }
private:
  SOCKET FSocket;
  HANDLE FSocketEvent;
  TSockets FPortFwdSockets;
  TSessionUI *FUI;
  TSessionData *FSessionData;
  bool FActive;
  mutable TSessionInfo FSessionInfo;
  mutable bool FSessionInfoValid;
  TDateTime FLastDataSent;
  Backend *FBackend;
  void *FBackendHandle;
  mutable const uint32_t *FMinPacketSize;
  mutable const uint32_t *FMaxPacketSize;
  TNotifyEvent FOnReceive;
  bool FFrozen;
  bool FDataWhileFrozen;
  bool FStoredPasswordTried;
  bool FStoredPasswordTriedForKI;
  bool FStoredPassphraseTried;
  mutable int FSshVersion;
  bool FOpened;
  intptr_t FWaiting;
  bool FSimple;
  bool FNoConnectionResponse;
  bool FCollectPrivateKeyUsage;
  intptr_t FWaitingForData;
  TSshImplementation FSshImplementation;

  intptr_t PendLen;
  intptr_t PendSize;
  intptr_t OutLen;
  uint8_t *OutPtr;
  uint8_t *Pending;
  TSessionLog *FLog;
  TConfiguration *FConfiguration;
  bool FAuthenticating;
  bool FAuthenticated;
  UnicodeString FStdErrorTemp;
  UnicodeString FStdError;
  UnicodeString FCWriteTemp;
  UnicodeString FAuthenticationLog;
  UnicodeString FLastTunnelError;
  UnicodeString FUserName;
  bool FUtfStrings;
  DWORD FLastSendBufferUpdate;
  intptr_t FSendBuf;

public:
  static TCipher FuncToSsh1Cipher(const void *Cipher);
  static TCipher FuncToSsh2Cipher(const void *Cipher);
  UnicodeString FuncToCompression(int SshVersion, const void *Compress) const;
  void Init();
  void SetActive(bool Value);
  void inline CheckConnection(int Message = -1);
  void WaitForData();
  void Discard();
  void FreeBackend();
  void PoolForData(WSANETWORKEVENTS &Events, uint32_t &Result);
  void CaptureOutput(TLogLineType Type,
    const UnicodeString Line);
  void ResetConnection();
  void ResetSessionInfo();
  void SocketEventSelect(SOCKET Socket, HANDLE Event, bool Startup);
  bool EnumNetworkEvents(SOCKET Socket, WSANETWORKEVENTS &Events);
  void HandleNetworkEvents(SOCKET Socket, WSANETWORKEVENTS &Events);
  bool ProcessNetworkEvents(SOCKET Socket);
  bool EventSelectLoop(uintptr_t MSec, bool ReadEventRequired,
    WSANETWORKEVENTS *Events);
  void UpdateSessionInfo() const;
  bool GetReady() const;
  void DispatchSendBuffer(intptr_t BufSize);
  void SendBuffer(uint32_t &Result);
  uint32_t TimeoutPrompt(TQueryParamsTimerEvent PoolEvent);
  bool TryFtp();
  UnicodeString ConvertInput(RawByteString Input, uintptr_t CodePage = CP_ACP) const;
  void GetRealHost(UnicodeString &Host, intptr_t &Port) const;
  UnicodeString RetrieveHostKey(const UnicodeString Host, intptr_t Port, const UnicodeString KeyType) const;

protected:
  TCaptureOutputEvent FOnCaptureOutput;

  void GotHostKey();
  int TranslatePuttyMessage(const TPuttyTranslation *Translation,
    intptr_t Count, UnicodeString &Message, UnicodeString *HelpKeyword = nullptr) const;
  int TranslateAuthenticationMessage(UnicodeString &Message, UnicodeString *HelpKeyword = nullptr);
  int TranslateErrorMessage(UnicodeString &Message, UnicodeString *HelpKeyword = nullptr);
  void AddStdError(const UnicodeString AStr);
  void AddStdErrorLine(const UnicodeString AStr);
  void LogEvent(const UnicodeString AStr);
  void FatalError(const UnicodeString Error, const UnicodeString HelpKeyword = L"");
  UnicodeString FormatKeyStr(const UnicodeString AKeyStr) const;
  static Conf *StoreToConfig(TSessionData *Data, bool Simple);

public:
  explicit TSecureShell(TSessionUI *UI, TSessionData *SessionData,
    TSessionLog *Log, TConfiguration *Configuration);
  virtual ~TSecureShell();
  void Open();
  void Close();
  void KeepAlive();
  intptr_t Receive(uint8_t *Buf, intptr_t Length);
  bool Peek(uint8_t *& Buf, intptr_t Length) const;
  UnicodeString ReceiveLine();
  void Send(const uint8_t *Buf, intptr_t Length);
  void SendSpecial(intptr_t Code);
  void Idle(uintptr_t MSec = 0);
  void SendEOF();
  void SendLine(const UnicodeString Line);
  void SendNull();

  const TSessionInfo &GetSessionInfo() const;
  void GetHostKeyFingerprint(UnicodeString &SHA256, UnicodeString &MD5) const;
  bool SshFallbackCmd() const;
  uint32_t MinPacketSize() const;
  uint32_t MaxPacketSize() const;
  void ClearStdError();
  bool GetStoredCredentialsTried() const;
  void CollectUsage();
  bool CanChangePassword() const;

  void RegisterReceiveHandler(TNotifyEvent Handler);
  void UnregisterReceiveHandler(TNotifyEvent Handler);

  // interface to PuTTY core
  void UpdateSocket(SOCKET Value, bool Startup);
  void UpdatePortFwdSocket(SOCKET Value, bool Startup);
  void PuttyFatalError(const UnicodeString AError);
  TPromptKind IdentifyPromptKind(UnicodeString &AName) const;
  bool PromptUser(bool ToServer,
    const UnicodeString AName, bool NameRequired,
    const UnicodeString AInstructions, bool InstructionsRequired,
    TStrings *Prompts, TStrings *Results);
  void FromBackend(bool IsStdErr, const uint8_t *Data, intptr_t Length);
  void CWrite(const char *Data, intptr_t Length);
  UnicodeString GetStdError() const;
  void VerifyHostKey(
    const UnicodeString AHost, intptr_t Port, const UnicodeString AKeyType, const UnicodeString AKeyStr,
    const UnicodeString AFingerprint);
  bool HaveHostKey(const UnicodeString AHost, intptr_t Port, const UnicodeString KeyType);
  void AskAlg(const UnicodeString AAlgType, const UnicodeString AlgName);
  void DisplayBanner(const UnicodeString Banner);
  void OldKeyfileWarning();
  void PuttyLogEvent(const char *AStr);
  UnicodeString ConvertFromPutty(const char *Str, intptr_t Length) const;

  __property bool Active = { read = FActive, write = SetActive };
  __property bool Ready = { read = GetReady };
  __property TCaptureOutputEvent OnCaptureOutput = { read = FOnCaptureOutput, write = FOnCaptureOutput };
  __property TDateTime LastDataSent = { read = FLastDataSent };
  __property UnicodeString LastTunnelError = { read = FLastTunnelError };
  __property UnicodeString UserName = { read = FUserName };
  __property bool Simple = { read = FSimple, write = FSimple };
  __property TSshImplementation SshImplementation = { read = FSshImplementation };
  __property bool UtfStrings = { read = FUtfStrings, write = FUtfStrings };

  bool GetActive() const { return FActive; }
  const TCaptureOutputEvent GetOnCaptureOutput() const { return FOnCaptureOutput; }
  void SetOnCaptureOutput(TCaptureOutputEvent Value) { FOnCaptureOutput = Value; }
  TDateTime GetLastDataSent() const { return FLastDataSent; }
  UnicodeString GetLastTunnelError() const { return FLastTunnelError; }
  UnicodeString ShellGetUserName() const { return FUserName; }
  bool GetSimple() const { return FSimple; }
  void SetSimple(bool Value) { FSimple = Value; }
  TSshImplementation GetSshImplementation() const { return FSshImplementation; }
  bool GetUtfStrings() const { return FUtfStrings; }
  void SetUtfStrings(bool Value) { FUtfStrings = Value; }
};
//---------------------------------------------------------------------------
