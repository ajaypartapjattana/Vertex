#pragma once

#include <cstddef>
#include <vector>
#include <stdexcept>
#include <type_traits>

namespace sgb {

	template <typename T>
	class span {
	public:
		using element_type = T;
		using value_type = std::remove_cv_t<T>;
		using pointer = T*;
		using reference = T&;
		using size_type = std::size_t;

		using iterator = T*;
		using const_iterator = const T*;

		using difference_type = std::ptrdiff_t;

		constexpr span() noexcept
			:
			m_data(nullptr),
			m_size(0)
		{

		}

		constexpr span(pointer data, size_type size) noexcept
			:
			m_data(data),
			m_size(size)
		{

		}

		constexpr span(pointer begin, pointer end) noexcept
			:
			m_data(begin),
			m_size(end - begin)
		{

		}

		template <size_t N>
		constexpr span(T (&array)[N]) noexcept
			:
			m_data(array),
			m_size(N)
		{

		}

		template <typename Alloc, typename U = T, typename = std::enable_if_t<!std::is_const_v<U>>>
		span(std::vector<value_type, Alloc>& vec) noexcept
			: 
			m_data(vec.data()), 
			m_size(vec.size()) 
		{

		}

		template <typename Alloc, typename U = T, typename = std::enable_if_t<std::is_const_v<U>>>
		span(const std::vector<value_type, Alloc>& vec) noexcept
			: 
			m_data(vec.data()), 
			m_size(vec.size())
		{

		}

		template <typename U, typename = std::enable_if_t<std::is_convertible_v<U(*)[], T(*)[]>>>
		constexpr span(const span<U>& other) noexcept
			:
			m_data(other.data()),
			m_size(other.size())
		{

		}

		[[nodiscard]] constexpr pointer data() const noexcept {
			return m_data;
		}

		[[nodiscard]] constexpr size_type size() const noexcept {
			return m_size;
		}

		[[nodiscard]] constexpr bool empty() const noexcept {
			return m_size == 0;
		}

		constexpr reference operator[](size_type i) const noexcept {
			return m_data[i];
		}

		constexpr reference at(size_type i) const {
			if (i >= m_size)
				throw std::out_of_range("span::at");
			
			return m_data[i];
		}

		constexpr reference front() const noexcept {
			return m_data[0];
		}

		constexpr reference back() const noexcept {
			return m_data[m_size - 1];
		}

		constexpr iterator begin() noexcept {
			return m_data;
		}

		constexpr iterator end() noexcept {
			return m_data + m_size;
		}

		constexpr const_iterator begin() const noexcept {
			return m_data;
		}

		constexpr const_iterator end() const noexcept {
			return m_data + m_size;
		}

		constexpr const_iterator cbegin() const noexcept {
			return m_data;
		}

		constexpr const_iterator cend() const noexcept {
			return m_data + m_size;
		}

		[[nodiscard]] constexpr span subspan(size_type offset, size_type count) const {
			if (offset > m_size || count > m_size - offset)
				throw std::out_of_range("span::subspan");

			return span{ m_data + offset, count };
		}

		[[nodiscard]] constexpr span subspan(size_type offset) const {
			if (offset > m_size)
				throw std::out_of_range("span::subspan");

			return span{ m_data + offset, m_size - offset };
		}

	private:
		T* m_data;
		size_type m_size;

	};

}
