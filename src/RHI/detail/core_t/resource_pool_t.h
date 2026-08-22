#pragma once

#include <vector>
#include <stdexcept>
#include <utility>

template <typename Traits>
class resource_pool {
private:

	using _Ty = typename Traits::handle_type;
	using _allocTy = typename Traits::allocation_type;
	using _parentTy = typename Traits::parent_type;
	using _resultTy = typename Traits::result_type;
	using _createInfoTy = typename Traits::createInfo_type;
	using _allocCreateInfoTy = typename Traits::allocationCreateInfo_type;
	using _allocInfo = typename Traits::allocationInfo_type;

	std::vector<_Ty> m_resources;
	std::vector<_allocTy> m_allocations;

	_parentTy r_root = Traits::null();

public:
	resource_pool() noexcept = default;
	resource_pool(_parentTy root) noexcept
		:
		r_root(root)
	{

	}
	resource_pool(const resource_pool&) = delete;
	resource_pool(resource_pool&& other) noexcept
		:
		m_resources(std::move(other.m_resources)),
		m_allocations(std::move(other.m_allocations)),
		r_root(other.r_root)
	{
		other.r_root = Traits::null();
	}

	resource_pool& operator=(const resource_pool&) = delete;
	resource_pool& operator=(resource_pool&& other) noexcept {
		if (this == &other)
			return *this;

		reset();

		m_resources = std::move(other.m_resources);
		m_allocations = std::move(other.m_allocations);
		r_root = std::exchange(other.r_root, Traits::null());

		return *this;
	}

	~resource_pool() noexcept {
		reset();
	}

	void reset() noexcept {
		const size_t size = m_resources.size();

		for (size_t i = 0; i < size; ++i) {
			_Ty resource = m_resources[i];
			_allocTy allocation = m_allocations[i];

			if (resource == Traits::null() && allocation == Traits::null())
				continue;

			Traits::destroy(r_root, m_resources[i], m_allocations[i]);
		}

		m_resources.clear();
		m_allocations.clear();
	}

	_Ty operator[](size_t index) const noexcept {
		return m_resources[index];
	}

	_allocTy allocation(size_t index) const noexcept {
		return m_allocations[index];
	}

	void root(_parentTy root) noexcept {
		if (r_root == root)
			return;

		reset();
		r_root = root;
	}

	_resultTy create(const _createInfoTy* pCreateInfo, const _allocCreateInfoTy* pAllocationCreateInfo, size_t* index, _allocInfo* pAllocationInfo) noexcept {
		_Ty target = Traits::null();
		_allocTy allocation = Traits::null();
		_resultTy result = Traits::create(r_root, pCreateInfo, pAllocationCreateInfo, &target, &allocation, pAllocationInfo);
		if (result != Traits::success())
			return result;

		size_t pushIndex = m_resources.size();

		m_resources.push_back(target);
		m_allocations.push_back(allocation);
		*index = pushIndex;

		return Traits::success();
	}

	_resultTy replace(const _createInfoTy* pCreateInfo, const _allocCreateInfoTy* pAllocationCreateInfo, size_t index, _allocInfo* pAllocationInfo) noexcept {
		if (index >= m_resources.size())
			return Traits::invalid();
		
		_Ty target = Traits::null();
		_allocTy allocation = Traits::null();
		_resultTy result = Traits::create(r_root, pCreateInfo, pAllocationCreateInfo, &target, &allocation, pAllocationInfo);
		if (result != Traits::success())
			return result;

		Traits::destroy(r_root, m_resources[index], m_allocations[index]);
		
		m_resources[index] = target;
		m_allocations[index] = allocation;

		return Traits::success();
	}

};