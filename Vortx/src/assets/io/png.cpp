#include "io.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <array>
#include <stdexcept>
#include <cassert>

#include <core/Memory/memory.h>

namespace io {

	template <bool(*GetChunkFn)(void* user, const uint8_t** pBegin, size_t* pSize) noexcept>
	class BitStream {
	private:
		void* user;

		uint64_t bitBuffer = 0;
		size_t bitCount = 0;

		const uint8_t* cursor = nullptr;
		const uint8_t* pEnd = nullptr;

		constexpr uint64_t bitMask(size_t bits) const noexcept {
			return bits == 64 ? ~0ull : ((1ull << bits) - 1);
		}

		void refill() noexcept {
			while (bitCount <= 32) {
				if (cursor + 4 <= pEnd) {
					uint32_t word;
					memcpy(&word, cursor, sizeof(word));

					bitBuffer |= uint64_t(word) << bitCount;

					cursor += 4;
					bitCount += 32;

					return;
				}

				while (cursor != pEnd) {
					bitBuffer |= uint64_t(*cursor++) << bitCount;
					bitCount += 8;
				}

				size_t chunkSize = 0;
				if (!GetChunkFn(user, &cursor, &chunkSize))
					return;

				pEnd = cursor + chunkSize;
			}
		}

	public:
		BitStream() noexcept = default;
		BitStream(void* _User) noexcept
			: user(_User)
		{
			refill();
		}

		inline void consume(size_t bits) noexcept {
			bitBuffer >>= bits;
			bitCount -= bits;
		}

		inline void alignByte() noexcept {
			bitBuffer >>= (bitCount & 7);
			bitCount &= ~7;
		}

		constexpr const uint8_t* readBytes(size_t count) noexcept {
			alignByte();

			const uint8_t* p = cursor;
			cursor += count;

			return p;
		}

		uint8_t read_Byte() noexcept {
			const uint8_t* p = readBytes(1);

			return *p;
		}

		constexpr uint16_t read_LE16() noexcept {
			const uint8_t* p = readBytes(2);

			return uint16_t(p[0]) | (uint16_t(p[1]) << 8);
		}

		int copy_raw(void* const pDst, size_t _ByteCount) noexcept {
			assert(bitCount & 7u == 0);
			
			uint8_t* _pDst = reinterpret_cast<uint8_t*>(pDst);
			size_t remaining = _ByteCount;

			const uint32_t bufferBytes = bitCount >> 3u;
			
			if (bufferBytes) {
				size_t bufferCopy = std::min<size_t>(bufferBytes, remaining);

				memcpy(_pDst, &bitBuffer, bufferCopy);
				consume(bufferCopy << 3u);

				_pDst += bufferCopy;
				remaining -= bufferCopy;
			}

			while (remaining) {
				size_t available = static_cast<size_t>(pEnd - cursor);

				if (!available) {
					size_t chunkSize;
					if (!GetChunkFn(user, &cursor, &chunkSize))
						return -1;

					pEnd = cursor + chunkSize;
				}

				const size_t streamCopy = std::min<size_t>(available, remaining);

				memcpy(_pDst, cursor, streamCopy);
				cursor += streamCopy;

				_pDst += streamCopy;
				remaining -= streamCopy;
			}

			return 0;
		}

		const void* flushRange(size_t* const _Size) noexcept {
			assert(bitCount & 7u == 0);
			
			const size_t bufferBytes = bitCount >> 3u;

			if (bufferBytes) {
				*_Size = std::min<size_t>(*_Size, bufferBytes);

				return &bitBuffer;
			}

			const size_t streamBytes = static_cast<size_t>(pEnd - cursor);
			*_Size = std::min<size_t>(*_Size, streamBytes);

			return cursor;
		}

		void consumeBytes(size_t _Size) noexcept {
			assert((bitCount & 7u) == 0);

			const size_t bufferBytes = bitCount >> 3u;

			if (bufferBytes) {
				const size_t bufferConsume = std::min<size_t>(bufferBytes, _Size);
				consume(bufferConsume << 3u);
				
				_Size -= bufferConsume;

				if (!_Size)
					return;
			}

			assert(_Size <= static_cast<size_t>(pEnd - cursor) + bufferBytes);

			cursor += _Size;

			if (cursor == pEnd) {
				size_t chunkSize = 0;

				if (GetChunkFn(user, &cursor, &chunkSize))
					pEnd = cursor + chunkSize;
			}
		}

		uint32_t read(size_t bits) noexcept {
			if (bitCount < bits)
				refill();

			uint32_t value = static_cast<uint32_t>(bitBuffer) & bitMask(bits);

			bitBuffer >>= bits;
			bitCount -= bits;

			return value;
		}

		uint64_t peek(size_t _BitCount) {
			if(bitCount < _BitCount)
				refill();

			return bitBuffer & bitMask(_BitCount);
		}

		bool endsBefore(size_t byteCount) const noexcept {
			return cursor + byteCount >= pEnd;
		}

		bool ensure(size_t bits) noexcept {
			refill();

			return bitCount > bits;
		}

	};

	enum HflEntryType : uint8_t {
		HFL_ENTRY_TYPE_UNDEFINED_BIT,
		HFL_ENTRY_TYPE_LITERAL_BIT,
		HFL_ENTRY_TYPE_BLOCK_END_BIT,
		HFL_ENTRY_TYPE_LENGTH_BIT,
		HFL_ENTRY_TYPE_DISTANCE_BIT,
		HFL_ENTRY_TYPE_REPEAT_BIT,
		HFL_ENTRY_TYPE_ZERO_INITIALIZE_BIT,
		HFL_ENTRY_TYPE_SUBTABLE_CANDIDATE_BIT,
		HFL_ENTRY_TYPE_SUBTABLE_BIT,
	};

	struct HflEntry {
		uint32_t data;

		inline HflEntry(const uint16_t _Val, const HflEntryType _Type, const uint8_t _Extra, const uint8_t _Bits) noexcept 
			: data(uint32_t(_Val) << 12 | uint32_t(_Type) << 8 | uint32_t(_Extra) << 4 | uint32_t(_Bits)) 
		{

		}

		constexpr uint32_t bits() const noexcept {
			return data & 0xF;
		}

		constexpr uint32_t extraBits() const noexcept {
			return (data >> 4u) & 0xF;
		}

		constexpr HflEntryType type() const noexcept {
			return static_cast<HflEntryType>((data >> 8u) & 0xF);
		}

		constexpr uint32_t value() const noexcept {
			return data >> 12u;
		}

		inline void set_Val(const uint16_t _Val) noexcept {
			data = (uint32_t(_Val) << 12u) | (data & 0x0FFF);
		}

		inline void set_Bits(const uint8_t _Bits) noexcept {
			data = (data & ~0xFu) | (_Bits & 0xF);
		}

		inline void set_Extra(const uint8_t _Extra) noexcept {
			data = (data & ~(0xFu << 4)) | ((uint32_t(_Extra) & 0xF) << 4);
		}

		inline void set_Type(const HflEntryType _Type) noexcept {
			data = (data & ~(0xFu << 8)) | ((uint32_t(_Type) & 0xF) << 8);
		}

	};

	constexpr uint16_t FIXED_HUFFMAN_LENGTH_BASE[29]{ 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258 };
	constexpr uint8_t FIXED_HUFFMAN_LENGTH_EXTRA[29]{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0 };

	inline HflEntry createEntryTypeLIT(const uint16_t _Symbol, const uint8_t _Bits) noexcept {
		if (_Symbol < 256u) {
			return HflEntry{ _Symbol, HFL_ENTRY_TYPE_LITERAL_BIT, 0, _Bits };
		}
		else if (_Symbol == 256) {
			return HflEntry{ 256, HFL_ENTRY_TYPE_BLOCK_END_BIT, 0, _Bits };
		}
		else {
			const uint16_t lengthIndex = _Symbol - 257u;
			return HflEntry{ FIXED_HUFFMAN_LENGTH_BASE[lengthIndex], HFL_ENTRY_TYPE_LENGTH_BIT, FIXED_HUFFMAN_LENGTH_EXTRA[lengthIndex], _Bits };
		}
	}

	constexpr uint16_t FIXED_HUFFMAN_DISTANCE_BASE[30]{ 1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577 };
	constexpr uint8_t FIXED_HUFFMAN_DISTANCE_EXTRA[30]{ 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13 };
	
	inline HflEntry createEntryTypeDIST(const uint16_t _Symbol, const uint8_t _Bits) noexcept {
		return HflEntry{ FIXED_HUFFMAN_DISTANCE_BASE[_Symbol], HFL_ENTRY_TYPE_DISTANCE_BIT, FIXED_HUFFMAN_DISTANCE_EXTRA[_Symbol], _Bits };
	}

	inline HflEntry createEntryTypeCLEN(const uint16_t _Symbol, const uint8_t _Bits) noexcept {
		if (_Symbol < 16)
			return HflEntry{ _Symbol, HFL_ENTRY_TYPE_LITERAL_BIT, 0, _Bits };
		
		switch (_Symbol) {
		case 16: return HflEntry{ 3, HFL_ENTRY_TYPE_REPEAT_BIT, 2, _Bits };
		case 17: return HflEntry{ 3, HFL_ENTRY_TYPE_ZERO_INITIALIZE_BIT, 3, _Bits };
		case 18: return HflEntry{ 11, HFL_ENTRY_TYPE_ZERO_INITIALIZE_BIT, 7, _Bits };
		}
		return HflEntry{ _Symbol, HFL_ENTRY_TYPE_LITERAL_BIT, 0, _Bits };
	}

	inline uint16_t reverseBits(uint16_t x, uint8_t bits) noexcept {
		x = ((x & 0x5555) << 1) | ((x >> 1) & 0x5555);
		x = ((x & 0x3333) << 2) | ((x >> 2) & 0x3333);
		x = ((x & 0x0F0F) << 4) | ((x >> 4) & 0x0F0F);

		return (x >> 8 | x << 8) >> (16 - bits);
	}

	inline uint32_t reverseBits(uint32_t x, uint8_t bits) noexcept {
		x = ((x & 0x55555555u) << 1) | ((x >> 1) & 0x55555555u);
		x = ((x & 0x33333333u) << 2) | ((x >> 2) & 0x33333333u);
		x = ((x & 0x0F0F0F0Fu) << 4) | ((x >> 4) & 0x0F0F0F0Fu);
		x = ((x & 0x00FF00FFu) << 8) | ((x >> 8) & 0x00FF00FFu);

		return ((x >> 16) | (x << 16)) >> (32 - bits);
	}

	template <unsigned _RootBits, HflEntry(*translator)(uint16_t _Symbol, uint8_t _Bits) noexcept>
	void* createHuffmanLookupTable(HflEntry* const pTable, const uint8_t* pLengths, const uint32_t _SymbolCount) noexcept {
		const uint8_t* const pLengthsEnd = pLengths + _SymbolCount;

		uint16_t count[16]{};

		for (const uint8_t* pLength{ pLengths }; pLength != pLengthsEnd; ++pLength)
			++count[*pLength];
		
		count[0] = 0;

		uint32_t nextCode[16]{};

		uint32_t code = 0;
		for (uint32_t bits = 1; bits <= 15; ++bits) {
			code = (code + count[bits - 1]) << 1u;
			nextCode[bits] = code;
		}

		uint32_t pCode[16];

		memset(pTable, 0, (1u << _RootBits) * sizeof(HflEntry));
		
		memcpy(pCode, nextCode, sizeof(nextCode));

		for (const uint8_t* pLength{ pLengths }; pLength != pLengthsEnd; ++ pLength) {
			const uint8_t length = *pLength;

			if (length <= _RootBits)
				continue;

			uint32_t code = reverseBits(pCode[length]++, length);
			const uint32_t prefix = code & ((1u << _RootBits) - 1u);
			const uint8_t remaining = length - _RootBits;

			pTable[prefix] = HflEntry(0, HFL_ENTRY_TYPE_SUBTABLE_CANDIDATE_BIT, std::max<uint8_t>(pTable[prefix].extraBits(), remaining), _RootBits);
		}

		HflEntry* pNext = pTable + (1u << _RootBits);

		memcpy(pCode, nextCode, sizeof(nextCode));

		for (uint32_t i = 0; i < (1u << _RootBits); ++i) {
			if (pTable[i].type() != HFL_ENTRY_TYPE_SUBTABLE_CANDIDATE_BIT)
				continue;
			
			const uint32_t offset = static_cast<uint32_t>(pNext - pTable);

			const uint32_t subtableBits = pTable[i].extraBits();

			pTable[i].set_Val(offset);
			pTable[i].set_Type(HFL_ENTRY_TYPE_SUBTABLE_BIT);

			pNext += 1ull << subtableBits;
		}

		for (uint16_t symbol = 0; symbol < _SymbolCount; ++symbol) {
			const uint8_t length = pLengths[symbol];

			if (!length)
				continue;

			uint32_t code = reverseBits(nextCode[length]++, length);

			if (length > _RootBits) {
				const uint32_t prefix = code & ((1u << _RootBits) - 1u);

				const uint8_t tableBits = pTable[prefix].extraBits();
				const uint32_t Offset = pTable[prefix].value();

				HflEntry* const pSubTable = pTable + Offset;

				const uint32_t remaining = length - _RootBits;

				const uint32_t start = code >> _RootBits & ((1u << remaining) - 1u);
				const uint32_t fill = 1u << (tableBits - remaining);

				const HflEntry entry = translator(symbol, remaining);

				for (uint32_t i = 0; i < fill; ++i)
					pSubTable[start | (i << remaining)] = entry;

				continue;
			}

			const HflEntry entry = translator(symbol, length);
			
			const uint32_t fill = 1u << (_RootBits - length);

			for (uint32_t i = 0; i < fill; ++i)
				pTable[code | (i << length)] = entry;
		}

		return pNext;
	}

	inline uint32_t read_u32_be(const uint8_t* const _Ptr) noexcept {
		uint32_t raw;
		memcpy(&raw, _Ptr, 4);

		return _byteswap_ulong(raw);
	}

	int fetchPngInfo(const void* pBin, size_t _Size, ImageInfo* const pInfo) noexcept {
		if (_Size < 67)
			return -1;

		constexpr uint64_t PNG_SIGNATURE = 0x0A1A0A0D474E5089ull;

		uint64_t signature;
		memcpy(&signature, pBin, 8);

		if (signature != PNG_SIGNATURE)
			return -1;

		const uint8_t* pDat = reinterpret_cast<const uint8_t*>(pBin) + 8;

		if (read_u32_be(pDat) != 13)
			return -1;
		
		pDat += 4;
		if (memcmp(pDat, "IHDR", 4))
			return -1;

		pDat += 4;
		pInfo->width = read_u32_be(pDat);
		
		pDat += 4;
		pInfo->height = read_u32_be(pDat);

		pDat += 4;
		pInfo->bitDepth = pDat[0];
		
		switch (pDat[1]) {
		case 0: pInfo->channels = 1;
			break;

		case 2: pInfo->channels = 3;
			break;

		case 3: pInfo->channels = 1;
			break;

		case 4: pInfo->channels = 2;
			break;

		case 6: pInfo->channels = 4;
			break;

		default:
			return -1;

		}

		return 0;
	}

	inline uint8_t paeth(uint8_t a, uint8_t b, uint8_t c) noexcept {
		const int p = int(a) + int(b) + int(c);

		const int pa = abs(p - int(a));
		const int pb = abs(p - int(b));
		const int pc = abs(p - int(c));

		if (pa <= pb && pa <= pc)
			return a;

		if (pb <= pc)
			return b;

		return c;
	}

	struct ImageResolveState {
		uint8_t* pImage;
		uint8_t* pScanLine;
		uint8_t* pDst;

		uint16_t filter = 0;

		const uint16_t texelSize;
		const uint32_t rowSize;
		const uint64_t imageSize;

		ImageResolveState(void* const _Dst, const ImageInfo* const pInfo) noexcept
			: pImage(reinterpret_cast<uint8_t*>(_Dst))
			, pScanLine(reinterpret_cast<uint8_t*>(_Dst))
			, pDst(reinterpret_cast<uint8_t*>(_Dst))
			, texelSize((pInfo->bitDepth* pInfo->channels) >> 3u)
			, rowSize(texelSize* pInfo->width)
			, imageSize(rowSize* pInfo->height)
		{
			assert(((pInfo->bitDepth * pInfo->channels) & 7u) == 0);
		}
	};

	void* resolveImage(ImageResolveState* const pState, const void* pBegin, const void* const pEnd) noexcept {
		const uint8_t* cursor = reinterpret_cast<const uint8_t*>(pBegin);

		while (cursor != pEnd) {
			if (pState->pDst == pState->pScanLine)
				pState->filter = *cursor++;

			const size_t byteOffset = static_cast<size_t>(pState->pDst - pState->pScanLine);
			const size_t remaining = pState->rowSize - byteOffset;
			const size_t available = static_cast<size_t>(reinterpret_cast<const uint8_t*>(pEnd) - cursor);

			size_t byteResolve = std::min<size_t>(remaining, available);

			switch (pState->filter) {
			case 0:
				memcpy(pState->pDst, cursor, byteResolve);

				pState->pDst += byteResolve;
				cursor += byteResolve;

				break;

			case 1:
				if (byteOffset < pState->texelSize) {
					size_t prefix = std::min<size_t>(pState->texelSize - byteOffset, byteResolve);

					memcpy(pState->pDst, cursor, prefix);

					pState->pDst += prefix;
					cursor += prefix;
					byteResolve -= prefix;
				}

				{
					uint8_t* _pDst = pState->pDst;

					while (byteResolve--) {
						*_pDst = static_cast<uint8_t>(*cursor + _pDst[-static_cast<ptrdiff_t>(pState->texelSize)]);

						++_pDst;
						++cursor;
					}

					pState->pDst = _pDst;
				}

				break;

			case 2:
				if (pState->pScanLine == pState->pImage) {
					memcpy(pState->pDst, cursor, byteResolve);

					pState->pDst += byteResolve;
					cursor += byteResolve;

					break;
				}

				{
					uint8_t* _pDst = pState->pDst;
					const uint8_t* up = pState->pDst - pState->rowSize;

					while (byteResolve--) {
						*_pDst = static_cast<uint8_t>(*cursor + *up);

						++_pDst;
						++cursor;
						++up;
					}

					pState->pDst = _pDst;
				}

				break;

			case 3:
				if (byteOffset < pState->texelSize) {
					size_t prefix = std::min<size_t>(pState->texelSize - byteOffset, byteResolve);

					if (pState->pScanLine == pState->pImage) {
						memcpy(pState->pDst, cursor, prefix);

						pState->pDst += prefix;
						cursor += prefix;
					}
					else {
						uint8_t* _pDst = pState->pDst;
						const uint8_t* up = pState->pDst - pState->rowSize;

						size_t prefixBytes = prefix;

						while (prefixBytes--) {
							*_pDst = static_cast<uint8_t>(*cursor + (*up >> 1));

							++_pDst;
							++cursor;
							++up;
						}

						pState->pDst = _pDst;
					}

					byteResolve -= prefix;
				}

				if (pState->pDst == pState->pScanLine + pState->rowSize)
					break;

				if (pState->pScanLine == pState->pImage) {
					uint8_t* _pDst = pState->pDst;

					while (byteResolve--) {
						*_pDst = static_cast<uint8_t>(*cursor + (_pDst[-static_cast<ptrdiff_t>(pState->texelSize)] >> 1));

						++_pDst;
						++cursor;
					}

					pState->pDst = _pDst;

					break;
				}

				{
					uint8_t* _pDst = pState->pDst;
					const uint8_t* up = pState->pDst - pState->rowSize;

					while (byteResolve--) {
						*_pDst = static_cast<uint8_t>(*cursor + ((uint32_t(_pDst[-pState->texelSize]) + *up) >> 1u));

						++_pDst;
						++cursor;
						++up;
					}

					pState->pDst = _pDst;
				}

				break;

			case 4:
				if (byteOffset < pState->texelSize) {
					size_t prefix = std::min<size_t>(pState->texelSize - byteOffset, byteResolve);

					if (pState->pScanLine == pState->pImage) {
						memcpy(pState->pDst, cursor, prefix);

						pState->pDst += prefix;
						cursor += prefix;
					}
					else {
						uint8_t* _pDst = pState->pDst;
						const uint8_t* up = pState->pDst - pState->rowSize;

						size_t prefixBytes = prefix;

						while (prefixBytes--) {
							*_pDst = static_cast<uint8_t>(*cursor + *up);

							++_pDst;
							++cursor;
							++up;
						}

						pState->pDst = _pDst;
					}

					byteResolve -= prefix;
				}

				if (pState->pDst == pState->pScanLine + pState->rowSize)
					break;

				if (pState->pScanLine == pState->pImage) {
					uint8_t* _pDst = pState->pDst;

					while (byteResolve--) {
						*_pDst = static_cast<uint8_t>(*cursor + _pDst[-static_cast<ptrdiff_t>(pState->texelSize)]);

						++_pDst;
						++cursor;
					}

					pState->pDst = _pDst;

					break;
				}

				{
					uint8_t* _pDst = pState->pDst;
					const uint8_t* up = pState->pDst - pState->rowSize;

					while (byteResolve--) {
						*_pDst = static_cast<uint8_t>(*cursor + paeth(_pDst[-static_cast<ptrdiff_t>(pState->texelSize)], *up, up[-static_cast<ptrdiff_t>(pState->texelSize)]));

						++_pDst;
						++cursor;
						++up;
					}

					pState->pDst = _pDst;
				}

				break;

			default:
				assert(0);

			}

			if (static_cast<size_t>(pState->pDst - pState->pScanLine) == pState->rowSize)
				pState->pScanLine = pState->pDst;
		}

		return (void*)cursor;
	}

	struct ImageResolveBuffer{
		mem::span<uint8_t> storage;
		uint8_t* pCurrent;
		const uint8_t* pResolve;

		ImageResolveBuffer(mem::span<uint8_t> _Span) noexcept
			: storage(_Span)
			, pCurrent(_Span.pBegin)
			, pResolve(_Span.pBegin)
		{
		
		}

		inline void push_Literal(const uint8_t _Val) noexcept {
			*pCurrent++ = _Val;
		}

		inline void copyDistance(const size_t _Distance, size_t _Length) noexcept {
			uint8_t* src = pCurrent - _Distance;
			while (_Length--) {
				*pCurrent++ = *src++;
			}
		}

	};

	struct PNG_ParserStatus {
		const uint8_t* pCursor;
		const uint8_t* pEnd;
	};

	bool getChunk_PNG(void* pUser, const uint8_t** pBegin, size_t* pSize) noexcept {
		PNG_ParserStatus* pStatus = reinterpret_cast<PNG_ParserStatus*>(pUser);

		while (pStatus->pCursor < pStatus->pEnd) {
			const uint8_t* chunk = pStatus->pCursor;

			assert(static_cast<size_t>(pStatus->pEnd - chunk) >= 12);

			const uint32_t length = read_u32_be(chunk);
			const uint8_t* signature = chunk + 4;
			
			if (!memcmp(signature, "IEND", 4))
				return false;
			
			const uint8_t* data = chunk + 8;
			const uint8_t* next = data + length + 4;

			assert(next <= pStatus->pEnd);

			pStatus->pCursor = next;

			if (!memcmp(signature, "IDAT", 4)) {
				*pBegin = data;
				*pSize = length;

				return true;
			}
		}

		return false;
	}

	constexpr uint8_t* generateStaticHLIT(uint8_t* pLengths) noexcept {
		uint8_t* pEnd = pLengths + 144u;

		while (pLengths != pEnd)
			*pLengths++ = 8;

		pEnd = pLengths + 112u;
		while (pLengths != pEnd)
			*pLengths++ = 9;

		pEnd = pLengths + 24u;
		while (pLengths != pEnd)
			*pLengths++ = 7;

		pEnd = pLengths + 8u;
		while (pLengths != pEnd)
			*pLengths++ = 8;

		return pLengths;
	}

	constexpr uint8_t* generateStaticHDIST(uint8_t* pLengths) noexcept {
		uint8_t* const pEnd = pLengths + 32u;

		while (pLengths != pEnd)
			*pLengths++ = 5;

		return pLengths;
	}

	constexpr size_t HLIT_TABLE_SIZE = 2048;
	constexpr size_t HDIST_TABLE_SIZE = 1024;

	constexpr uint8_t DYNAMIC_HUFFMAN_LENGTH_ORDER[19]{ 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

	bool verifyKraft(const uint8_t* lengths, size_t count, uint32_t maxBits = 15)
	{
		uint32_t counts[16]{};

		for (size_t i = 0; i < count; i++) {
			if (lengths[i] > maxBits)
				return false;

			if (lengths[i])
				counts[lengths[i]]++;
		}

		uint32_t left = 1u << maxBits;

		for (uint32_t len = 1; len <= maxBits; len++) {
			left -= counts[len] << (maxBits - len);

			if ((int32_t)left < 0)
				return false;
		}

		return left == 0;
	}

	int decodePng(void* pDst, const ImageInfo* const pInfo, const void* const pSrc, size_t _SrcSize) noexcept {
		const uint8_t* const pBegin = reinterpret_cast<const uint8_t*>(pSrc) + 8;
		const uint8_t* const pEnd = reinterpret_cast<const uint8_t*>(pSrc) + _SrcSize;

		PNG_ParserStatus chunkParserState = { pBegin, pEnd };
		BitStream<getChunk_PNG> stream{ &chunkParserState };

		if (stream.endsBefore(2))
			return -1;

		uint8_t cmf = stream.read(8);
		uint8_t flg = stream.read(8);

		if (cmf != 0x78)
			return -1;

		if (flg & 0x20)
			return -1;

		if (((uint16_t(cmf) << 8) | flg) % 31 != 0)
			return -1;

		mem::scratch.mark();

		ImageResolveState target{ pDst, pInfo };

		uint8_t lengths[318u];

		mem::span<HflEntry> literalLengthTable = mem::scratch.alloc<HflEntry>(HLIT_TABLE_SIZE);
		mem::span<HflEntry> distanceTable = mem::scratch.alloc<HflEntry>(HDIST_TABLE_SIZE);

		const size_t rowBytes = ((size_t)pInfo->width * pInfo->channels * pInfo->bitDepth + 7) >> 3;
		const size_t filteredSize = (rowBytes + 1) * pInfo->height;

		mem::span<uint8_t> resolveBuffer = mem::scratch.alloc<uint8_t>(filteredSize);
		ImageResolveBuffer buffer{ resolveBuffer };

		uint16_t BFINAL;
		uint16_t BTYPE;

		do {
			if (stream.endsBefore(1))
				return -1;

			BFINAL = stream.read(1);
			BTYPE = stream.read(2);

			switch (BTYPE) {
			case 0: {
				uint16_t BLEN = _byteswap_ushort(static_cast<uint16_t>(stream.read(16)));
				uint16_t NBLEN = _byteswap_ushort(static_cast<uint16_t>(stream.read(16)));

				if (~BLEN != NBLEN)
					return -1;

				size_t rangeSize = static_cast<size_t>(BLEN);
				
				do {
					const void* pRange = stream.flushRange(&rangeSize);
					resolveImage(&target, pRange, reinterpret_cast<const uint8_t*>(pRange) + rangeSize);

					stream.consumeBytes(rangeSize);
					BLEN -= rangeSize;
				} while (BLEN);
			}
				
				break;

			case 1: {
				uint8_t* const LIT_base = lengths;

				uint8_t* const DIST_base = generateStaticHLIT(LIT_base);
				createHuffmanLookupTable<9, createEntryTypeLIT>(literalLengthTable, LIT_base, 288);

				generateStaticHDIST(DIST_base);
				createHuffmanLookupTable<6, createEntryTypeDIST>(distanceTable, DIST_base, 32);

				while (true) {
					HflEntry entry = literalLengthTable[stream.peek(9)];
					stream.consume(entry.bits());

					const HflEntryType type = entry.type();

					if (type == HFL_ENTRY_TYPE_BLOCK_END_BIT)
						break;

					switch (type) {
					case HFL_ENTRY_TYPE_LITERAL_BIT:
						buffer.push_Literal(static_cast<uint8_t>(entry.value()));

						break;

					case HFL_ENTRY_TYPE_LENGTH_BIT: {
						uint32_t extra = entry.extraBits();
						const uint32_t length = extra ? entry.value() + uint32_t(stream.read(extra)) : entry.value();

						entry = distanceTable[stream.peek(6)];
						stream.consume(entry.bits());

						extra = entry.extraBits();
						const uint32_t distance = extra ? entry.value() + uint32_t(stream.read(extra)) : entry.value();

						buffer.copyDistance(distance, length);
					}
						
						break;
						
					default:
						mem::scratch.restore();
						return -1;
						
					}
				}

				void* const pResolved = resolveImage(&target, buffer.pResolve, buffer.pCurrent);
				buffer.pResolve = reinterpret_cast<uint8_t*>(pResolved);
			}

				break;
			
			case 2: {
				const uint16_t HLIT = stream.read(5) + 257u;
				const uint16_t HDIST = stream.read(5) + 1u;
				const uint32_t HCLEN = stream.read(4) + 4u;

				memset(lengths, 0, 19u);

				const uint8_t* const orderEnd = DYNAMIC_HUFFMAN_LENGTH_ORDER + HCLEN;
				for (const uint8_t* order{ DYNAMIC_HUFFMAN_LENGTH_ORDER }; order != orderEnd; ++order)
					lengths[*order] = static_cast<uint8_t>(stream.read(3));

				createHuffmanLookupTable<7, createEntryTypeCLEN>(literalLengthTable, lengths, 19u);

				uint8_t* pLength = lengths;

				uint8_t* const LIT_base = pLength;
				uint8_t* const DIST_base = pLength + HLIT;

				const uint8_t* const pLengthsEnd = DIST_base + HDIST;

				while (pLength < pLengthsEnd) {
					HflEntry entry = literalLengthTable[stream.peek(7)];
					stream.consume(entry.bits());

					const HflEntryType type = entry.type();

					switch (type) {
					case HFL_ENTRY_TYPE_LITERAL_BIT:
						*pLength++ = static_cast<uint8_t>(entry.value());

						break;

					case HFL_ENTRY_TYPE_REPEAT_BIT: {
						uint32_t repeat = static_cast<uint32_t>(stream.read(entry.extraBits())) + entry.value();
						const uint8_t value = *(pLength - 1u);

						while (repeat--)
							*pLength++ = value;
					}
						
						break;

					case HFL_ENTRY_TYPE_ZERO_INITIALIZE_BIT: {
						uint32_t count = static_cast<uint32_t>(stream.read(entry.extraBits())) + entry.value();

						while (count--)
							*pLength++ = 0;
					}
						break;

					default:
						mem::scratch.restore();
						return -1;
					}
				}

				createHuffmanLookupTable<9, createEntryTypeLIT>(literalLengthTable, LIT_base, HLIT);
				createHuffmanLookupTable<6, createEntryTypeDIST>(distanceTable, DIST_base, HDIST);

				while (true) {
					HflEntry entry = literalLengthTable[stream.peek(9)];
					stream.consume(entry.bits());

					HflEntryType type = entry.type();

					if (type == HFL_ENTRY_TYPE_SUBTABLE_BIT) {
						entry = literalLengthTable[entry.value() + stream.peek(entry.extraBits())];
						stream.consume(entry.bits());

						type = entry.type();
					}

					if (type == HFL_ENTRY_TYPE_BLOCK_END_BIT)
						break;

					switch (type) {
					case HFL_ENTRY_TYPE_LITERAL_BIT:
						buffer.push_Literal(static_cast<uint8_t>(entry.value()));

						break;

					case HFL_ENTRY_TYPE_LENGTH_BIT: {
						uint32_t extra = entry.extraBits();
						const uint32_t length = extra ? entry.value() + uint32_t(stream.read(extra)) : entry.value();

						entry = distanceTable[stream.peek(6)];
						stream.consume(entry.bits());

						if (entry.type() == HFL_ENTRY_TYPE_SUBTABLE_BIT) {
							entry = distanceTable[entry.value() + stream.peek(entry.extraBits())];
							stream.consume(entry.bits());
						}

						extra = entry.extraBits();
						const uint32_t distance = extra ? entry.value() + uint32_t(stream.read(extra)) : entry.value();

						buffer.copyDistance(distance, length);
					}
												  
						break;

					default:
						mem::scratch.restore();
						return -1;

					}
				}

				void* const pResolved = resolveImage(&target, buffer.pResolve, buffer.pCurrent);
				buffer.pResolve = reinterpret_cast<uint8_t*>(pResolved);
			}
				
				break;

			default:
				mem::scratch.restore();
				return -1;
				
			}

		} while (!BFINAL);

		return 0;
	}

}