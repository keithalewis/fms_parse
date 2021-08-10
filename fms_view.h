// fms_view.h - view of contiguous data
#pragma once
#ifdef _DEBUG
#include <cassert>
#endif
#include <algorithm>
#include <compare>
//#include <concepts>
#include <iterator>

namespace fms {

	/// <summary>
	/// Non-owning view of contiguous data
	/// </summary>
	/// <remarks>
	/// No exceptions are thrown. Errors are indicated by negative length.
	/// </remarks>
	template<class T>
	struct view {
		T* buf;
		int len;

		using iterator_category = std::input_iterator_tag;
		using value_type = T;
		using reference = T&;
		using pointer = T*;
		using difference_type = ptrdiff_t;

		view() noexcept
			: buf(nullptr), len(0)
		{ }
		view(T* buf, int len) noexcept
			: buf(buf), len(len)
		{ }
		view(const view&) = default;
		view& operator=(const view&) = default;
		~view()
		{ }

		bool is_error() const
		{
			return len < 0;
		}
		explicit operator bool() const
		{
			return len > 0;
		}
		auto operator<=>(const view& v) const = default;
		bool equal(const view& v) const
		{
			return len == v.len and std::equal(buf, buf + len, v.buf);
		}

		// STL friendly
		view begin() const
		{
			return *this;
		}
		view end() const
		{
			return view(buf + len, 0);
		}

		value_type operator*() const
		{
			return buf[0];
		}
		reference operator*()
		{
			return buf[0];
		}
		view& operator++()
		{
			if (len > 0) {
				++buf;
				--len;
			}

			return *this;
		}
		view operator++(int)
		{
			view v_(*this);

			if (len > 0) {
				++buf;
				--len;
			}

			return v_;
		}

		// no bounds checking
		T operator[](int n) const
		{
			return buf[n];
		}
		T& operator[](int n)
		{
			return buf[n];
		}

		T front() const
		{
			return buf[0];
		}
		T back() const
		{
			return buf[len - 1];
		}

		// drop first (n > 0) or last (n < 0) items
		view& drop(int n)
		{
			n = std::clamp(n, -len, len);

			if (n > 0) {
				buf += n;
				len -= n;
			}
			else if (n < 0) {
				take(len + n);
			}

			return *this;
		}

		// take first (n > 0) or last (n < 0) items
		view& take(int n)
		{
			n = std::clamp(n, -len, len);

			if (n >= 0) {
				len = n;
			}
			else {
				drop(len + n);
			}

			return *this;
		}

#ifdef _DEBUG
		static int test()
		{
			{
				view<char> v;
				assert(!v);
				auto v2{ v };
				assert(v2 == v);
				v = v2;
				assert(!(v != v2));
			}
			{
				char buf[] = "123";
				view<const char> v(buf, 3);
				assert(v.len == 3);
				assert(v.front() == '1');
				assert(v.back() == '3');
				auto v2{ v };
				assert(v2 == v);
				char i = '1';
				for (auto vi : v) {
					assert(vi == i++);
				}
				v.drop(1);
				assert(*v == '2');
				++v;
				assert(*v == '3');
				v.drop(1);
				assert(!v);
			}
			{
				char buf[] = "123";
				view<const char> v(buf, 3);
				v.take(2);
				assert(v.len == 2);
				assert(v.buf[0] == '1');
				assert(v.buf[1] == '2');
			}
			{
				char buf[] = "123";
				view<const char> v(buf, 3);
				for (int i = 0; i < 3; ++i) {
					assert(v[i] == buf[i]);
				}

				auto v_1{ v };
				v_1.take(-1);
				assert(v_1.len == 1);
				assert(v_1[0] == v[2]);

				auto v_2{ v };
				v_2.drop(-2);
				assert(v_2.len == 1);
				assert(v_2[0] == v[0]);
			}

			return 0;
		}
#endif // _DEBUG
	};

	template<class T>
	inline bool equal(const view<T>& v, const view<T>& w)
	{
		return v.equal(w);
	}

	template<class T>
	inline view<T> drop(view<T> v, int n)
	{
		return v.drop(n);
	}

	template<class T>
	inline view<T> take(view<T> v, int n)
	{
		return v.take(n);
	}

} // namespace fms
