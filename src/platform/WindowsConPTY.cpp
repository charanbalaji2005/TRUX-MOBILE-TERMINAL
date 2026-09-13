#include "PlatformPTY.hpp"

#if defined(_WIN32) || defined(_WIN64)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#include <string>
#include <vector>

namespace meridian::platform {

class WindowsConPTY : public PlatformPTY {
public:
    WindowsConPTY() = default;

    ~WindowsConPTY() override {
        close_pty();
    }

    bool spawn_shell(const std::string& shell_path, const std::vector<std::string>& /*args*/) override {
        close_pty();

        // Create Pipes for pseudo-console input & output
        HANDLE hPipeInRead = NULL, hPipeInWrite = NULL;
        HANDLE hPipeOutRead = NULL, hPipeOutWrite = NULL;

        if (!CreatePipe(&hPipeInRead, &hPipeInWrite, NULL, 0) ||
            !CreatePipe(&hPipeOutRead, &hPipeOutWrite, NULL, 0)) {
            return false;
        }

        COORD coordSize;
        coordSize.X = size_.cols;
        coordSize.Y = size_.rows;

        HPCON hPC = NULL;
        HRESULT hr = CreatePseudoConsole(coordSize, hPipeInRead, hPipeOutWrite, 0, &hPC);
        if (FAILED(hr)) {
            CloseHandle(hPipeInRead);
            CloseHandle(hPipeInWrite);
            CloseHandle(hPipeOutRead);
            CloseHandle(hPipeOutWrite);
            return false;
        }

        // Close child-side pipe handles now that pseudo console has them
        CloseHandle(hPipeInRead);
        CloseHandle(hPipeOutWrite);

        hPipeIn_ = hPipeInWrite;
        hPipeOut_ = hPipeOutRead;
        hPC_ = hPC;

        // Prepare STARTUPINFOEX with PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE
        SIZE_T attrListSize = 0;
        InitializeProcThreadAttributeList(NULL, 1, 0, &attrListSize);
        std::vector<BYTE> attrList(attrListSize);
        LPPROC_THREAD_ATTRIBUTE_LIST pAttrList = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attrList.data());

        if (!InitializeProcThreadAttributeList(pAttrList, 1, 0, &attrListSize) ||
            !UpdateProcThreadAttribute(pAttrList, 0, PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE, hPC, sizeof(HPCON), NULL, NULL)) {
            close_pty();
            return false;
        }

        STARTUPINFOEXW siEx{};
        siEx.StartupInfo.cb = sizeof(STARTUPINFOEXW);
        siEx.lpAttributeList = pAttrList;

        PROCESS_INFORMATION pi{};
        std::wstring cmd = shell_path.empty() ? L"powershell.exe" : std::wstring(shell_path.begin(), shell_path.end());

        BOOL success = CreateProcessW(
            NULL,
            &cmd[0],
            NULL,
            NULL,
            FALSE,
            EXTENDED_STARTUPINFO_PRESENT,
            NULL,
            NULL,
            &siEx.StartupInfo,
            &pi
        );

        DeleteProcThreadAttributeList(pAttrList);

        if (!success) {
            close_pty();
            return false;
        }

        hProcess_ = pi.hProcess;
        CloseHandle(pi.hThread);
        child_pid_ = static_cast<int64_t>(pi.dwProcessId);

        return true;
    }

    ssize_t read_master(char* buffer, size_t max_bytes, int /*timeout_ms*/) override {
        if (!hPipeOut_) return -1;
        DWORD bytesAvailable = 0;
        if (!PeekNamedPipe(hPipeOut_, NULL, 0, NULL, &bytesAvailable, NULL) || bytesAvailable == 0) {
            return 0;
        }

        DWORD bytesRead = 0;
        DWORD toRead = (bytesAvailable < max_bytes) ? bytesAvailable : static_cast<DWORD>(max_bytes);
        if (!ReadFile(hPipeOut_, buffer, toRead, &bytesRead, NULL)) {
            return -1;
        }
        return static_cast<ssize_t>(bytesRead);
    }

    ssize_t write_master(const char* data, size_t length) override {
        if (!hPipeIn_) return -1;
        DWORD bytesWritten = 0;
        if (!WriteFile(hPipeIn_, data, static_cast<DWORD>(length), &bytesWritten, NULL)) {
            return -1;
        }
        return static_cast<ssize_t>(bytesWritten);
    }

    bool resize(const PTYSize& size) override {
        size_ = size;
        if (!hPC_) return false;
        COORD coord;
        coord.X = size.cols;
        coord.Y = size.rows;
        return SUCCEEDED(ResizePseudoConsole(hPC_, coord));
    }

    bool is_alive() const override {
        if (!hProcess_) return false;
        DWORD exitCode = 0;
        if (GetExitCodeProcess(hProcess_, &exitCode)) {
            return (exitCode == STILL_ACTIVE);
        }
        return false;
    }

    int64_t child_pid() const override {
        return child_pid_;
    }

    void close_pty() override {
        if (hPipeIn_) { CloseHandle(hPipeIn_); hPipeIn_ = NULL; }
        if (hPipeOut_) { CloseHandle(hPipeOut_); hPipeOut_ = NULL; }
        if (hPC_) { ClosePseudoConsole(hPC_); hPC_ = NULL; }
        if (hProcess_) {
            TerminateProcess(hProcess_, 0);
            CloseHandle(hProcess_);
            hProcess_ = NULL;
        }
        child_pid_ = -1;
    }

private:
    HPCON hPC_ = NULL;
    HANDLE hPipeIn_ = NULL;
    HANDLE hPipeOut_ = NULL;
    HANDLE hProcess_ = NULL;
    int64_t child_pid_ = -1;
    PTYSize size_{80, 24, 0, 0};
};

std::unique_ptr<PlatformPTY> PlatformPTY::create_native_pty() {
    return std::make_unique<WindowsConPTY>();
}

PlatformOS PlatformPTY::current_platform() {
    return PlatformOS::Windows;
}

std::string PlatformPTY::platform_name() {
    return "Windows";
}

} // namespace meridian::platform

#endif

