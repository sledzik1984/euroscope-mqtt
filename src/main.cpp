#include <memory>
#pragma warning(push, 0)
#include <EuroScopePlugIn.h>
#pragma warning(pop)

#include "plugin.h"

static euroscope_mqtt::Plugin* g_pPlugin = nullptr;

void __declspec(dllexport) EuroScopePlugInInit(EuroScopePlugIn::CPlugIn **ppPlugInInstance)
{
    g_pPlugin = new euroscope_mqtt::Plugin();
    *ppPlugInInstance = g_pPlugin;
}

void __declspec(dllexport) EuroScopePlugInExit(void)
{
    delete g_pPlugin;
    g_pPlugin = nullptr;
}
