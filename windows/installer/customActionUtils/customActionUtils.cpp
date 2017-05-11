#include <Windows.h>
#include <exception>
#include <Msiquery.h>
#include <string>
#include "msiHelpers.h"
#include "pathHelpers.h"

extern "C" __declspec(dllexport) UINT __stdcall Commit(MSIHANDLE hInstall) {
    std::wstring targetDir = GetCustomActionData(hInstall);
    if (targetDir.empty())
        return ERROR_INSTALL_FAILURE;

    try {
        addToPath(targetDir);
    }
    catch (std::exception&) {
        return ERROR_INSTALL_FAILURE;
    }

    return ERROR_SUCCESS;
}

extern "C" __declspec(dllexport) UINT __stdcall Uninstall(MSIHANDLE hInstall) {
    std::wstring targetDir = GetCustomActionData(hInstall);
    if (targetDir.empty())
        return ERROR_INSTALL_FAILURE;

    try {
        removeFromPath(targetDir);
    }
    catch (std::exception&) {
        return ERROR_INSTALL_FAILURE;
    }

    return ERROR_SUCCESS;
}
