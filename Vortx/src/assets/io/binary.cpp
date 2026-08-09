#include "io.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cassert>

int io::loadBinary(const wchar_t* const _Path, size_t* const pSize, void* const pDst) noexcept {
	assert(_Path && pSize);

	HANDLE file = CreateFileW(_Path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (file == INVALID_HANDLE_VALUE)
		return -1;

	BOOL result;

	if (!pDst) {
		LARGE_INTEGER fileSize;
		result = GetFileSizeEx(file, &fileSize);

		if (!result) {
			CloseHandle(file);

			return -1;
		}

		*pSize = fileSize.QuadPart;
		CloseHandle(file);

		return 0;
	}

	DWORD bytesRead = 0;
	result = ReadFile(file, pDst, (DWORD)*pSize, &bytesRead, nullptr);

	*pSize = static_cast<size_t>(bytesRead);
	
	CloseHandle(file);
	
	if (!result)
		return -1;

	return 0;
}