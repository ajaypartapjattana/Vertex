#pragma once

#include <vector>
#include <utility>
#include <memory>

template <typename Traits>
class allocation_pool {
private:
	using _Ty = typename Traits::handle_type;
	using _resultTy = typename Traits::result_type;
	using _parentTy = typename Traits::parent_type;
	using _allocatorTy = typename Traits::allocator_type;
	using _allocTy = typename Traits::allocationInfo_type;

	std::vector<_Ty> m_allocations;
	size_t mark = 0;
	size_t currentSize = 0;

	_parentTy r_root = Traits::null();

public:
	allocation_pool() noexcept = default;
	allocation_pool(const allocation_pool&) = delete;
	allocation_pool(allocation_pool&& other) noexcept
		:
		m_allocations(std::move(other.m_allocations)),
		mark(other.mark),
		currentSize(other.currentSize),
		r_root(std::exchange(other.r_root, Traits::null()))
	{

	}

	allocation_pool& operator=(const allocation_pool&) = delete;
	allocation_pool& operator=(allocation_pool&& other) noexcept {
		if (this == &other)
			return *this;

		m_allocations = std::move(other.m_allocations);
		mark = other.mark;
		currentSize = other.currentSize;
		r_root = std::exchange(other.r_root, Traits::null());

		return *this;
	}

	~allocation_pool() = default;

	void root(_parentTy root) noexcept {
		if (root == r_root)
			return;

		reset();
		r_root = root;
	}

	_Ty grant() {
		if (mark >= currentSize)
			throw std::logic_error("IMPERMISSIBLE : pool_exhausted!");

		return m_allocations[mark++];
	}

	void recycle(_Ty resource) {
		if (!mark)
			throw std::runtime_error("IMPERMISSIBLE : pool_domain_contaminated!");

		m_allocations[--mark] = resource;
	}

	_resultTy allocate(const _allocTy* pAllocInfo, size_t size) noexcept {
		_resultTy result;
		
		m_allocations.resize(size);

		result = Traits::allocate(r_root, pAllocInfo, &m_allocations[currentSize]);
		if (result != Traits::success()) {
			m_allocations.resize(currentSize);
			return result;
		}

		currentSize = size;

		return Traits::success();
	}

	void reset() noexcept {
		m_allocations.clear();
		mark = 0;
		currentSize = 0;
	}

};