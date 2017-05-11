#pragma once

#include <Windows.h>
#include <string>
#include <tuple>

class editEnv {
public:
    editEnv::~editEnv();
    void open();
    std::tuple <std::wstring, DWORD> get(LPCWSTR valueName) const;
    void set(LPCWSTR valueName, const std::wstring &value, const DWORD reg_type);
    void close();

private:
    HKEY subKey;
    bool opened = false;
    bool envChanged = false;
};
