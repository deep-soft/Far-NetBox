#include "afxdll.h"
#include <vcl.h>

#include <plugin.hpp>
#include <Sysutils.hpp>
#include <nbutils.h>

#include "resource.h"
#include "FarDialog.h"
#include "plugin_version.hpp"

static HINSTANCE HInstanceDLL;

// extern void InitExtensionModule(HINSTANCE HInst);
// extern void TermExtensionModule();
extern TCustomFarPlugin * CreateFarPlugin(HINSTANCE HInst);
extern void DestroyFarPlugin(TCustomFarPlugin *& Plugin);

class TFarPluginGuard final : public TFarPluginEnvGuard, public TGuard
{
public:
  TFarPluginGuard() :
    TGuard(FarPlugin->GetCriticalSection())
  {
  }
};

void CreatePlugin()
{
  FarPlugin = CreateFarPlugin(HInstanceDLL);
#if !defined(NO_FILEZILLA)
  InitExtensionModule(HInstanceDLL);
#endif //if !defined(NO_FILEZILLA)
}

void DestroyPlugin()
{
  DestroyFarPlugin(FarPlugin);
  TermExtensionModule();
}

[[noreturn]]
void InvalidParameterHandler(const wchar_t * expression, const wchar_t * function,
  const wchar_t * file, unsigned int line, uintptr_t pReserved)
{
  (void)expression;
  (void)function;
  (void)file;
  (void)line;
  (void)pReserved;
  RaiseException(STATUS_INVALID_PARAMETER, 0, 0, nullptr);
}

extern "C" {

void WINAPI GetGlobalInfoW(struct GlobalInfo * Info)
{
  if (!Info || (Info->StructSize < sizeof(GlobalInfo)))
    return;
  CreatePlugin();
  Info->StructSize = sizeof(*Info);
  Info->MinFarVersion = MAKEFARVERSION(FARMANAGERVERSION_MAJOR, FARMANAGERVERSION_MINOR, FARMANAGERVERSION_REVISION, FARMANAGERVERSION_BUILD, FARMANAGERVERSION_STAGE);
  Info->Version = MAKEFARVERSION(NETBOX_VERSION_MAJOR, NETBOX_VERSION_MINOR, NETBOX_VERSION_PATCH, NETBOX_VERSION_BUILD, VS_RELEASE);
  Info->Guid = NetBoxPluginGuid;
  Info->Title = L"NetBox";
  Info->Description = PLUGIN_DESCR;
  Info->Author = PLUGIN_AUTHOR;
  Info->Instance = FarPlugin;
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo * Info)
{
  if (!Info || (Info->StructSize < sizeof(PluginStartupInfo)))
    return;
  const TFarPluginGuard Guard;
  FarPlugin->SetStartupInfo(Info);
}

void WINAPI ExitFARW(const struct ExitInfo * Info)
{
  if (!Info || (Info->StructSize < sizeof(ExitInfo)))
    return;
  {
    const TFarPluginGuard Guard;
    FarPlugin->ExitFAR();
  }
  // Now Guard is released
  DestroyPlugin();
}

void WINAPI GetPluginInfoW(PluginInfo * Info)
{
  if (!Info || (Info->StructSize < sizeof(PluginInfo)))
    return;
  DebugAssert(FarPlugin);
  const TFarPluginGuard Guard;
  FarPlugin->GetPluginInfo(Info);
}

intptr_t WINAPI ProcessSynchroEventW(const struct ProcessSynchroEventInfo * Info)
{
  // DEBUG_PRINTF("Info: %p", Info->Param);
  if (!Info || (Info->StructSize < sizeof(ProcessSynchroEventInfo)))
    return 0;
  DebugAssert(FarPlugin);
  const TFarPluginGuard Guard;
  return FarPlugin->ProcessSynchroEvent(Info);
}

intptr_t WINAPI ConfigureW(const struct ConfigureInfo * Info)
{
  if (!Info || (Info->StructSize < sizeof(ConfigureInfo)))
    return FALSE;
  const TFarPluginGuard Guard;
  return FarPlugin->Configure(Info);
}

void WINAPI ClosePanelW(const struct ClosePanelInfo * Info)
{
  if (!Info || (Info->StructSize < sizeof(ClosePanelInfo)))
    return;
  const TFarPluginGuard Guard;
  FarPlugin->ClosePanel(Info->hPanel);
}

void WINAPI GetOpenPanelInfoW(struct OpenPanelInfo * Info)
{
  if (!Info || (Info->StructSize < sizeof(OpenPanelInfo)))
    return;
  const TFarPluginGuard Guard;
  FarPlugin->GetOpenPanelInfo(Info);
}

intptr_t WINAPI GetFindDataW(struct GetFindDataInfo * Info)
{
  if (!Info || (Info->StructSize < sizeof(GetFindDataInfo)))
    return FALSE;
  const TFarPluginGuard Guard;
  return FarPlugin->GetFindData(Info);
}

void WINAPI FreeFindDataW(const struct FreeFindDataInfo * Info)
{
  if (!Info || (Info->StructSize < sizeof(FreeFindDataInfo)))
    return;
  const TFarPluginGuard Guard;
  FarPlugin->FreeFindData(Info);
}

intptr_t WINAPI ProcessHostFileW(const struct ProcessHostFileInfo * Info)
{
  if (!Info || (Info->StructSize < sizeof(ProcessHostFileInfo)))
    return FALSE;
  const TFarPluginGuard Guard;
  return FarPlugin->ProcessHostFile(Info);
}

intptr_t WINAPI ProcessPanelInputW(const struct ProcessPanelInputInfo * Info)
{
  DebugAssert(Info && FarPlugin);
  const TFarPluginGuard Guard;
  return FarPlugin->ProcessPanelInput(Info);
}

intptr_t WINAPI ProcessPanelEventW(const struct ProcessPanelEventInfo * Info)
{
  if (!Info || (Info->StructSize < sizeof(ProcessPanelEventInfo)))
    return FALSE;
  const TFarPluginGuard Guard;
  return FarPlugin->ProcessPanelEvent(Info);
}

intptr_t WINAPI SetDirectoryW(const struct SetDirectoryInfo * Info)
{
  if (!Info || (Info->StructSize < sizeof(SetDirectoryInfo)))
    return FALSE;
  const TFarPluginGuard Guard;
  const intptr_t Result = FarPlugin->SetDirectory(Info);
  return Result;
}

intptr_t WINAPI MakeDirectoryW(struct MakeDirectoryInfo * Info)
{
  if (!Info || (Info->StructSize < sizeof(MakeDirectoryInfo)))
    return FALSE;
  const TFarPluginGuard Guard;
  const intptr_t Result = FarPlugin->MakeDirectory(Info);
  return Result;
}

intptr_t WINAPI DeleteFilesW(const struct DeleteFilesInfo * Info)
{
  if (!Info || (Info->StructSize < sizeof(DeleteFilesInfo)))
    return FALSE;
  const TFarPluginGuard Guard;
  return FarPlugin->DeleteFiles(Info);
}

intptr_t WINAPI GetFilesW(struct GetFilesInfo * Info)
{
  if (!Info || (Info->StructSize < sizeof(GetFilesInfo)))
    return FALSE;
  const TFarPluginGuard Guard;
  return FarPlugin->GetFiles(Info);
}

intptr_t WINAPI PutFilesW(const struct PutFilesInfo * Info)
{
  if (!Info || (Info->StructSize < sizeof(PutFilesInfo)))
    return FALSE;
  const TFarPluginGuard Guard;
  const intptr_t Result = FarPlugin->PutFiles(Info);
  return Result;
}

intptr_t WINAPI ProcessEditorEventW(const struct ProcessEditorEventInfo * Info)
{
  if (!Info || (Info->StructSize < sizeof(ProcessEditorEventInfo)))
    return FALSE;
  const TFarPluginGuard Guard;
  return FarPlugin->ProcessEditorEvent(Info);
}

intptr_t WINAPI ProcessEditorInputW(const struct ProcessEditorInputInfo * Info)
{
  if (!Info || (Info->StructSize < sizeof(ProcessEditorInputInfo)))
    return FALSE;
  const TFarPluginGuard Guard;
  return FarPlugin->ProcessEditorInput(Info);
}

HANDLE WINAPI AnalyseW(const struct AnalyseInfo * Info)
{
  if (!Info || (Info->StructSize < sizeof(AnalyseInfo)))
    return nullptr;
  const TFarPluginGuard Guard;
  if (Info->OpMode & (OPM_FIND | OPM_COMMANDS))
  {
    return nullptr;
  }
  if (!Info->FileName)
  {
    return nullptr;
  }

  const size_t fileNameLen = nb::StrLength(Info->FileName);
  if (fileNameLen < 8 || _wcsicmp(Info->FileName + fileNameLen - 7, L".netbox") != 0)
  {
    return nullptr;
  }
  if (Info->BufferSize > 4 && strncmp(static_cast<const char *>(Info->Buffer), "<?xml", 5) != 0)
  {
    return nullptr;
  }
  return reinterpret_cast<HANDLE>(1);
}

HANDLE WINAPI OpenW(const struct OpenInfo * Info)
{
  if (!Info || (Info->StructSize < sizeof(OpenInfo)))
    return nullptr;
  DebugAssert(FarPlugin);
  const TFarPluginGuard Guard;
  const HANDLE Handle = static_cast<HANDLE>(FarPlugin->OpenPlugin(Info));
  if (!Handle && Info->OpenFrom == OPEN_ANALYSE)
  {
    return PANEL_STOP;
  }
  return Handle;
}

BOOL WINAPI DllMain(HINSTANCE HInstDLL, DWORD Reason, LPVOID /*ptr*/ )
{
  if (Reason == DLL_PROCESS_ATTACH)
  {
    HInstanceDLL = HInstDLL;
    (void) _set_invalid_parameter_handler(InvalidParameterHandler);
  }
  return TRUE;
}

} // extern "C"
