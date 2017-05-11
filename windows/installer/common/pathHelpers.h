#include "editEnv.h"

#define valueName L"Path"

size_t findInPath(const std::wstring &path, const std::wstring &str) {
    size_t pos;
    std::wstring s = L';' + str + L';';

    pos = path.find(s);
    if (pos == std::wstring::npos) {
        // we haven't found anything

        s = L';' + str;
        pos = path.rfind(s);
        if (pos == std::wstring::npos || pos + s.length() != path.length()) {
            // we haven't found anything or we found it but it's not at the end of path

            s = str + L';';
            pos = path.find(s);
            if (pos != 0) {
                // we haven't found anything or we found it but it's not at the beginning of path

                pos = std::wstring::npos;
            }
        }
    }
    return pos;
}

void addToPath(const std::wstring &str) {
    std::wstring value;
    DWORD reg_type;
    editEnv ee;
    size_t pos;

    ee.open();
    std::tie(value, reg_type) = ee.get(valueName);

    pos = findInPath(value, str);
    if (pos == std::wstring::npos) {
        value.insert(0, str + L';');
        ee.set(valueName, value, reg_type);
    }
    ee.close();
}

void removeFromPath(std::wstring str) {
    std::wstring value;
    DWORD reg_type;
    editEnv ee;
    size_t pos;

    ee.open();
    std::tie(value, reg_type) = ee.get(valueName);

    pos = findInPath(value, str);
    if (pos != std::wstring::npos) {
        value.erase(pos, str.length() + 1); // + 1 for semicolon
        ee.set(valueName, value, reg_type);
    }
    ee.close();
}
