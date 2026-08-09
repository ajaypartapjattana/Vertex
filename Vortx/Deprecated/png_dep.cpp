template<unsigned SymbolBits>
class HEntry {
private:
	static_assert(SymbolBits > 0 && SymbolBits < 16);

	static constexpr unsigned LenBits = 16 - SymbolBits;
	static constexpr uint16_t ValueMask = (1u << SymbolBits) - 1;
	static constexpr uint16_t BitsMask = (1u << LenBits) - 1;

	uint16_t data;

public:
	constexpr HEntry() = default;
	constexpr HEntry(uint16_t entry) noexcept
		:
		data(entry)
	{

	}
	constexpr HEntry(uint16_t value, uint16_t bits) noexcept
		:
		data(((value& ValueMask) << LenBits) | (bits & BitsMask))
	{

	}

	inline uint16_t value() const noexcept {
		return data >> LenBits;
	}

	inline uint16_t bits() const noexcept {
		return data & BitsMask;
	}
};

template <unsigned short RootBits, unsigned MaxBits = 15>
constexpr uint32_t generateHTable(HEntry<RootBits + 1>* pTable, const uint8_t* pCodeLengths, const uint32_t symbolCount) {
	using ENTRY = HEntry<RootBits + 1>;

	std::fill_n(pTable, 1ull << RootBits, ENTRY{});

	std::vector<uint16_t> symbolCodes(symbolCount);

	{
		std::array<uint16_t, MaxBits + 1> blCount{};

		for (uint32_t symbol = 0; symbol < symbolCount; ++symbol) {
			uint8_t len = pCodeLengths[symbol];

			if (len)
				++blCount[len];
		}

		std::array<uint16_t, MaxBits + 1> nextCode{};

		uint16_t code = 0;

		for (size_t bits = 1; bits <= MaxBits; ++bits) {
			code = (code + blCount[bits - 1]) << 1;
			nextCode[bits] = code;
		}

		for (uint16_t symbol = 0; symbol < symbolCount; ++symbol) {
			uint8_t len = pCodeLengths[symbol];

			if (!len)
				continue;

			uint16_t code = nextCode[len]++;

			uint16_t rCode = 0;

			for (uint8_t i = 0; i < len; ++i) {
				rCode <<= 1;
				rCode |= code & 1;
				code >>= 1;
			}

			symbolCodes[symbol] = rCode;
		}
	}

	uint16_t nextFree = 1u << RootBits;

	{
		std::array<uint8_t, 1u << RootBits> maxSuffix{};

		for (uint16_t symbol = 0; symbol < symbolCount; ++symbol) {
			uint32_t len = pCodeLengths[symbol];

			if (len <= RootBits)
				continue;

			uint16_t root = symbolCodes[symbol] & ((1u << RootBits) - 1);

			uint8_t suffix = len - RootBits;

			maxSuffix[root] = maxSuffix[root] > suffix ? maxSuffix[root] : suffix;
		}

		for (uint32_t root = 0; root < (1u << RootBits); ++root) {
			if (!maxSuffix[root])
				continue;

			pTable[root] = ENTRY{ nextFree, static_cast<uint16_t>(RootBits + maxSuffix[root]) };

			nextFree += 1u << maxSuffix[root];
		}
	}

	for (uint16_t symbol = 0; symbol < symbolCount; ++symbol) {
		uint8_t len = pCodeLengths[symbol];

		if (!len)
			continue;

		uint16_t rCode = symbolCodes[symbol];

		if (len <= RootBits) {
			ENTRY entry{ symbol, len };

			uint32_t fillCount = 1u << (RootBits - len);

			for (uint32_t i = 0; i < fillCount; ++i) {
				pTable[(i << len) | rCode] = entry;
			}

			continue;
		}

		uint32_t root = rCode & ((1u << RootBits) - 1);

		uint32_t tail = rCode >> RootBits;
		uint32_t tailLen = len - RootBits;

		ENTRY entry(symbol, len);

		uint32_t fillCount = 1u << (pTable[root].bits() - len);

		uint32_t base = pTable[root].value() + tail;
		for (uint32_t i = 0; i < fillCount; ++i) {
			pTable[base + (i << tailLen)] = entry;
		}
	}

	return nextFree;
}



constexpr static uint32_t signature_png_chunk_type(const char(&s)[5]) {
	return (uint32_t(s[0]) << 24) | (uint32_t(s[1]) << 16) | (uint32_t(s[2]) << 8) | uint32_t(s[3]);
}

enum PNGChunkSignature : uint32_t {
	PNG_CHUNK_SIGNATURE_IHDR = signature_png_chunk_type("IHDR"),
	PNG_CHUNK_SIGNATURE_cHRM = signature_png_chunk_type("cHRM"),
	PNG_CHUNK_SIGNATURE_gAMA = signature_png_chunk_type("gAMA"),
	PNG_CHUNK_SIGNATURE_iCCP = signature_png_chunk_type("iCCP"),
	PNG_CHUNK_SIGNATURE_sBIT = signature_png_chunk_type("sBIT"),
	PNG_CHUNK_SIGNATURE_sRGB = signature_png_chunk_type("sRGB"),
	PNG_CHUNK_SIGNATURE_PLTE = signature_png_chunk_type("PLTE"),
	PNG_CHUNK_SIGNATURE_bKGD = signature_png_chunk_type("bKGD"),
	PNG_CHUNK_SIGNATURE_hIST = signature_png_chunk_type("hIST"),
	PNG_CHUNK_SIGNATURE_tRNS = signature_png_chunk_type("tRNS"),
	PNG_CHUNK_SIGNATURE_pHYs = signature_png_chunk_type("pHYs"),
	PNG_CHUNK_SIGNATURE_sPLT = signature_png_chunk_type("sPLT"),
	PNG_CHUNK_SIGNATURE_tIME = signature_png_chunk_type("tIME"),
	PNG_CHUNK_SIGNATURE_iTXt = signature_png_chunk_type("iTXt"),
	PNG_CHUNK_SIGNATURE_tEXt = signature_png_chunk_type("tEXt"),
	PNG_CHUNK_SIGNATURE_zTXt = signature_png_chunk_type("zTXt"),
	PNG_CHUNK_SIGNATURE_IDAT = signature_png_chunk_type("IDAT"),
	PNG_CHUNK_SIGNATURE_IEND = signature_png_chunk_type("IEND")
};

enum PNGChunkFlagBits : uint32_t {
	PNG_CHUNK_IHDR_BIT = 0x00000001,
	PNG_CHUNK_cHRM_BIT = 0x00000002,
	PNG_CHUNK_gAMA_BIT = 0x00000004,
	PNG_CHUNK_iCCP_BIT = 0x00000008,
	PNG_CHUNK_sBIT_BIT = 0x00000010,
	PNG_CHUNK_sRGB_BIT = 0x00000020,
	PNG_CHUNK_PLTE_BIT = 0x00000040,
	PNG_CHUNK_bKGD_BIT = 0x00000080,
	PNG_CHUNK_hIST_BIT = 0x00000100,
	PNG_CHUNK_tRNS_BIT = 0x00000200,
	PNG_CHUNK_pHYs_BIT = 0x00000400,
	PNG_CHUNK_sPLT_BIT = 0x00000800,
	PNG_CHUNK_tIME_BIT = 0x00001000,
	PNG_CHUNK_iTXt_BIT = 0x00002000,
	PNG_CHUNK_tEXt_BIT = 0x00004000,
	PNG_CHUNK_zTXt_BIT = 0x00008000,
	PNG_CHUNK_IDAT_BIT = 0x00010000,
	PNG_CHUNK_IEND_BIT = 0x00020000
};

Result ImageFile::loadPNGw(const wchar_t* _Path) {
	std::vector<uint8_t> dataA;

	{
		BOOL result;

		HANDLE file = CreateFileW(_Path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (file == INVALID_HANDLE_VALUE)
			return FAILURE;

		std::array<uint8_t, 8> fileSignature;

		DWORD bytesRead;
		result = ReadFile(file, fileSignature.data(), (DWORD)8, &bytesRead, nullptr);
		if (!result || bytesRead != 8) {
			CloseHandle(file);

			return FAILURE;
		}

		constexpr std::array<uint8_t, 8> PNGSignature{
			0x89, 0x50, 0x4E, 0x47,
			0x0D, 0x0A, 0x1A, 0x0A
		};

		if (fileSignature != PNGSignature) {
			CloseHandle(file);

			return FAILURE;
		}

		LARGE_INTEGER size;
		result = GetFileSizeEx(file, &size);
		if (!result) {
			CloseHandle(file);

			return FAILURE;
		}

		if (size.QuadPart < 8) {
			CloseHandle(file);

			return VALIDITY_CHECK_FAILURE;
		}

		dataA.resize(size.QuadPart - 8);

		result = ReadFile(file, dataA.data(), (DWORD)dataA.size(), &bytesRead, nullptr);
		if (!result || bytesRead != dataA.size()) {
			CloseHandle(file);

			return FAILURE;
		}

		CloseHandle(file);
	}

	uint32_t _width;
	uint32_t _height;
	uint8_t _bitDepth;

	uint8_t colorType;

	std::vector<uint8_t> dataB;

	{
		uint32_t chunkLength;
		uint32_t chunkSignature;
		const uint8_t* chunkData;
		uint32_t chunkCRC;

		const uint8_t* cursor = dataA.data();
		const uint8_t* end = dataA.data() + dataA.size();

		dataB.reserve(dataA.size());

		uint32_t gatheredChunkTypes = 0;

		uint32_t currentChunkType = 0;
		uint32_t previousChunkType = 0;

		auto read_U32_BE = [](const uint8_t* ptr) -> uint32_t {
			return (uint32_t(ptr[0]) << 24 | uint32_t(ptr[1]) << 16 | uint32_t(ptr[2]) << 8 | uint32_t(ptr[3]));
			};

		do {
			chunkLength = read_U32_BE(cursor);
			cursor += 4;

			chunkSignature = read_U32_BE(cursor);
			cursor += 4;

			chunkData = cursor;
			cursor += chunkLength;

			if (cursor + 4 > end)
				return VALIDITY_CHECK_FAILURE;

			chunkCRC = read_U32_BE(cursor);
			cursor += 4;

			switch (chunkSignature) {
			case PNG_CHUNK_SIGNATURE_IHDR:
				currentChunkType = PNG_CHUNK_IHDR_BIT;

				if (gatheredChunkTypes & PNG_CHUNK_IHDR_BIT)
					return VALIDITY_CHECK_FAILURE;

				_width = read_U32_BE(chunkData);
				_height = read_U32_BE(chunkData + 4);

				if (chunkData[12] != 0)
					return UNSUPPORTED_FEATURE;

				if (chunkData[10] != 0 || chunkData[11] != 0)
					return VALIDITY_CHECK_FAILURE;

				_bitDepth = chunkData[8];

				colorType = chunkData[9];

				break;

			case PNG_CHUNK_SIGNATURE_IDAT:
				currentChunkType = PNG_CHUNK_IDAT_BIT;

				if (previousChunkType != PNG_CHUNK_IDAT_BIT && gatheredChunkTypes & PNG_CHUNK_IDAT_BIT)
					return VALIDITY_CHECK_FAILURE;

				dataB.insert(dataB.end(), chunkData, chunkData + chunkLength);

				break;

			case PNG_CHUNK_SIGNATURE_PLTE:
				currentChunkType = PNG_CHUNK_PLTE_BIT;

				if (previousChunkType != PNG_CHUNK_PLTE_BIT && gatheredChunkTypes & PNG_CHUNK_PLTE_BIT)
					return VALIDITY_CHECK_FAILURE;

				break;

			case PNG_CHUNK_SIGNATURE_IEND:
				currentChunkType = PNG_CHUNK_IEND_BIT;

				break;

			default:
				break;
			}

			gatheredChunkTypes |= currentChunkType;
			previousChunkType = currentChunkType;

		} while (~gatheredChunkTypes & PNG_CHUNK_IEND_BIT);
	}

	dataA.clear();

	{
		BitStream stream{ dataB.data(), dataB.size() };

		size_t windowSize;

		{
			if (stream.endsBefore(2))
				return VALIDITY_CHECK_FAILURE;

			uint8_t CMF = stream.read_Byte();
			uint8_t FLG = stream.read_Byte();

			if ((CMF & 0x0F) != 8)
				return UNSUPPORTED_FEATURE;

			windowSize = size_t(1) << ((CMF >> 4) + 8);

			if (((uint16_t(CMF) << 8) | FLG) % 31)
				return VALIDITY_CHECK_FAILURE;

			if (FLG & 0x20)
				return UNSUPPORTED_FEATURE;

		}

		dataA.reserve(dataB.size() << 2);

		uint16_t BFINAL;

		std::vector<HEntry<8>> codeLength_HTable;

		std::vector<uint8_t> HLengths;

		std::vector<HEntry<10>> LITTable;
		std::vector<HEntry<7>> DISTTable;

		do {
			BFINAL = (uint16_t)stream.read(1);

			uint16_t BTYPE = (uint16_t)stream.read(2);

			switch (BTYPE) {
			case 0: {
				uint16_t BLEN = stream.read_LE16();

				if (~BLEN != stream.read_LE16())
					return VALIDITY_CHECK_FAILURE;

				const uint8_t* begin = stream.readBytes(BLEN);

				dataA.insert(dataA.end(), begin, begin + BLEN);
			}

				  break;

			case 1: {
				HLengths.resize(320);

				uint32_t i = 0;

				do {
					HLengths[i++] = 8;
				} while (i < 144);

				do {
					HLengths[i++] = 9;
				} while (i < 256);

				do {
					HLengths[i++] = 7;
				} while (i < 280);

				do {
					HLengths[i++] = 8;
				} while (i < 288);

				do {
					HLengths[i++] = 5;
				} while (i < 320);

				LITTable.resize(512);

				uint32_t litSize = generateHTable<9>(LITTable.data(), HLengths.data(), 288);
				LITTable.resize(litSize);

				DISTTable.resize(128);

				uint32_t distSize = generateHTable<6>(DISTTable.data(), HLengths.data() + 288, 32);
				DISTTable.resize(distSize);

				do {
					HEntry entry = LITTable[stream.peek(9)];

					stream.consume(entry.bits());

					uint16_t symbol = entry.value();


					if (symbol < 256) {
						dataA.push_back(uint8_t(symbol));

						continue;
					}

					if (symbol == 256)
						break;

					uint32_t length;

					{
						uint32_t index = symbol - 257;

						length = FIXED_HUFFMAN_LENGTH_BASE[index];
						uint32_t extra = FIXED_HUFFMAN_LENGTH_EXTRA[index];

						if (extra)
							length += (uint32_t)stream.read(extra);
					}

					HEntry disEntry = DISTTable[stream.read(6)];

					symbol = disEntry.value();

					uint32_t distance;

					{
						distance = FIXED_HUFFMAN_DISTANCE_BASE[symbol];
						uint32_t extra = FIXED_HUFFMAN_DISTANCE_EXTRA[symbol];

						if (extra)
							distance += (uint32_t)stream.read(extra);
					}

					size_t start = dataA.size() - distance;

					for (uint32_t i = 0; i < length; ++i)
						dataA.push_back(dataA[start + i]);

				} while (true);
			}

				  break;

			case 2: {

				uint16_t HLIT = (uint16_t)stream.read(5) + 257;
				uint16_t HDIST = (uint16_t)stream.read(5) + 1;
				uint32_t HCLEN = (uint32_t)stream.read(4) + 4;

				{

					std::array<uint8_t, 19> codeLengths{};

					for (uint32_t i = 0; i < HCLEN; ++i) {
						codeLengths[DYNAMIC_HUFFMAN_LENGTH_ORDER[i]] = (uint8_t)stream.read(3);
					}

					codeLength_HTable.resize(128);

					uint32_t entryCount = generateHTable<7>(codeLength_HTable.data(), codeLengths.data(), 19);
					codeLength_HTable.resize(entryCount);

					size_t HlengthCount = size_t(HLIT) + HDIST;
					HLengths.resize(HlengthCount);

					size_t writeIndex = 0;

					uint8_t pushVal;
					uint32_t pushCount;

					do {
						const HEntry entry = codeLength_HTable[stream.peek(7)];

						uint16_t len = entry.bits();
						if (len > 7)
							return VALIDITY_CHECK_FAILURE;

						uint16_t symbol = entry.value();

						stream.consume(len);

						switch (symbol) {
						case 16:
							if (!writeIndex)
								return VALIDITY_CHECK_FAILURE;

							pushVal = HLengths[writeIndex - 1];
							pushCount = (uint32_t)stream.read(2) + 3;

							break;

						case 17:
							pushVal = 0;
							pushCount = (uint32_t)stream.read(3) + 3;

							break;

						case 18:
							pushVal = 0;
							pushCount = (uint32_t)stream.read(7) + 11;

							break;

						default:
							if (symbol > 18)
								return VALIDITY_CHECK_FAILURE;

							pushVal = (uint8_t)symbol;
							pushCount = 1;

							break;

						}

						if (writeIndex + pushCount > HlengthCount)
							return VALIDITY_CHECK_FAILURE;

						while (pushCount--) {
							HLengths[writeIndex++] = pushVal;
						}

					} while (writeIndex < HlengthCount);
				}

				LITTable.resize(1024);

				uint32_t litSize = generateHTable<9>(LITTable.data(), HLengths.data(), HLIT);
				LITTable.resize(litSize);

				DISTTable.resize(128);

				uint32_t distSize = generateHTable<6>(DISTTable.data(), HLengths.data() + HLIT, HDIST);
				DISTTable.resize(distSize);

				do {
					HEntry entry = LITTable[stream.peek(9)];

					uint16_t len = entry.bits();

					if (len > 9) {
						entry = LITTable[entry.value() + (stream.peek(len) >> 9)];
						len = entry.bits();
					}

					stream.consume(len);

					uint16_t symbol = entry.value();

					if (symbol < 256) {
						dataA.push_back(uint8_t(symbol));

						continue;
					}

					if (symbol == 256)
						break;

					uint32_t length;

					{
						uint32_t index = symbol - 257;

						length = FIXED_HUFFMAN_LENGTH_BASE[index];
						uint32_t extra = FIXED_HUFFMAN_LENGTH_EXTRA[index];

						if (extra)
							length += (uint32_t)stream.read(extra);
					}


					HEntry disEntry = DISTTable[stream.peek(6)];

					len = disEntry.bits();
					if (len > 6) {
						disEntry = DISTTable[disEntry.value() + (stream.peek(len) >> 6)];
						len = disEntry.bits();
					}

					stream.consume(len);

					symbol = disEntry.value();

					uint32_t distance = FIXED_HUFFMAN_DISTANCE_BASE[symbol];

					{
						uint32_t extra = FIXED_HUFFMAN_DISTANCE_EXTRA[symbol];

						if (extra)
							distance += (uint32_t)stream.read(extra);
					}

					size_t start = dataA.size() - distance;

					for (uint32_t i = 0; i < length; ++i)
						dataA.push_back(dataA[start + i]);

				} while (true);
			}

				  break;

			default:
				return VALIDITY_CHECK_FAILURE;

			}

		} while (!BFINAL);

	}

	dataB.clear();

	ImageType _type;

	{
		const uint8_t* pGet = dataA.data();
		const uint8_t* pGet_oldScanLine = nullptr;

		uint32_t channels;

		switch (colorType) {
		case 0:
			_type = IMAGE_TYPE_GRAYSCALE;
			channels = 1;

			break;

		case 2:
			_type = IMAGE_TYPE_RGB;
			channels = 3;

			break;

		case 3:
			_type = IMAGE_TYPE_INDEXED;
			channels = 1;

			break;

		case 4:
			_type = IMAGE_TYPE_GRAYSCALE_ALPHA;
			channels = 2;

			break;

		case 6:
			_type = IMAGE_TYPE_RGB_ALPHA;
			channels = 4;

			break;

		default:
			return UNSUPPORTED_FEATURE;

		}


		uint32_t rowByteCount = _width * channels;

		dataB.resize(size_t(_height) * rowByteCount);

		uint8_t* pPut = dataB.data();

		for (uint32_t i = 0; i < _height; ++i) {
			uint8_t filter = *pGet++;

			switch (filter) {
			case 0:
				std::copy_n(pGet, rowByteCount, pPut);

				break;

			case 1:
				for (uint32_t j = 0; j < rowByteCount; ++j) {
					uint8_t left = (j >= channels) ? pPut[j - channels] : 0;

					pPut[j] = pGet[j] + left;
				}

				break;

			case 2:
				for (uint32_t j = 0; j < rowByteCount; ++j) {
					uint8_t up = pGet_oldScanLine ? pGet_oldScanLine[j] : 0;

					pPut[j] = pGet[j] + up;
				}

				break;

			case 3:
				for (uint32_t j = 0; j < rowByteCount; ++j) {
					uint8_t left = (j >= channels) ? pPut[j - channels] : 0;
					uint8_t up = pGet_oldScanLine ? pGet_oldScanLine[j] : 0;

					pPut[j] = pGet[j] + (left + up) / 2;
				}

				break;

			case 4:
				for (uint32_t j = 0; j < rowByteCount; ++j) {
					uint8_t left = (j >= channels) ? pPut[j - channels] : 0;
					uint8_t up = pGet_oldScanLine ? pGet_oldScanLine[j] : 0;
					uint8_t upLeft = (pGet_oldScanLine && j >= channels) ? pGet_oldScanLine[j - channels] : 0;

					int p = left + up - upLeft;

					int pa = abs(p - left);
					int pb = abs(p - up);
					int pc = abs(p - upLeft);

					int predictor;

					if (pa <= pb && pa <= pc)
						predictor = left;
					else if (pb <= pc)
						predictor = up;
					else
						predictor = upLeft;

					pPut[j] = pGet[j] + predictor;
				}

				break;

			default:
				return VALIDITY_CHECK_FAILURE;

			}

			pGet_oldScanLine = pPut;
			pGet += rowByteCount;
			pPut += rowByteCount;
		}
	}

	width = _width;
	height = _height;
	bitDepth = _bitDepth;
	type = _type;

	bin = std::move(dataB);

	return SUCCESS;
}