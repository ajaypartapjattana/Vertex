#include "io.h"

struct VertexKey {
	uint32_t position;
	uint32_t texCoord;
	uint32_t normal;
};

static uint64_t hashVertexKey(const VertexKey* const pKey) noexcept {
	uint64_t x = pKey->position;
	x = x * 0x9E3779B185EBCA87ull + pKey->texCoord;
	x = x * 0x9E3779B185EBCA87ull + pKey->normal;

	x ^= x >> 30;

	return 0;
}

enum WavefrontRecordType {
	WAVEFRONT_RECORD_TYPE_COMMENT,
	WAVEFRONT_RECORD_TYPE_VECTOR,
	WAVEFRONT_RECORD_TYPE_FACE,
	WAVEFRONT_RECORD_TYPE_OBJECT,
	WAVEFRONT_RECORD_TYPE_GROUP,
	WAVEFRONT_RECORD_TYPE_MTLLIB,
	WAVEFRONT_RECORD_TYPE_USEMTL,
	WAVEFRONT_RECORD_TYPE_SMOOTHING
};

enum WavefrontVectorType {
	WAVEFRONT_VECTOR_TYPE_POSITION,
	WAVEFRONT_VECTOR_TYPE_TEXTURE_COORDINATE,
	WAVEFRONT_VECTOR_TYPE_NORMAL
};

static const char* getNextCharPtr(const char* _Ptr) noexcept {
	while (*_Ptr == ' ' || *_Ptr == '\t' || *_Ptr == '\r') {
		++_Ptr;
	}

	return _Ptr;
}

static const char* getLineEndCharPtr(const char* _Ptr) noexcept {
	while (*_Ptr != '\n') {
		++_Ptr;
	}

	return _Ptr;
}

using VertexAttributeFlags = uint32_t;
enum VertexAttributeFlagBits : VertexAttributeFlags {
	VERTEX_ATTRIBUTE_POSITION_BIT = 1u << 0,
	VERTEX_ATTRIBUTE_TEXTURE_COORDINATE_BIT = 1u << 1,
	VERTEX_ATTRIBUTE_NORMAL_BIT = 1u << 2,
};

struct VertexAttributeOffset{
	uint32_t position;
	uint32_t textureCoordinate;
	uint32_t normal;
};

struct TargetWriteInfo {
	VertexAttributeFlags attributes;
	VertexAttributeOffset offset;
	void* pVertexDst;
	const void* pVertexLimit;
	void* pIndexDst;
	const void* pIndexLimit;
};

static bool parseFloat(const char** const pPtr, float* const pFloat) noexcept {
	bool negative = false;

	const char* _Ptr = getNextCharPtr(*pPtr);
	
	if (*_Ptr == '-') {
		negative = true;
		++_Ptr;
	}

	if (*_Ptr == ' ' || *_Ptr == '.')
		return false;

	uint64_t integar = 0;

	while (*_Ptr >= '0' && *_Ptr <= '9')
		integar = integar * 10u + (*_Ptr++ - '0');
	
	if (*_Ptr != '.')
		return false;

	float fraction = 0.0f;
	float scale = 0.1f;

	while (*_Ptr >= '0' && *_Ptr <= '9') {
		fraction = (*_Ptr++ - '0') * scale;
		scale *= 0.1f;
	}

	float Val = static_cast<float>(integar) + fraction;

	Val = negative ? -Val : Val;
	
	*pFloat = Val;
	*pPtr = _Ptr;
	
	return true;
}

int parseWavefront(const void* const pSrc, const size_t _Size, const TargetWriteInfo* const pTarget) noexcept {
	const char* cursor = reinterpret_cast<const char*>(pSrc);
	const char* const pEnd = reinterpret_cast<const char*>(pSrc) + _Size;

	float* const pDst = reinterpret_cast<float*>(pTarget->pVertexDst);

	while (cursor != pEnd) {
		switch (*cursor++) {
			case 'v':{
				switch (*cursor++) {
				case ' ':
					cursor = getNextCharPtr(cursor);
					if (!parseFloat(&cursor, pDst))
						return -1;
					
					break;

				case 't':
				
					break;
				
				case 'n':

					break;

				default:
					return -1;
				}
			}

				break;
			
			case '#': 
				break;

			
				

		}
		
	}
	
}