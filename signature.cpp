#include "signature.hpp"
#include <Windows.h>
#include <Psapi.h>

Signature::Signature(uint32_t process_id, std::wstring module_name, std::string pattern)
	: process_id_(process_id), module_name_(module_name), pattern_(pattern)
{
	address_ = NULL;
	module_addr_ = NULL;
	module_size_ = NULL;
	get_module_info();
	convert_hex_pattern_to_byte_array();
}

Signature::~Signature(){}

Signature& Signature::Scan()
{
	uint8_t* buffer = new uint8_t[module_size_];
	HANDLE handle = OpenProcess(PROCESS_VM_READ, FALSE, process_id_);
	if (handle != NULL)
	{
		size_t read_bytes = NULL;
		if (ReadProcessMemory(handle, (LPCVOID)module_addr_, buffer, module_size_, &read_bytes))
		{
			if (read_bytes == module_size_)
			{
				for (size_t i = 0; i < module_size_ - bytes_.size() + 1; i++)
				{
					size_t j;
					for (j = 0; j < bytes_.size(); j++)
					{
						if (bytes_[j].has_value() == false) continue;
						if (buffer[i + j] != bytes_[j]) break;
					}
					if (j == bytes_.size())
					{
						address_ = module_addr_ + i;
						break;
					}
				}
			}
		}
		CloseHandle(handle);
	}
	delete[] buffer;
	return *this;
}

Signature& Signature::Add(int32_t offset)
{
	address_ += offset;
	return *this;
}

Signature& Signature::Sub(int32_t offset)
{
	address_ -= offset;
	return *this;
}

Signature& Signature::Rip()
{
	int32_t temp = NULL;
	uintptr_t read_bytes = NULL;
	HANDLE handle = OpenProcess(PROCESS_VM_READ, FALSE, process_id_);
	if (handle != NULL)
	{
		if (ReadProcessMemory(handle, (LPCVOID)address_, &temp, sizeof(int32_t), &read_bytes))
		{
			if (read_bytes == sizeof(int32_t))
			{
				address_ += temp;
				address_ += sizeof(int32_t);
			}
		}
		CloseHandle(handle);
	}
	return *this;
}

void Signature::get_module_info()
{
    HMODULE hMods[1024];
    HANDLE hProcess;
    DWORD cbNeeded;
    unsigned int i;
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_id_);
	if (NULL != hProcess)
	{
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
						MODULEINFO module_info{};
						if (GetModuleInformation(hProcess, hMods[i], &module_info, sizeof(MODULEINFO)) != 0)
						{
							module_addr_ = (uintptr_t)module_info.lpBaseOfDll;
							module_size_ = module_info.SizeOfImage;
						}
					}
				}
			}
		}
		CloseHandle(hProcess);
	}
}

void Signature::convert_hex_pattern_to_byte_array()
{
	auto split = [](std::string s, std::string delimiter)-> std::vector<std::string>
	{
		size_t pos_start = 0, pos_end, delim_len = delimiter.length();
		std::vector<std::string> res;

		while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
			res.push_back(s.substr(pos_start, pos_end - pos_start));
			pos_start = pos_end + delim_len;
		}

		res.push_back(s.substr(pos_start));
		return res;
	};

	std::vector<std::string> str = split(pattern_, " ");
	
	auto to_hex = [](char c)-> uint8_t
	{
		switch (std::toupper(c))
		{
		case '0':
			return static_cast<std::uint8_t>(0);
		case '1':
			return static_cast<std::uint8_t>(1);
		case '2':
			return static_cast<std::uint8_t>(2);
		case '3':
			return static_cast<std::uint8_t>(3);
		case '4':
			return static_cast<std::uint8_t>(4);
		case '5':
			return static_cast<std::uint8_t>(5);
		case '6':
			return static_cast<std::uint8_t>(6);
		case '7':
			return static_cast<std::uint8_t>(7);
		case '8':
			return static_cast<std::uint8_t>(8);
		case '9':
			return static_cast<std::uint8_t>(9);
		case 'A':
			return static_cast<std::uint8_t>(10);
		case 'B':
			return static_cast<std::uint8_t>(11);
		case 'C':
			return static_cast<std::uint8_t>(12);
		case 'D':
			return static_cast<std::uint8_t>(13);
		case 'E':
			return static_cast<std::uint8_t>(14);
		case 'F':
			return static_cast<std::uint8_t>(15);
		default:
			return 0;
		}
	};

	for (auto& item : str)
	{
		if (item[0] == '?')
		{
			bytes_.push_back(std::nullopt);
			continue;
		}
		uint8_t byte = 0;
		for (auto& element : item)
		{
			byte *= 10;
			byte += to_hex(element);
		}
		bytes_.push_back(byte);
	}
}