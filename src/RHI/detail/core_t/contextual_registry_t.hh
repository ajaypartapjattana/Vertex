#pragma once

template <typename Traits>
class contextual_registry {
private:

	using _Ty = typename Traits::handle_type;
	using _parentTy = typename Traits::parent_type;
	using _resultTy = typename Traits::result_type;
	using _infoTy = typename Traits::createInfo_type;

	_Ty m_resource = Traits::null();

	_parentTy r_root = Traits::null();

public:
	contextual_registry() noexcept = default;
	explicit contextual_registry(_parentTy root) noexcept
		:
		r_root(root) 
	{

	}
	contextual_registry(const contextual_registry&) = delete;
	contextual_registry(contextual_registry&& other) noexcept
		:
		m_resource(std::exchange(other.m_resource, Traits::null())),
		r_root(std::exchange(other.r_root, Traits::null()))
	{

	}

	contextual_registry& operator=(const contextual_registry&) = delete;
	contextual_registry& operator=(contextual_registry&& other) noexcept {
		if (this == &other)
			return *this;

		reset();

		m_resource = std::exchange(other.m_resource, Traits::null());
		r_root = std::exchange(other.r_root, Traits::null());

		return *this;
	}

	~contextual_registry() noexcept {
		reset();
	}

	operator _Ty() const noexcept {
		return m_resource;
	}

	explicit operator bool() const noexcept {
		return m_resource != Traits::null();
	}

	_Ty* data() noexcept {
		return &m_resource;
	}

	const _Ty* data() const noexcept {
		return &m_resource;
	}

	_resultTy create(typename _infoTy* pInfo) noexcept {
		_Ty resource = Traits::null();
		_resultTy result = Traits::create(r_root, pInfo, &resource);
		if (result != Traits::success())
			return result;

		reset();

		m_resource = resource;

		return Traits::success();
	}

	void root(_parentTy root) noexcept {
		if (root == r_root)
			return;

		reset();
		r_root = root;
	}

	_parentTy get_root() const noexcept {
		return r_root;
	}

	_Ty release() noexcept {
		_Ty resource = m_resource;
		m_resource = Traits::null();
		return resource;
	}

	void reset() noexcept {
		if (m_resource != Traits::null())
			Traits::destroy(r_root, m_resource);

		m_resource = Traits::null();
	}

};