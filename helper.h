#ifndef HELPER_H
#define HELPER_H

#include <windows.h>
#include <QString>

// Helper function to run command elevated
bool runElevatedCopy(const QString &srcPath, const QString &destPath)
{
    QString params = QString("/C copy /Y \"%1\" \"%2\"").arg(srcPath, destPath);

    // Convert to wide strings
    wchar_t verb[] = L"runas"; // causes UAC prompt
    wchar_t file[] = L"cmd.exe";
    std::wstring paramsW = params.toStdWString();

    SHELLEXECUTEINFOW sei = { sizeof(sei) };
    sei.lpVerb = verb;
    sei.lpFile = file;
    sei.lpParameters = paramsW.c_str();
    sei.nShow = SW_HIDE;
    sei.fMask = SEE_MASK_NOASYNC | SEE_MASK_NOCLOSEPROCESS;

    if (!ShellExecuteExW(&sei))
        return false;

    WaitForSingleObject(sei.hProcess, INFINITE);
    CloseHandle(sei.hProcess);

    return true;
}

#endif // HELPER_H
