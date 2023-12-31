
#pragma once

#include "FileZillaIntern.h"

class CApiLog
{
CUSTOM_MEM_ALLOCATION_IMPL
public:
  CApiLog();
  virtual ~CApiLog();

  void InitIntern(TFileZillaIntern * Intern);
  TFileZillaIntern * GetIntern();

  bool LoggingMessageType(int nMessageType) const;

  void LogMessage(int nMessageType, LPCTSTR pMsgFormat, ...) const;
  void LogMessageRaw(int nMessageType, LPCTSTR pMsg) const;
  void LogError(int Error);

  CString GetOption(int OptionID) const;
  int GetOptionVal(int OptionID) const;

protected:
  void SendLogMessage(int nMessageType, LPCTSTR pMsg) const;

  TFileZillaIntern * FIntern;
};

