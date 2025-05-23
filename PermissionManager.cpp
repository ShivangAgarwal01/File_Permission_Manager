#include <windows.h>
#include <string>
#include <fstream>
#include <ctime>
#include <cstring>
#include <sstream>
#include <iostream>

using namespace std;

// GUI input fields
HWND hFilePath, hUsername, hPermission, hOutput;

// Logging function
void logAction(const string &action) {
    ofstream logFile("permissions_log.txt", ios::app);
    if (logFile) {
        time_t now = time(0);
        char *dt = ctime(&now);
        dt[strlen(dt) - 1] = '\0'; // remove newline
        logFile << "[" << dt << "] " << action << endl;
    }
}

// Run command and capture output
std::string runCommandAndCaptureOutput(const std::string& command) {
    HANDLE hRead, hWrite;
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) return "Error creating pipe";
    if (!SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0)) return "Error setting handle";

    PROCESS_INFORMATION pi;
    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;
    si.wShowWindow = SW_HIDE;

    std::string cmd = "cmd.exe /c " + command;
    char* cmdline = &cmd[0];

    if (!CreateProcessA(NULL, cmdline, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        return "Failed to start process";
    }

    CloseHandle(hWrite);

    char buffer[128];
    DWORD bytesRead;
    std::string output;

    while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        output += buffer;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hRead);

    return output;
}

// Run a command silently without showing the CMD window
bool runCommandSilently(const string& command) {
    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    string fullCmd = "cmd.exe /c " + command;
    char* cmd = &fullCmd[0];

    BOOL success = CreateProcessA(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

    if (!success) return false;

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    char file[256], user[256], perm[16];

    switch (msg) {
        case WM_CREATE:
            CreateWindow("STATIC", "File Path:", WS_VISIBLE | WS_CHILD, 20, 20, 80, 20, hwnd, NULL, NULL, NULL);
            hFilePath = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 100, 20, 400, 20, hwnd, NULL, NULL, NULL);

            CreateWindow("STATIC", "Username:", WS_VISIBLE | WS_CHILD, 20, 50, 80, 20, hwnd, NULL, NULL, NULL);
            hUsername = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 100, 50, 400, 20, hwnd, NULL, NULL, NULL);

            CreateWindow("STATIC", "Permission (F/M/RX/R/W):", WS_VISIBLE | WS_CHILD, 20, 80, 200, 20, hwnd, NULL, NULL, NULL);
            hPermission = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 220, 80, 280, 20, hwnd, NULL, NULL, NULL);

            CreateWindow("BUTTON", "View", WS_VISIBLE | WS_CHILD, 20, 120, 80, 30, hwnd, (HMENU)1, NULL, NULL);
            CreateWindow("BUTTON", "Grant", WS_VISIBLE | WS_CHILD, 110, 120, 80, 30, hwnd, (HMENU)2, NULL, NULL);
            CreateWindow("BUTTON", "Revoke", WS_VISIBLE | WS_CHILD, 200, 120, 80, 30, hwnd, (HMENU)3, NULL, NULL);
            CreateWindow("BUTTON", "ReadOnly", WS_VISIBLE | WS_CHILD, 290, 120, 80, 30, hwnd, (HMENU)4, NULL, NULL);
            CreateWindow("BUTTON", "Revert", WS_VISIBLE | WS_CHILD, 380, 120, 80, 30, hwnd, (HMENU)6, NULL, NULL);
            CreateWindow("BUTTON", "Exit", WS_VISIBLE | WS_CHILD, 470, 120, 80, 30, hwnd, (HMENU)5, NULL, NULL);

            hOutput = CreateWindow("STATIC", "", WS_VISIBLE | WS_CHILD | SS_LEFT, 20, 170, 530, 200, hwnd, NULL, NULL, NULL);
            break;

        case WM_COMMAND:
            GetWindowTextA(hFilePath, file, 256);
            GetWindowTextA(hUsername, user, 256);
            GetWindowTextA(hPermission, perm, 16);

            switch (wParam) {
                case 1: { // View
                    std::string result = runCommandAndCaptureOutput("icacls \"" + string(file) + "\"");
                    logAction("Viewed permissions for " + string(file));
                    SetWindowTextA(hOutput, result.c_str());
                    break;
                }
                case 2: // Grant
                    runCommandSilently("icacls \"" + string(file) + "\" /grant " + string(user) + ":" + string(perm));
                    logAction("Granted " + string(perm) + " to " + string(user) + " for " + string(file));
                    SetWindowTextA(hOutput, ("Granted " + string(perm) + " to " + string(user)).c_str());
                    break;
                case 3: // Revoke
                    runCommandSilently("icacls \"" + string(file) + "\" /inheritance:r");
                    runCommandSilently("icacls \"" + string(file) + "\" /remove " + string(user));
                    logAction("Revoked permissions from " + string(user) + " for " + string(file));
                    SetWindowTextA(hOutput, ("Revoked permissions from " + string(user)).c_str());
                    break;
                case 4: // Enforce Read-only
                    runCommandSilently("icacls \"" + string(file) + "\" /inheritance:r");
                    runCommandSilently("icacls \"" + string(file) + "\" /grant Everyone:R");
                    runCommandSilently("icacls \"" + string(file) + "\" /remove %username%");
                    runCommandSilently("icacls \"" + string(file) + "\" /remove SYSTEM");
                    runCommandSilently("icacls \"" + string(file) + "\" /remove Administrators");
                    logAction("Enforced strict Read-Only on " + string(file));
                    SetWindowTextA(hOutput, ("Strict Read-only enforced on " + string(file)).c_str());
                    break;
                case 6: // Revert full access
                    runCommandSilently("icacls \"" + string(file) + "\" /grant %username%:F");
                    logAction("Reverted to Full Control for owner on " + string(file));
                    SetWindowTextA(hOutput, ("Permissions reverted for " + string(file)).c_str());
                    break;
                case 5:
                    PostQuitMessage(0);
                    break;
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR args, int nCmdShow) {
    WNDCLASS wc = {};
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.hInstance = hInst;
    wc.lpszClassName = "PermissionManager";
    wc.lpfnWndProc = WindowProc;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    CreateWindow("PermissionManager", "File Permissions Manager", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                 100, 100, 600, 450, NULL, NULL, NULL, NULL);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
