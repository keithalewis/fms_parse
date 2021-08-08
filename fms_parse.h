// fms_parse.h - parse contiguous data
// View based parsing. Return parsed value and advance view.
// If len < 0 then view is an error with buf pointing at the error message
#pragma once
#ifdef _DEBUG
#include <cassert>
#endif
#include <ctime>
#include <algorithm>
#include <charconv>
#include <chrono>
#include <iterator>

namespace fms::parse {

	template<class T>
	struct view {
		T* buf;
		int len;

		using iterator_category = std::input_iterator_tag;
		using value_type = T;
		using reference = T&;

		view()
			: buf(nullptr), len(0)
		{ }
		view(T* buf, int len)
			: buf(buf), len(len)
		{ }
		template<size_t N>
		view(T(&buf)[N])
			: view(buf, static_cast<int>(N))
		{
			if constexpr (std::is_same_v<char, std::remove_const<T>::type>) {
				--len;
			}
		}
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
		bool operator==(const view& v) const
		{
			return len == v.len and buf == v.buf;
		}
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

		T operator*() const
		{
			return buf[0];
		}
		T& operator*()
		{
			return buf[0];
		}
		view& operator++()
		{
			++buf;
			--len;

			return *this;
		}
		view operator++(int)
		{
			view v_(*this);

			++buf;
			--len;

			return v_;
		}

		T front() const
		{
			return buf[0];
		}
		T back() const
		{
			return buf[len - 1];
		}

		view& drop(int n)
		{
			buf += n;
			len -= n;

			return *this;
		}

		view& eat(T t)
		{
			if (buf[0] != t) {
				len = -1;
				buf = __FUNCTION__ ": indigestion";
			}
			else {
				drop(1);
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
				view<const char> v("123");
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
				assert(v.equal(view<const char>("23")));
				assert(*v == '2');
				++v;
				assert(*v == '3');
				v.eat('3');
				assert(!v);
				v.drop(-1); // unsafe
				assert(*v == '3');
			}

			return 0;
		}
#endif // _DEBUG
	};

	// convert to type from characters
	template<class X, class T>
	inline X to(view<T>& v)
	{
		X x;

		std::from_chars_result result = std::from_chars(v.buf, v.buf + v.len, x);
		if (result.ec != std::errc()) {
			v.len = -1;
			v.buf = __FUNCTION__ ": std::from_chars failed";
		}
		else {
			v.drop(static_cast<int>(result.ptr - v.buf));
		}

		return x;
	}

#ifdef _DEBUG
	inline int to_test()
	{
		{
			view v("123abc");
			auto x = to<int>(v);
			assert(x == 123);
			assert(v.equal(view("abc")));
		}
		{
			view v("1.23abc");
			auto x = to<double>(v);
			assert(x == 1.23);
			assert(v.equal(view("abc")));
		}

		return 0;
	}
#endif // _DEBUG

} // namespace fms::parse