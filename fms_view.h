// fms_view.h - non-owning view of contiguous data
#ifndef FMS_VIEW_INCLUDED
#define FMS_VIEW_INCLUDED
#include <algorithm>
#include <compare>
//#include <concepts>
//#include <initializer_list>
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
		long len;

		using iterator_category = std::input_iterator_tag;
		using value_type = T;
		using reference = value_type&;
		using pointer = value_type*;
		using difference_type = ptrdiff_t;

		view() noexcept
			: buf(nullptr), len(0)
		{ }
		view(T* buf, long len) noexcept
			: buf(buf), len(len)
		{ }
		view(const view&) = default;
		view& operator=(const view&) = default;
		//virtual ~view()
		//{ }

		bool is_error() const noexcept
		{
			return len < 0;
		}
		explicit operator bool() const noexcept
		{
			return len > 0;
		}
		bool operator==(const view& v) const
		{
			return len == v.len and std::equal(buf, buf + len, v.buf);
		}

		// STL friendly
		view begin() const noexcept
		{
			return *this;
		}
		view end() const noexcept
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
		view& operator++() noexcept
		{
			if (len > 0) {
				++buf;
				--len;
			}

			return *this;
		}
		view operator++(int) noexcept
		{
			view v_(*this);

			operator++();

			return v_;
		}

		// no bounds checking
		value_type operator[](int n) const
		{
			return buf[n];
		}
		reference operator[](int n)
		{
			return buf[n];
		}

		T front() const
		{
			return buf[0];
		}
		T& front()
		{
			return buf[0];
		}
		T back() const
		{
			return buf[len - 1];
		}
		T& back()
		{
			return buf[len - 1];
		}

		// drop first (n > 0) or last (n < 0) items
		view& drop(long n) noexcept
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
		view& take(long n) noexcept
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
				view<T> v;
				assert(!v);
				auto v2{ v };
				assert(v2 == v);
				auto v3 = v2;
				assert(!(v3 != v2));
			}
			{
				static T buf[] = { 1,2,3 };
				view<T> v(buf, 3);
				assert(v.len == 3);
				assert(v.front() == 1);
				assert(v.back() == 3);
				auto v2{ v };
				assert(v2.operator==(v));
				int i = 1;
				for (auto vi : v) {
					assert(vi == i++);
				}
				view v3 = v;
				v3.drop(1);
				assert(*v3 == buf[1]);
				++v3;
				assert(*v3 == buf[2]);
				v3.drop(1);
				assert(!v3);
			}
			{
				T buf[] = { 1,2,3 };
				view<T> v(buf, 3);
				assert(v == view(buf, 3));
				//assert(v.equal({ T(1),T(2),T(3) }));
			}
			{
				char buf[] = "123";
				view<char> v(buf, 3);
				v.take(2);
				assert(v.len == 2);
				assert(v.buf[0] == '1');
				assert(v.buf[1] == '2');
			}
			{
				char buf[] = "123";
				view<char> v(buf, 3);
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

				auto vt{ v };
				vt.take(10);
				assert(vt == v);
				vt.take(-10);
				assert(vt == v);
				vt.take(0);
				assert(!vt);

				auto vd{ v };
				vd.drop(0);
				assert(vd == v);
				vd.drop(10);
				assert(!vt);
				vd = v;
				assert(vd);
				vd.drop(-10);
				assert(!vd);
			}

			return 0;
		}
#endif // _DEBUG
	};

	template<class T>
	inline bool operator==(const view<T>& v, const view<T>& w)
	{
		return v == w;
	}

	template<class T>
	inline view<T> drop(long n, view<T> v)
	{
		return v.drop(n);
	}

	template<class T>
	inline view<T> take(long n, view<T> v)
	{
		return v.take(n);
	}

} // namespace fms

#endif // FMS_VIEW_INCLUDED
