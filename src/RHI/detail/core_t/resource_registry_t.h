#pragma once

#include <utility>

template <typename Traits>
class resource_registry {
private:

	using _Ty = typename Traits::handle_type;
	using _allocTy = typename Traits::allocation_type;
	using _parentTy = typename Traits::parent_type;
	using _resultTy = typename Traits::result_type;
	using _createInfoTy = typename Traits::createInfo_type;
	using _allocCreateInfoTy = typename Traits::allocationCreateInfo_type;
	using _allocInfo = typename Traits::allocationInfo_type;

	_Ty m_resource = Traits::null();
	_allocTy m_allocation = Traits::null();

	_parentTy r_root = Traits::null();

public:
	resource_registry() noexcept = default;
	explicit resource_registry(_parentTy root) noexcept
		:
		r_root(root)
	{

	}
	resource_registry(const resource_registry&) = delete;
	resource_registry(resource_registry&& other) noexcept
		:
		m_resource(std::exchange(other.m_resource, Traits::null())),
		m_allocation(std::exchange(other.m_allocation, Traits::null())),
		r_root(std::exchange(other.r_root, Traits::null()))
	{

	}

	resource_registry& operator=(const resource_registry&) = delete;
	resource_registry& operator=(resource_registry&& other) noexcept {
		if (this == &other)
			return *this;

		reset();

		m_resource = std::exchange(other.m_resource, Traits::null());
		m_allocation = std::exchange(other.m_allocation, Traits::null());
		r_root = std::exchange(other.r_root, Traits::null());

		return *this;
	}

	~resource_registry() noexcept {
		reset();
	}

	operator _Ty() const noexcept {
		return m_resource;
	}

	_allocTy allocation() const noexcept {
		return m_allocation;
	}

	explicit operator bool() const noexcept {
		return (m_resource != Traits::null() || m_allocation != Traits::null());
	}

	_resultTy create(const _createInfoTy* pCreateInfo, const _allocCreateInfoTy* pAllocationCreateInfo, _allocInfo* pAllocationInfo) noexcept {
		_Ty target = Traits::null();
		_allocTy allocation = Traits::null();
		_resultTy result = Traits::create(r_root, pCreateInfo, pAllocationCreateInfo, &target, &allocation, pAllocationInfo);
		if (result != Traits::success())
			return result;

		reset();

		m_resource = target;
		m_allocation = allocation;

		return Traits::success();
	}

	void root(_parentTy root) noexcept {
		if (r_root == root)
			return;

		reset();
		r_root = root;
	}

	void reset() noexcept {
		if (m_resource != Traits::null() || m_allocation != Traits::null())
			Traits::destroy(r_root, m_resource, m_allocation);

		m_resource = Traits::null();
		m_allocation = Traits::null();
	}

};