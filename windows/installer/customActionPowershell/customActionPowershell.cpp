#include <Windows.h>
#include <exception>
#include <Msiquery.h>
#include <string>
#include "msiHelpers.h"

extern "C" __declspec(dllexport) UINT __stdcall Commit(MSIHANDLE hInstall) {
    std::wstring targetDir = GetCustomActionData(hInstall);
    if (targetDir.empty())
        return ERROR_INSTALL_FAILURE;
    return runPSScript(targetDir, PSScriptType::install);
}

extern "C" __declspec(dllexport) UINT __stdcall Uninstall(MSIHANDLE hInstall) {
    std::wstring targetDir = GetCustomActionData(hInstall);
    if (targetDir.empty())
        return ERROR_INSTALL_FAILURE;
    return runPSScript(targetDir, PSScriptType::uninstall);
}
