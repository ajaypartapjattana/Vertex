#pragma once

#include <vector>
#include <memory>

template <typename Traits>
class fungible_pool {
private:

	using _Ty = typename Traits::handle_type;
	using _parentTy = typename Traits::parent_type;
	using _resultTy = typename Traits::result_type;
	using _infoTy = typename Traits::createInfo_type;

	std::vector<_Ty> m_resources;
	size_t mark = 0;
	size_t currentSize = 0;

	std::unique_ptr<_infoTy> m_ownedInfo = std::make_unique<_infoTy>();
	
	_parentTy r_root = Traits::null();

public:

	fungible_pool() noexcept = default;
	fungible_pool(_parentTy root) noexcept
		:
		r_root(root)
	{

	}

	fungible_pool(const fungible_pool&) = delete;
	fungible_pool(fungible_pool&& other) noexcept 
		:
		m_resources(std::move(other.m_resources)),
		mark(other.mark),
		currentSize(other.currentSize),
		m_ownedInfo(std::move(other.m_ownedInfo)),
		r_root(std::exchange(other.r_root, Traits::null()))
	{

	}

	fungible_pool& operator=(const fungible_pool&) = delete;
	fungible_pool& operator=(fungible_pool&& other) noexcept {
		if (this == &other)
			return *this;

		m_resources = std::move(other.m_resources);
		mark = other.mark;
		currentSize = other.currentSize;
		m_ownedInfo = std::move(other.m_ownedInfo);
		r_root = std::exchange(other.r_root, Traits::null());

		return *this;
	}

	~fungible_pool() noexcept {
		for (size_t i = mark; i < currentSize; ++i) {
			Traits::destroy(r_root, m_resources[i]);
		}
	}

	_NODISCARD _infoTy* createInfo() noexcept {
		return m_ownedInfo.get();
	}

	size_t size() const noexcept {
		return currentSize;
	}

	_resultTy grant(_Ty* resource) noexcept {
		if (mark >= currentSize) {
			_resultTy result = resize(currentSize ? currentSize * 2 : 1);
			if (result != Traits::success())
				return result;
		}

		*resource = m_resources[mark++];
		return Traits::success();
	}

	_resultTy recycle(_Ty resource) noexcept {
		if (!mark)
			return Traits::impermissible();

		m_resources[--mark] = resource;
		return Traits::success();
	}

	_resultTy root(_parentTy root) {
		if (r_root == root)
			return Traits::success();

		if (!currentSize) {
			r_root = root;
			return Traits::success();
		}

		if (mark)
			return Traits::impermissible();

		std::vector<_Ty> resources(currentSize);

		_resultTy result;
		for (size_t i = 0; i < currentSize; ++i) {
			result = Traits::create(root, m_ownedInfo.get(), &resources[i]);
			if (result == Traits::success())
				continue;

			for (size_t j = 0; j < i; ++j) {
				Traits::destroy(root, resources[j]);
			}

			return result;
		}

		for (size_t i = 0; i < currentSize; ++i) {
			Traits::destroy(r_root, m_resources[i]);
		}

		m_resources.swap(resources);
		r_root = root;

		return Traits::success();
	}

	_resultTy resize(size_t _Newsize) noexcept {
		if (_Newsize == currentSize)
			return Traits::success();

		if (_Newsize < currentSize) {
			if (_Newsize < mark)
				return Traits::impermissible();

			for (size_t i = _Newsize; i < currentSize; ++i) {
				Traits::destroy(r_root, m_resources[i]);
			}

			m_resources.resize(_Newsize);
			currentSize = _Newsize;

			return Traits::success();
		}

		m_resources.resize(_Newsize);

		_resultTy result;
		for (size_t i = currentSize; i < _Newsize; ++i) {
			result = Traits::create(r_root, m_ownedInfo.get(), &m_resources[i]);
			if (result == Traits::success()) {
				continue;
			}

			for (size_t j = currentSize; j < i; ++j) {
				Traits::destroy(r_root, m_resources[j]);
			}

			m_resources.resize(currentSize);

			return result;
		}

		currentSize = _Newsize;

		return Traits::success();
	}

};
