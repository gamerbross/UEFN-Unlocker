#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <iostream>

namespace Memory {
	//SigScan (not mine)
	uintptr_t SigScan(const char* signature, bool bRelative = false, uint32_t offset = 0) {
		uintptr_t base_address = reinterpret_cast<uintptr_t>(GetModuleHandle(NULL));
		static auto patternToByte = [](const char* pattern)
		{
			auto bytes = std::vector<int>{};
			const auto start = const_cast<char*>(pattern);
			const auto end = const_cast<char*>(pattern) + strlen(pattern);

			for (auto current = start; current < end; ++current)
			{
				if (*current == '?')
				{
					++current;
					if (*current == '?') ++current;
					bytes.push_back(-1);
				}
				else { bytes.push_back(strtoul(current, &current, 16)); }
			}
			return bytes;
		};

		const auto dosHeader = (PIMAGE_DOS_HEADER)base_address;
		const auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)base_address + dosHeader->e_lfanew);

		const auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
		auto patternBytes = patternToByte(signature);
		const auto scanBytes = reinterpret_cast<std::uint8_t*>(base_address);

		const auto s = patternBytes.size();
		const auto d = patternBytes.data();

		for (auto i = 0ul; i < sizeOfImage - s; ++i)
		{
			bool found = true;
			for (auto j = 0ul; j < s; ++j)
			{
				if (scanBytes[i + j] != d[j] && d[j] != -1)
				{
					found = false;
					break;
				}
			}
			if (found)
			{
				uintptr_t address = reinterpret_cast<uintptr_t>(&scanBytes[i]);
				if (bRelative)
				{
					address = ((address + offset + 4) + *(int32_t*)(address + offset));
					return address;
				}
				return address;
			}
		}
		return 0;
	}

	uintptr_t GetAddressFromOffset(DWORD Offset) {
		return reinterpret_cast<uintptr_t>(GetModuleHandle(NULL) + Offset);
	}

	uintptr_t GetAddressFromSig(const char* Sig)
	{
		if (Sig == "")
			return 0;
		uintptr_t Addr = SigScan(Sig);
		return Addr;
	}

	uintptr_t GetAddressFromSigR(const char* Sig, const char* Name, uint32_t ROffset)
	{
		if (Sig == "")
			return 0;
		std::cout << "\nFinding Address for " << Name << "!" << std::endl;
		uintptr_t Addr = SigScan(Sig, true, ROffset);
		return Addr;
	}
}