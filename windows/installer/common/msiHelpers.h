#include <Windows.h>
#include <Msiquery.h>
#include <string>
#include <sstream>

// 3 minutes - chosen arbitrarily
#define INSTALL_SCRIPT_TIMEOUT (3 * 60 * 1000)

enum class PSScriptType {
    install,
    uninstall,
};

std::wstring GetCustomActionData(MSIHANDLE hInstall) {
    LPWSTR szValueBuf = NULL;
    DWORD cchValueBuf = 0;
    UINT uiStat = MsiGetProperty(hInstall, L"CustomActionData", L"", &cchValueBuf);
    if (uiStat == ERROR_MORE_DATA) {
        ++cchValueBuf; // add 1 for null termination
        try {
            szValueBuf = new TCHAR[cchValueBuf];
        }
        catch (const std::bad_alloc&) {
            return L"";
        }
        uiStat = MsiGetProperty(hInstall, L"CustomActionData", szValueBuf, &cchValueBuf);
    }
    if (uiStat != ERROR_SUCCESS) {
        delete[] szValueBuf;
        return L"";
    }

    std::wstring data(szValueBuf);
    delete[] szValueBuf;
    return data;
}

UINT RunCommand(const std::wstring &command, const DWORD timeout) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    bool fail = false;
    BOOL status;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    size_t commandSize = command.size() + 1;

    LPWSTR commandWchar;
    try {
        commandWchar = new TCHAR[commandSize];
    }
    catch (const std::bad_alloc&) {
        return ERROR_INSTALL_FAILURE;
    }
    wcscpy_s(commandWchar, commandSize, command.c_str());

    // in future we can use CREATE_NO_WINDOW flag
    status = CreateProcess(NULL, commandWchar, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    delete[] commandWchar;
    if (!status)
        return ERROR_INSTALL_FAILURE;

    if (WaitForSingleObject(pi.hProcess, timeout) != WAIT_OBJECT_0) {
        TerminateProcess(pi.hProcess, 9);
        fail = true;
    }
    else {
        DWORD exit_code;
        if (!GetExitCodeProcess(pi.hProcess, &exit_code) || exit_code)
            fail = true;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    if (fail)
        return ERROR_INSTALL_FAILURE;

    return ERROR_SUCCESS;
}

UINT runPSScript(const std::wstring &dir, const PSScriptType type) {
    std::wstringstream stream;
    stream << L"powershell -ExecutionPolicy \"Bypass\" \"& '" << dir;
    switch (type) {
    case PSScriptType::install:
        stream << L"\\install.ps1' '";
        break;
    case PSScriptType::uninstall:
        stream << L"\\uninstall.ps1' '";
        break;
    }
    stream << dir << L"'\"";

    return RunCommand(stream.str(), INSTALL_SCRIPT_TIMEOUT);
}
