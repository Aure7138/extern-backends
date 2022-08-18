#pragma once
#include <Windows.h>
#include <string>
#include <memory>
#include <vector>

class Memory
{
public:
	Memory(std::wstring process_name, std::wstring module_name = L"");
	~Memory();
	bool IsProcessRunning();
	void Suspen();
	void Resume();
	inline bool Read(uintptr_t address, LPVOID buffer_pointer, SIZE_T size)
	{
		SIZE_T read_bytes;
		if (ReadProcessMemory(handle_, (LPCVOID)address, buffer_pointer, size, &read_bytes) == TRUE)
			if (read_bytes == size)
				return true;
		return false;
	}
	inline bool Write(uintptr_t address, LPCVOID buffer_pointer, SIZE_T size)
	{
		SIZE_T read_bytes;
		if (WriteProcessMemory(handle_, (LPVOID)address, buffer_pointer, size, &read_bytes) == TRUE)
			if (read_bytes == size)
				return true;
		return false;
	}
	template<typename T> inline T Read(uintptr_t address, std::vector<int64_t> offsets = {})
	{
		for (auto& offset : offsets)
		{
			if (Read(address, &address, sizeof(address)) == false) return static_cast<T>(NULL);
			address += offset;
		}
		T buffer;
		if (Read(address, &buffer, sizeof(buffer)) == false) return static_cast<T>(NULL);
		return buffer;
	}
	template<typename T> inline void Write(uintptr_t address, T value, std::vector<int64_t> offsets = {})
	{
		for (auto& offset : offsets)
		{
			if (Read(address, &address, sizeof(address)) == false) return;
			address += offset;
		}
		Write(address, &value, sizeof(value));
	}
	inline bool TestBit(uintptr_t address, int offset) { return (Read<int>(address) & (1 << offset)) == (1 << offset); }
	inline bool TestBits(uintptr_t address, int bits) { return (Read<int>(address) & bits) == bits; }
	inline void SetBit(uintptr_t address, int offset, bool toggle) { Write<int>(address, toggle ? (Read<int>(address) | (1 << offset)) : (Read<int>(address) & ~(1 << offset))); }
	inline void SetBits(uintptr_t address, int bits, bool toggle) { Write<int>(address, toggle ? (Read<int>(address) | bits) : (Read<int>(address) & ~bits)); }
private:
	void get_process_id();
	void get_window_handle();
	void get_module_info();
private:
	std::wstring process_name_;
	std::wstring module_name_;
	DWORD process_id_;
	HANDLE handle_;
	HWND window_handle_;
	std::wstring window_text_;
	DWORD_PTR module_addr_;
	DWORD module_size_;
};

inline std::unique_ptr<Memory> g_memory;

inline std::string WideChraracterToMultiByte(std::wstring WideChraracter)
{
    DWORD dBufSize = WideCharToMultiByte(CP_OEMCP, 0, WideChraracter.c_str(), -1, NULL,0,NULL, FALSE);
    char* dBuf = new char[dBufSize];
    memset(dBuf, 0, dBufSize);
    int nRet = WideCharToMultiByte(CP_OEMCP, 0, WideChraracter.c_str(), -1, dBuf, dBufSize, NULL, FALSE);
    std::string str = "";
    if(nRet > 0)
    {
        str = dBuf;
    }
    delete[] dBuf;
    return str;
}

inline std::wstring MultiByteToWideChraracter(std::string MultiByte)
{
    DWORD dBufSize = MultiByteToWideChar(CP_ACP, 0, MultiByte.c_str(), -1, NULL, 0);
    wchar_t* dBuf = new wchar_t[dBufSize];
    wmemset(dBuf, 0, dBufSize);
    int nRet = MultiByteToWideChar(CP_ACP, 0, MultiByte.c_str(), -1, dBuf, dBufSize);
    std::wstring str = L"";
    if(nRet > 0)
    {
        str = dBuf;
    }
    delete[] dBuf;
    return str;
}