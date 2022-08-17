#include "memory.hpp"
#include <Psapi.h>

Memory::Memory(std::wstring process_name, std::wstring module_name)
	: process_name_(process_name), module_name_(module_name)
{
	if (module_name_ == L"") module_name_ = process_name_;
	get_process_id();
	handle_ = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_id_);
	get_window_handle();
    wchar_t text[MAX_PATH];
    GetWindowText(window_handle_, text, MAX_PATH);
    window_text_ = text;
	get_module_info();
}

Memory::~Memory()
{
    CloseHandle(handle_);
}

bool Memory::IsProcessRunning()
{
    HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, process_id_);
    DWORD ret = WaitForSingleObject(process, 0);
    CloseHandle(process);
    return ret == WAIT_TIMEOUT;
}

void Memory::Suspen()
{
    typedef LONG(NTAPI* NtSuspendProcess)(IN HANDLE ProcessHandle);
    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id_);
    NtSuspendProcess pfnNtSuspendProcess = (NtSuspendProcess)GetProcAddress(GetModuleHandleA("ntdll"), "NtSuspendProcess");
    pfnNtSuspendProcess(processHandle);
    CloseHandle(processHandle);
}

void Memory::Resume()
{
    typedef LONG(NTAPI* NtResumeProcess)(IN HANDLE ProcessHandle);
    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id_);
    NtResumeProcess pfnNtResumeProcess = (NtResumeProcess)GetProcAddress(GetModuleHandleA("ntdll"), "NtResumeProcess");
    pfnNtResumeProcess(processHandle);
    CloseHandle(processHandle);
}

void Memory::get_process_id()
{
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;
    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
    {
        return;
    }
    cProcesses = cbNeeded / sizeof(DWORD);
    for (i = 0; i < cProcesses; i++)
    {
        if (aProcesses[i] != 0)
        {
            TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);
            if (NULL != hProcess)
            {
                HMODULE hMod;
                DWORD cbNeeded;
                if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
                {
                    GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
                    if (process_name_ == szProcessName)
                    {
                        process_id_ = aProcesses[i];
                    }
                }
            }
            CloseHandle(hProcess);
        }
    }
}

void Memory::get_window_handle()
{
    std::pair<DWORD, HWND> params = { process_id_, window_handle_ };
    EnumWindows([](_In_ HWND hwnd, _In_ LPARAM lParam)->BOOL
        {
            DWORD process_id;
            GetWindowThreadProcessId(hwnd, &process_id);
            if ((*(std::pair<DWORD, HWND>*)lParam).first == process_id && GetWindow(hwnd, GW_OWNER) == NULL && IsWindowVisible(hwnd) == TRUE)
                (*(std::pair<DWORD, HWND>*)lParam).second = hwnd;
            return TRUE;
        }, (LPARAM)&params);
}

void Memory::get_module_info()
{
    HMODULE hMods[1024];
    HANDLE hProcess;
    DWORD cbNeeded;
    unsigned int i;
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_id_);
    if (NULL == hProcess)
        return;
    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
    {
        for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
        {
            TCHAR szModName[MAX_PATH];
            if (GetModuleFileNameEx(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
            {
                std::wstring str = szModName;
                if (str.substr(str.find_last_of(L"\\") + 1) == module_name_)
                {
                    MODULEINFO modinfo;
                    if (GetModuleInformation(hProcess, hMods[i], &modinfo, sizeof(modinfo)))
                    {
                        module_addr_ = (DWORD_PTR)modinfo.lpBaseOfDll;
                        module_size_ = modinfo.SizeOfImage;
                    }
                }
            }
        }
    }
    CloseHandle(hProcess);
}