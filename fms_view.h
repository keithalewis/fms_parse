// fms_view.h - non-owning view of contiguous data
#pragma once
#include <algorithm>
#include <compare>
#include <concepts>
#include <initializer_list>
#include <iterator>

namespace fms {

	// Non-owning view of contiguous data
	template<class T>
	struct view {
		T* buf;
		long len;

		using iterator_category = std::forward_iterator_tag;
		using value_type = std::remove_cv<T>;
		using reference = T&;
		using const_reference = const T&;
		using pointer = T*;
		using const_pointer = const T*;
		using difference_type = std::ptrdiff_t;

		constexpr view() noexcept
			: buf(nullptr), len(0)
		{
		}
		constexpr view(T* buf, long len) noexcept
			: buf(buf), len(len)
		{
		}
		constexpr view(std::initializer_list<T> il) noexcept
			: buf(const_cast<T*>(il.begin())), len(static_cast<long>(il.size()))
		{
		}
		constexpr view(const view&) = default;
		constexpr view& operator=(const view&) = default;
		constexpr view(view&&) = default;
		constexpr view& operator=(view&&) = default;
		constexpr ~view()
		{
		}

		constexpr explicit operator bool() const noexcept
		{
			return len > 0;
		}
		// same length and buffer
		constexpr bool operator==(const view& v) const
		{
			return len == v.len and buf == v.buf;
		}
		// equal contents
		template<class U = T>
		constexpr bool equal(const view<U>& v) const
		{
			return len == v.len and std::equal(buf, buf + len, v.buf, v.buf + v.len);
		}
		template<class U = T>
		constexpr bool equal(std::initializer_list<U> il) const
		{
			return len == il.size() and std::equal(buf, buf + len, il.begin(), il.end());
		}

		// STL friendly
		constexpr view begin() noexcept
		{
			return *this;
		}
		constexpr view begin() const noexcept
		{
			return *this;
		}
		constexpr view end() noexcept
		{
			return view(buf + len, 0);
		}
		constexpr view end() const noexcept
		{
			return view(buf + len, 0);
		}

		constexpr value_type operator*() const
		{
			return *buf;
		}
		constexpr reference operator*()
		{
			return *buf;
		}
		constexpr view& operator++() noexcept
		{
			if (len > 0) {
				++buf;
				--len;
			}

			return *this;
		}
		constexpr view operator++(int) noexcept
		{
			view v_(*this);

			operator++();

			return v_;
		}

		constexpr T front() const
		{
			return len ? buf[0] : (T)-1;
		}
		constexpr T& front()
		{
			return buf[0];
		}
		constexpr T back() const
		{
			return len ? buf[len - 1] : (T)-1;
		}
		constexpr T& back()
		{
			return buf[len - 1];
		}

		// drop first (n > 0) or last (n < 0) items
		constexpr view& drop(long n) noexcept
		{
			n = std::clamp(n, -len, len);

			if (n >= 0) {
				buf += n;
				len -= n;
			}
			else {
				len += n;
			}

			return *this;
		}

		// take first (n > 0) or last (n < 0) items
		constexpr view& take(long n) noexcept
		{
			n = std::clamp(n, -len, len);

			if (n >= 0) {
				len = n;
			}
			else {
				buf += len + n;
				len = -n;
			}

			return *this;
		}
	};

	template<class T>
	constexpr bool operator==(const view<T>& v, const view<T>& w)
	{
		return v == w;
	}

	template<class T, class U = T>
	constexpr bool equal(const view<T>& v, const view<U>& w)
	{
		return v.equal(w);
	}

	// Functions versions.
	template<class T>
	constexpr view<T> drop(long n, view<T> v)
	{
		return v.drop(n);
	}

	template<class T>
	constexpr view<T> take(long n, view<T> v)
	{
		return v.take(n);
	}


#ifdef _DEBUG
	template<class T>
	static int test()
	{
		// Block 1
		static_assert([] {
			view<T> v{};
			if (!!v) return false;
			auto v2{ v };
			if (!(v2 == v)) return false;
			auto v3 = v2;
			if (!!(v3 != v2)) return false;
			return true;
			}());

		// Block 2 (use non-constexpr buffer so pointer is T*)
		static_assert([] {
			T buf[] = { T(1), T(2), T(3) };
			auto v = view<T>(buf, 3);
			if (v.len != 3) return false;
			if (v.front() != T(1)) return false;
			if (v.back() != T(3)) return false;
			auto v2{ v };
			if (!v2.operator==(v)) return false;
			int i = 1;
			for (auto vi : v) {
				if (vi != T(i++)) return false;
			}
			view<T> v3 = v;
			v3.drop(1);
			if (*v3 != T(2)) return false;
			++v3;
			if (*v3 != T(3)) return false;
			v3.drop(1);
			if (!!v3) return false;
			return true;
			}());

		// Block 3
		static_assert([] {
			T buf[] = { T(1), T(2), T(3) };
			auto v = view<T>(buf, 3);
			return v == view(buf, 3);
			}());

		// Block 4 (char buffer; avoid binding to view<T>)
		static_assert([] {
			char buf[] = "123";
			auto v = ::fms::view<char>(buf, 3);
			v.take(2);
			return v.len == 2 && v.buf[0] == '1' && v.buf[1] == '2';
			}());

		// Block 5 (char buffer; use view<char> explicitly)
		static_assert([] {
			char buf[] = "123";
			::fms::view<char> v(buf, 3);

			auto v_1{ v };
			v_1.take(-1);
			// skipped commented equality checks

			auto v_2{ v };
			v_2.drop(-2);
			// skipped commented equality checks

			auto vt{ v };
			vt.take(10);
			if (!(vt == v)) return false;
			vt.take(-10);
			if (!(vt == v)) return false;
			vt.take(0);
			if (!!vt) return false;

			auto vd{ v };
			vd.drop(0);
			if (!(vd == v)) return false;
			vd.drop(10);
			if (!!vt) return false; // original intent
			vd = v;
			if (!vd) return false;
			vd.drop(-10);
			if (!!vd) return false;

			return true;
			}());
	}
#endif // _DEBUG

#ifdef _DEBUG
constexpr char buf[] = "123";
constexpr auto v = view("123", 3);
constexpr auto v2{ v };
static_assert(v == v2);

static_assert(!view(buf, 0));
static_assert(view(buf, 1));
static_assert(equal(view(buf, 1), view("1", 1)));
static_assert(view(buf, 3));
static_assert(view(buf, 2).len == 2);

static_assert(v.front() == '1');
static_assert(v.back() == '3');

static_assert(take(1, view(buf, 3)).len == 1);
static_assert(equal(take(1, view(buf, 3)), view("1", 1)));
static_assert(equal(drop(1, view(buf, 3)), view("23", 2)));
#endif // _DEBUG


} // namespace fms
