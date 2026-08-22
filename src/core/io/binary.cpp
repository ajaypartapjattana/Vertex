#include "io.h"

#if defined(PLATFORM_WINDOWS)
  #define WIN32_LEAN_AND_MEAN
  #include <Windows.h>	
#elif defined(PLATFORM_LINUX)
  #include <fcntl.h>
  #include <unistd.h>
  #include <sys/stat.h>
#else
	#error "Unsupported platform"
#endif

#include <cassert>

int io::getBinarySize(const char* const _Path, size_t* const pSize) noexcept {
	assert(_Path && pSize);

#if defined(PLATFORM_WINDOWS)
	HANDLE file = CreateFileW(_Path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	
	if (file == INVALID_HANDLE_VALUE)
		return -1;

	LARGE_INTEGER fileSize;

	if (!GetFileSizeEx(file, &fileSize)) {
		CloseHandle(file);
		return -1;
	}

	*pSize = fileSize.QuadPart;
		
	CloseHandle(file);
	return 0;
#elif defined(PLATFORM_LINUX)
	int fileDescriptor = open(_Path, O_RDONLY);

	if (fileDescriptor < 0)
		return -1;

	struct stat info;

	if(fstat(fileDescriptor, &info)){
		close(fileDescriptor);
		return -1;
	}

	*pSize = static_cast<size_t>(info.st_size);

	close(fileDescriptor);
	return 0;
#endif
}

int io::loadBinary(const char* const _Path, const size_t _LoadSize, void* const pDst) noexcept {
	assert(_Path && pDst);

#if defined(PLATFORM_WINDOWS)
	HANDLE file = CreateFileW(_Path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	
	if (file == INVALID_HANDLE_VALUE)
		return -1;

	DWORD bytesRead = 0;
	if (!ReadFile(file, pDst, (DWORD)*_LoadSize, &bytesRead, nullptr)) {
		CloseHandle(file);
		return -1;
	}

	CloseHandle(file);
	return bytesRead == _LoadSize ? 0 : -1;
#elif defined(PLATFORM_LINUX)
	int fileDescriptor = open(_Path, O_RDONLY);

	if (fileDescriptor < 0)
		return -1;

	size_t remaining = _LoadSize;
	uint8_t* dst = static_cast<uint8_t*>(pDst);

	size_t total = 0;

	while (remaining) {
		ssize_t result = read(fileDescriptor, dst, remaining);

		if (result < 0) {
			close(fileDescriptor);
			return -1;
		}

		dst += result;
		remaining -= result;
		total += result;
	}

	close(fileDescriptor);
	return 0;
#endif
	
}