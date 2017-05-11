#include <Windows.h>
#include <exception>
#include <Msiquery.h>
#include <string>
#include "msiHelpers.h"

// 3 minutes - chosen arbitrarily
#define COMMAND_TIMEOUT (3 * 60 * 1000)

extern "C" __declspec(dllexport) UINT __stdcall Commit(MSIHANDLE hInstall) {
    std::wstring targetDir = GetCustomActionData(hInstall);
    if (targetDir.empty())
        return ERROR_INSTALL_FAILURE;
    
    std::wstring commandBegin(L"powershell -ExecutionPolicy \"Bypass\" \"& '");
    std::wstring commandMiddle(L"\\install.ps1' '");
    std::wstring commandEnd(L"'\"");

    return RunCommand(commandBegin + targetDir + commandMiddle + targetDir + commandEnd, COMMAND_TIMEOUT);
}

extern "C" __declspec(dllexport) UINT __stdcall Uninstall(MSIHANDLE hInstall) {
    std::wstring targetDir = GetCustomActionData(hInstall);
    if (targetDir.empty())
        return ERROR_INSTALL_FAILURE;

    std::wstring commandBegin(L"powershell -ExecutionPolicy \"Bypass\" \"& '");
    std::wstring commandMiddle(L"\\uninstall.ps1' '");
    std::wstring commandEnd(L"'\"");

    return RunCommand(commandBegin + targetDir + commandMiddle + targetDir + commandEnd, COMMAND_TIMEOUT);
}
