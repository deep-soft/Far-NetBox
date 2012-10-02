#pragma once

#include <time.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

#include <vcl.h>
#include <Sysutils.hpp>

#include "FarPlugin.h"
#include "Cryptography.h"
#include "WinSCPSecurity.h"

//------------------------------------------------------------------------------

#define TEST_CASE_TODO(exp) \
    std::cerr << "TODO: " << #exp << std::endl

//------------------------------------------------------------------------------
class TStubFarPlugin : public TCustomFarPlugin
{
public:
    explicit TStubFarPlugin() :
        TCustomFarPlugin(GetModuleHandle(0))
    {
        CryptographyInitialize();
    }
    ~TStubFarPlugin()
    {
        CryptographyFinalize();
    }
protected:
    virtual void __fastcall GetPluginInfoEx(DWORD &Flags,
        TStrings *DiskMenuStrings, TStrings *PluginMenuStrings,
        TStrings *PluginConfigStrings, TStrings *CommandPrefixes)
    {
        DEBUG_PRINTF(L"call");
    }
    virtual TCustomFarFileSystem * __fastcall OpenPluginEx(int OpenFrom, intptr_t Item)
    {
        DEBUG_PRINTF(L"call");
        return NULL;
    }
    virtual bool __fastcall ConfigureEx(const struct ConfigureInfo *Info)
    {
        DEBUG_PRINTF(L"call");
        return false;
    }
    virtual int __fastcall ProcessEditorEventEx(const struct ProcessEditorEventInfo *Info)
    {
        DEBUG_PRINTF(L"call");
        return -1;
    }
    virtual int __fastcall ProcessEditorInputEx(const INPUT_RECORD *Rec)
    {
        DEBUG_PRINTF(L"call");
        return -1;
    }
};

//------------------------------------------------------------------------------

static TCustomFarPlugin *CreateStub()
{
    return new TStubFarPlugin();
}

//------------------------------------------------------------------------------

