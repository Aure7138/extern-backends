#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <optional>

class Signature
{
public:
	Signature(uint32_t process_id, std::wstring module_name, std::string pattern);
	~Signature();
	Signature& Scan();
	Signature& Add(int32_t offset);
	Signature& Sub(int32_t offset);
	Signature& Rip();
	template <typename T> T As(){ return static_cast<T>(address_ < module_addr_ ? 0 : address_); }
private:
	void get_module_info();
	void convert_hex_pattern_to_byte_array();
private:
	uintptr_t address_;
	uint32_t process_id_;
	std::wstring module_name_;
	std::string pattern_;
	uintptr_t module_addr_;
	uint32_t module_size_;
	std::vector<std::optional<uint8_t>> bytes_;
};