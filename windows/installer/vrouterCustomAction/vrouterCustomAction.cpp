#include "Windows.h"
#include <Msiquery.h>
#include <cstdio>
#include <tchar.h>

LPWSTR GetCustomActionData(MSIHANDLE hInstall) {
    TCHAR* szValueBuf = NULL;
    DWORD cchValueBuf = 0;
    UINT uiStat = MsiGetProperty(hInstall, TEXT("CustomActionData"), TEXT(""), &cchValueBuf);
    if (uiStat == ERROR_MORE_DATA) {
        ++cchValueBuf; // add 1 for null termination
        szValueBuf = new TCHAR[cchValueBuf];
        if (szValueBuf)
            uiStat = MsiGetProperty(hInstall, TEXT("CustomActionData"), szValueBuf, &cchValueBuf);

    }
    if (uiStat != ERROR_SUCCESS) {
        if (szValueBuf != NULL)
            delete[] szValueBuf;
        return NULL;
    }
    return szValueBuf;
}

UINT RunCommand(LPWSTR command) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcess(NULL, command, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
        return ERROR_INSTALL_FAILURE;

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exit_code;
    if (!GetExitCodeProcess(pi.hProcess, &exit_code))
        return ERROR_INSTALL_FAILURE;

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    if (exit_code)
        return ERROR_INSTALL_FAILURE;

    return ERROR_SUCCESS;
}

extern "C" __declspec(dllexport) UINT __stdcall Commit(MSIHANDLE hInstall) {
    LPWSTR targetDir = GetCustomActionData(hInstall);
    if (targetDir == NULL)
        return ERROR_INSTALL_FAILURE;
    
    TCHAR command[MAX_PATH] = TEXT("powershell \"& '");
    LPCWSTR commandMiddle = TEXT("\\install_vRouter.ps1' '");
    LPCWSTR commandEnd = TEXT("'\"");
    _tcscat(command, targetDir);
    _tcscat(command, commandMiddle);
    _tcscat(command, targetDir);
    _tcscat(command, commandEnd);
    delete[] targetDir;

    return RunCommand(command);
}

extern "C" __declspec(dllexport) UINT __stdcall Uninstall(MSIHANDLE hInstall) {
    LPWSTR targetDir = GetCustomActionData(hInstall);
    if (targetDir == NULL)
        return ERROR_INSTALL_FAILURE;

    TCHAR command[MAX_PATH] = TEXT("powershell \"& '");
    LPCWSTR commandMiddle = TEXT("\\uninstall_vRouter.ps1' '");
    LPCWSTR commandEnd = TEXT("'\"");
    _tcscat(command, targetDir);
    _tcscat(command, commandMiddle);
    _tcscat(command, targetDir);
    _tcscat(command, commandEnd);
    delete[] targetDir;

    return RunCommand(command);
}
