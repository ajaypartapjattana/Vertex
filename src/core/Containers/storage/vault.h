#pragma once

#include <cstdint>
#include <iterator>
#include <new>
#include <type_traits>
#include <vector>

#include "Signboard/Core/Math/bitops.h"

namespace sgb {

	template <typename Storage, typename T, typename... Masks> class storage_readIterator;

	template<typename T> class vltQuery;
	template<typename T> class vltView;
	template<typename T> class vltWrite;

	template <typename T>
	class vault {
	public:
		vault(uint32_t capacity = 0) noexcept {
			slots.reserve(capacity);
			aliveBits.reserve((capacity + 63) / 64);
		}

		vault(const vault&) = delete;
		vault& operator=(const vault&) = delete;
		
		vault(vault&&) noexcept = default;
		vault& operator=(vault&&) noexcept = default;

		constexpr _NODISCARD const T* operator[](uint32_t index) const noexcept {
			if (!_is_alive(index)) return nullptr;
			return slots[index].object_ptr();
		}

		~vault() noexcept(std::is_nothrow_destructible_v<T>) {
			uint32_t size = static_cast<uint32_t>(aliveBits.size());
			uint32_t _slotC = static_cast<uint32_t>(slots.size());
			for (uint32_t w = 0; w < size; ++w) {
				uint64_t _wd = aliveBits[w];

				while (_wd) {
					uint32_t bit = bitops::ctz(_wd);
					uint32_t idx = w * 64 + bit;

					if (idx < _slotC) {
						slots[idx].object_ptr()->~T();
					}

					_wd &= _wd - 1;
				}
			}
			slots.clear();
			aliveBits.clear();
		}

	private:
		struct Storage {
			std::aligned_storage_t<sizeof(T), alignof(T)> storage;

			T* object_ptr() {
				return std::launder(reinterpret_cast<T*>(&storage));
			}

			const T* object_ptr() const {
				return std::launder(reinterpret_cast<const T*>(&storage));
			}
		};

		std::vector<Storage> slots;
		std::vector<uint64_t> aliveBits;
		uint32_t nextFreeHint = 0;

		_NODISCARD uint32_t _alloc() {
			uint32_t _wdC = static_cast<uint32_t>(aliveBits.size());

			for (uint32_t w = nextFreeHint; w < _wdC; ++w) {
				uint64_t _wd = aliveBits[w];

				if (~_wd) {
					uint64_t _freeMk = ~_wd;
					uint32_t bit = bitops::ctz(_freeMk);

					uint32_t idx = w * 64 + bit;

					if (idx >= slots.size())
						slots.resize(idx + 1);

					nextFreeHint = w;
					return idx;
				}
			}

			for (uint32_t w = 0; w < nextFreeHint; ++w) {
				uint64_t _wd = aliveBits[w];

				if (~_wd) {
					uint64_t _freeMk = ~_wd;
					uint32_t bit = bitops::ctz(_freeMk);

					uint32_t idx = w * 64 + bit;
					
					if (idx >= slots.size())
						slots.resize(idx + 1);

					nextFreeHint = w;
					return idx;
				}
			}

			uint32_t idx = static_cast<uint32_t>(slots.size());
			slots.emplace_back();
			_ensure_cap(idx);
			nextFreeHint = idx / 64;

			return idx;
		}

		bool _is_alive(uint32_t index) const noexcept {
			uint32_t _wdi = index / 64;
			if (_wdi >= aliveBits.size()) return false;
			return (aliveBits[_wdi] >> (index % 64)) & 1ull;
		}

		void _ensure_cap(uint32_t index) {
			uint32_t _req = index / 64 + 1;
			if (aliveBits.size() < _req)
				aliveBits.resize(_req, 0);
		}

		void _set_alive(uint32_t index) noexcept {
			_ensure_cap(index);
			aliveBits[index / 64] |= (1ull << (index % 64));
		}

		void _set_dead(uint32_t index) {
			uint32_t _wdi = index / 64;
			if (_wdi < aliveBits.size())
				aliveBits[_wdi] &= ~(1ull << (index % 64));
		}

		template <typename, typename, typename...>
		friend class storage_readIterator;

		template <typename>	friend class vltWrite;
		template <typename> friend class vltView;
		template <typename, typename...> friend class vltView_const;

	};

	class bitmask {
	public:
		bitmask() noexcept = default;

		bool test(uint32_t index) const noexcept {
			uint32_t _wd = index >> 6;
			if (_wd >= _mask.size()) return false;
			return (_mask[_wd] >> (index & 63)) & 1ull;
		}

		void set(uint32_t index) {
			uint32_t _wd = index >> 6;
			_ensure(_wd);
			_mask[_wd] |= (1ull << (index & 63));
		}

		void clear(uint32_t index) noexcept {
			uint32_t _wd = index >> 6;
			if (_wd < _mask.size()) {
				_mask[_wd] &= ~(1ull << (index & 63));
			}
		}

		void reset() noexcept {
			_mask.clear();
		}

		void resize(uint32_t bitCount) {
			uint32_t _wdC = (bitCount + 63) >> 6;
			_mask.resize(_wdC, 0);
		}

		_NODISCARD uint32_t word_count() const noexcept {
			return static_cast<uint32_t>(_mask.size());
		}

		_NODISCARD const uint64_t* data() const noexcept {
			return _mask.data();
		}

	private:
		void _ensure(uint32_t _wdI) {
			if (_wdI >= _mask.size())
				_mask.resize(_wdI + 1, 0);
		}

	private:
		std::vector<uint64_t> _mask;

		template <typename, typename, typename...>
		friend class storage_readIterator;

	};

	
	template <typename Storage, typename T, typename... Masks>
	class storage_readIterator {
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = T;
		using difference_type = std::ptrdiff_t;
		using pointer = const T*;
		using reference = const T&;

		storage_readIterator(const Storage& storage, uint32_t wordIndex, const std::vector<const bitmask*>* masks = nullptr)
			:
			m_storage(&storage),
			m_wordIndex(wordIndex),
			m_masks(masks)
		{
			
		}
		
		storage_readIterator(const Storage& storage, uint32_t wordIndex, const std::tuple<const Masks*...>& masks)
			:
			m_storage(&storage),
			m_wordIndex(wordIndex),
			m_staticMasks(masks)
		{
			
		}

		void _init() {
			if (m_wordIndex < m_storage->aliveBits.size()) {
				m_currentWord = _compute_wd(m_wordIndex);
				m_baseIndex = m_wordIndex * 64;
				_advance_to_next();
			}
		}

		reference operator*() const {
			return *m_storage->slots[m_currentIndex].object_ptr();
		}

		pointer operator->() const {
			return m_storage->slots[m_currentIndex].object_ptr();
		}

		storage_readIterator& operator++() {
			_advance_to_next();
			return *this;
		}

		storage_readIterator operator++(int) {
			auto tmp = *this;
			++(*this);
			return tmp;
		}

		bool operator==(const storage_readIterator& other) const {
			return m_storage == other.m_storage &&
				m_currentIndex == other.m_currentIndex &&
				m_wordIndex == other.m_wordIndex;
		}

		bool operator!=(const storage_readIterator& other) const {
			return !(*this == other);
		}

	private:
		__forceinline bool _next_non_empty_word(const Storage* storage, uint32_t wordCount) {
			while (m_currentWord == 0) {
				++m_wordIndex;

				if (m_wordIndex >= wordCount) {
					m_currentIndex = static_cast<uint32_t>(-1);
					return false;
				}

				m_currentWord = _compute_wd(m_wordIndex);
				m_baseIndex = m_wordIndex * 64;
			}
			return true;
		}

		__forceinline void _advance_to_next() {
			const auto* storage = m_storage;
			const uint32_t _slC = static_cast<uint32_t>(storage->slots.size());
			const uint32_t _wdC = static_cast<uint32_t>(storage->aliveBits.size());

			if (!_next_non_empty_word(storage, _wdC))
				return;

			uint64_t word = m_currentWord;
			uint32_t bit = bitops::ctz(word);
			m_currentIndex = m_baseIndex + bit;

			constexpr uint32_t PREFETCH_DISTANCE = 4;

			if (m_currentIndex + PREFETCH_DISTANCE < _slC) {
				_mm_prefetch(reinterpret_cast<const char*>(&storage->slots[m_currentIndex + PREFETCH_DISTANCE]), _MM_HINT_T0);
			}

			m_currentWord &= m_currentWord - 1;
		}

		__forceinline uint64_t _compute_wd(uint32_t w) const {
			if constexpr (sizeof...(Masks) > 0) {
				return _compute_wd_static(w, std::index_sequence_for<Masks...>{});
			} else if (m_masks) {
				uint64_t _wd = m_storage->aliveBits[w];

				for (const bitmask* m : *m_masks) {
					if (w < m->_mask.size())
						_wd &= m->_mask[w];
					else
						return 0ull;
				}
				return _wd;
			} else {
				return m_storage->aliveBits[w];
			}
		}

		template <size_t... I>
		__forceinline uint64_t _compute_wd_static(uint32_t w, std::index_sequence<I...>) const {
			uint64_t _wd = m_storage->aliveBits[w];

			((_wd &= (w < std::get<I>(m_staticMasks)->_mask.size() 
				? std::get<I>(m_staticMasks)->_mask[w]	
				: 0ull)), ...);
			return _wd;
		}

	private:
		const Storage* m_storage = nullptr;
		const std::vector<const bitmask*>* m_masks = nullptr;
		std::tuple<const Masks*...> m_staticMasks;

		uint32_t m_wordIndex = 0;
		uint64_t m_currentWord = 0;
		uint32_t m_baseIndex = 0;

		uint32_t m_currentIndex = static_cast<uint32_t>(-1);

	};

	template <typename T>
	class vltQuery {
	public:
		vltQuery() noexcept = default;

		vltQuery(const vault<T>& vault) noexcept
			:
			m_vault(&vault)
		{

		}

		vltQuery(const vltQuery& other) noexcept = default;
		vltQuery& operator=(const vltQuery& other) noexcept = default;

		vltQuery(vltQuery&& other) noexcept = default;
		vltQuery& operator=(vltQuery&& other) noexcept = default;

		vltQuery& with(const bitmask& mask) {
			m_masks.push_back(&mask);
			return *this;
		}

		_NODISCARD const T* get(uint32_t index) const noexcept {
			if (!m_vault->_is_alive(index)) return nullptr;
			return m_vault->slots[index].object_ptr();
		}

		inline auto begin() const {
			auto it = storage_readIterator<vault<T>, T>(*m_vault, 0, m_masks.empty() ? nullptr : &m_masks);
			it._init();
			return it;
		}

		inline auto end() const {
			return storage_readIterator<vault<T>, T>(*m_vault, static_cast<uint32_t>(m_vault->aliveBits.size()), m_masks.empty() ? nullptr : &m_masks);
		}

	private:
		const vault<T>* m_vault;
		std::vector<const bitmask*> m_masks;

	};

	template <typename T>
	class vltView {
	public:
		vltView() noexcept = default;

		vltView(const vault<T>& vault) noexcept
			:
			m_vault(&vault)
		{

		}

		vltView(const vltView&) noexcept = default;
		vltView& operator=(const vltView&) noexcept = default;

		vltView(vltView&&) noexcept = default;
		vltView& operator=(vltView&&) noexcept = default;

		_NODISCARD const T* operator[](uint32_t index) noexcept {
			if (!m_vault) return nullptr;
			if (!m_vault->_is_alive(index)) return nullptr;

			return m_vault->slots[index].object_ptr();
		}

		_NODISCARD const T* get(uint32_t index) const noexcept {
			if (!m_vault) return nullptr;
			if (!m_vault->_is_alive(index)) return nullptr;

			return m_vault->slots[index].object_ptr();
		}

		inline auto begin() const {
			auto it = storage_readIterator<vault<T>, T>(*m_vault, 0);
			it._init();
			return it;
		}

		inline auto end() const {
			return storage_readIterator<vault<T>, T>(*m_vault, static_cast<uint32_t>(m_vault->aliveBits.size()));
		}

	private:
		const vault<T>* m_vault = nullptr;

	};

	template <typename T>
	class vltWrite {
	public:
		vltWrite(vault<T>& vault)
			:
			m_vault(vault)
		{

		}

		vltWrite(const vltWrite& other) noexcept
			:
			m_vault(other.m_vault)
		{

		}

		vltWrite& operator=(const vltWrite&) = delete;

		vltWrite(vltWrite&& other) noexcept
			:
			m_vault(other.m_vault)
		{

		}

		vltWrite& operator=(vltWrite&&) = delete;

		void reserve_slots(uint32_t count) {
			if (count == 0) return;

			uint32_t _start = static_cast<uint32_t>(m_vault.slots.size());

			m_vault.slots.resize(_start + count);
			m_vault._ensure_cap(_start + count - 1);
		}

		template<typename F>
		_NODISCARD uint32_t construct(F&& builder) {
			uint32_t idx = m_vault._alloc();
			T* obj = m_vault.slots[idx].object_ptr();

			bool constructed = false;
			try {
				new (obj) T();
				constructed = true;

				builder(obj);
				m_vault._set_alive(idx);
			}
			catch (...) {
				if (constructed)
					obj->~T();

				m_vault._set_dead(idx);
				throw;
			}

			return idx;
		}

		template <typename F>
		std::vector<uint32_t> construct_many(uint32_t count, F&& builder) {
			std::vector<uint32_t> indices;
			indices.reserve(count);

			uint32_t constructedCount = 0;

			try {
				for (uint32_t i = 0; i < count; ++i) {
					uint32_t idx = m_vault._alloc();
					indices.push_back(idx);

					T* obj = m_vault.slots[idx].object_ptr();
					
					new (obj) T();
					++constructedCount;

					builder(i, obj);
					
					m_vault._set_alive(idx);
				}
			}
			catch (...) {
				for (uint32_t i = 0; i < constructedCount; ++i) {
					uint32_t idx = indices[i];

					T* obj = m_vault.slots[idx].object_ptr();
					obj->~T();
					
					m_vault._set_dead(idx);
				}
				throw;
			}

			return indices;
		}

		template <typename... Args>
		_NODISCARD uint32_t create(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args&&...>) {
			uint32_t index = m_vault._alloc();

			T* obj = m_vault.slots[index].object_ptr();

			new (obj) T(std::forward<Args>(args)...);
			m_vault._set_alive(index);

			return index;
		}

		void destroy(uint32_t index) noexcept(std::is_nothrow_destructible_v<T>) {
			if (!m_vault._is_alive(index))
				return;

			m_vault.slots[index].object_ptr()->~T();
			m_vault._set_dead(index);
		}

		void clear() {
			uint32_t _sz = static_cast<uint32_t>(m_vault.aliveBits.size());
			uint32_t _slC = static_cast<uint32_t>(m_vault.slots.size());

			for (uint32_t w = 0; w < _sz; ++w) {
				uint64_t _wd = m_vault.aliveBits[w];

				while (_wd) {
					uint32_t bit = bitops::ctz(_wd);
					uint32_t idx = w * 64 + bit;

					if (idx < _slC) {
						m_vault.slots[idx].object_ptr()->~T();
					}

					_wd &= _wd - 1;
				}

			}

			m_vault.slots.clear();
			m_vault.aliveBits.clear();
			m_vault.nextFreeHint = 0;
		}

		_NODISCARD T* get(uint32_t index) const noexcept {
			if (!m_vault._is_alive(index)) return nullptr;
			return m_vault.slots[index].object_ptr();
		}

	private:
		vault<T>& m_vault;

	};

}
