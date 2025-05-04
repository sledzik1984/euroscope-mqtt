#include <memory>
#pragma warning(push, 0)
#include <EuroScopePlugIn.h>
#pragma warning(pop)

#include "plugin.h"

std::unique_ptr<EuroScopePlugIn::CPlugIn> Plugin;

void __declspec(dllexport) EuroScopePlugInInit(EuroScopePlugIn::CPlugIn **ppPlugInInstance)
{
    Plugin.reset(new euroscope_mqtt::euroscope_mqtt());
    *ppPlugInInstance = Plugin.get();
}

void __declspec(dllexport) EuroScopePlugInExit(void) {}
