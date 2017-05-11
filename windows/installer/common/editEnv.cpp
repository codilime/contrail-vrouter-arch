#include "editEnv.h"

void editEnv::open() {
    LPCWSTR keyName = L"System\\CurrentControlSet\\Control\\Session Manager\\Environment";

    LONG status;

    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyName, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &subKey);
    if (status != ERROR_SUCCESS)
        throw std::runtime_error("RegOpenKeyEx error");

    opened = true;
}

std::tuple <std::wstring, DWORD> editEnv::get(LPCWSTR valueName) const {
    const DWORD valueFlags = RRF_NOEXPAND | RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ;
    
    LONG status;
    DWORD size;
    LPBYTE data;
    std::tuple <std::wstring, DWORD> ret;

    status = RegGetValue(subKey, NULL, valueName, valueFlags, NULL, NULL, &size);
    if (status != ERROR_SUCCESS)
        throw std::runtime_error("first RegGetValue error");

    try {
        data = new BYTE[size];
    }
    catch (std::bad_alloc&) {
        throw;
    }

    status = RegGetValue(subKey, NULL, valueName, valueFlags, &std::get<1>(ret), data, &size);
    if (status != ERROR_SUCCESS) {
        delete[] data;
        throw std::runtime_error("second RegGetValue error");
    }

    std::get<0>(ret) = reinterpret_cast<LPWSTR>(data);
    delete[] data;

    return ret;
}

void editEnv::set(LPCWSTR valueName, const std::wstring &value, const DWORD reg_type) {
    LONG status;
    const size_t size = sizeof(std::wstring::value_type) * (value.length() + 1);

    status = RegSetValueEx(subKey, valueName, NULL, reg_type, reinterpret_cast<const BYTE *>(value.c_str()), static_cast<DWORD>(size));
    if (status != ERROR_SUCCESS)
        throw std::runtime_error("RegSetValueEx error");

    envChanged = true;
}

void editEnv::close() {
    BOOL status;

    opened = false;
    if (RegCloseKey(subKey) != ERROR_SUCCESS)
        throw std::runtime_error("RegCloseKey error");

    if (envChanged) {
        envChanged = false;
        status = SendNotifyMessage(HWND_BROADCAST, WM_SETTINGCHANGE, NULL, reinterpret_cast<LPARAM>(L"Environment"));
        if (status == 0)
            throw std::runtime_error("SendNotifyMessage error");
    }
}

editEnv::~editEnv() {
    if (opened)
        close();
}
