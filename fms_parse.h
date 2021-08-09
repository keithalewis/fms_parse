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
		using pointer = T*;
		using difference_type = ptrdiff_t;

		view()
			: buf(nullptr), len(0)
		{ }
		view(T* buf, int len)
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

		view take(int n) const
		{
			return view(buf, n);
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
				v.drop(-1); // unsafe
				assert(*v == '3');
			}

			return 0;
		}
#endif // _DEBUG
	};

	struct char_view : public view<const char> {
		template<size_t N>
		char_view(const char(&buf)[N])
			: view(buf, static_cast<int>(N - 1))
		{
		}
		char_view& eat(char t)
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

		char_view& eatws()
		{
			while (len and isspace(*buf)) {
				--len;
				++buf;
			}

			return *this;
		}
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
			char_view v("abc");
			char buf[3];
			std::copy(v.begin(), v.end(), buf);
			assert(0 == strncmp(buf, "abc", 3));
		}

		{
			char_view v("123abc");
			auto x = to<int>(v);
			assert(x == 123);
			assert(v.equal(char_view("abc")));
		}
		{
			char_view v("1.23abc");
			auto x = to<double>(v);
			assert(x == 1.23);
			assert(v.equal(char_view("abc")));
		}

		return 0;
	}
#endif // _DEBUG

	// skip matching left and right chars ignoring escaped
	// "{da\}ta}..." returns escaped "da\}ta" an updates fms::view to "..."
	inline char_view chop(char_view& v, char l, char r, char e)
	{
		auto tok = v;

		if (l == e or r == e) {
			v.len = -1;
			v.buf = __FUNCTION__ ": delimiters can't be used as escape";
		}
		else if (v) {
			tok.eat(l);
			v.eat(l);
			
			int level = 1;
			while (v and level) {
				if (v.front() == e) {
					v.drop(2);
				}
				// do first in case l == r
				if (v.front() == r) {
					--level;
				}
				else if (v.front() == l) {
					++level;
				}
				++v;
			};
			if (level != 0) {
				v.len = -1;
				v.buf = __FUNCTION__ ": left and right delimiters not matched";
			}
			else {
				tok.len = static_cast<int>(v.buf - tok.buf - 1);
			}
		}

		return tok;
	}

	class chopable {
		view<const char> v;
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = view<const char>;
		using refference = view<const char>&;
		using pointer = view<const char>*;
		using difference_type = ptrdiff_t;
	};
#ifdef _DEBUG

	inline int token_test()
	{
		{
			char_view v("{a}");
			auto t = chop(v, '{', '}', '\\');
			assert(t.equal(char_view("a")));
			assert(!v);
		}
		{
			char_view v("{a\\}b}c");
			auto t = chop(v, '{', '}', '\\');
			assert(t.equal(char_view("a\\}b")));
			assert(v.equal(char_view("c")));
		}
		{
			char_view v("{a\\}{b}}c");
			auto t = chop(v, '{', '}', '\\');
			assert(t.equal(char_view("a\\}{b}")));
			assert(v.equal(char_view("c")));
		}

		return 0;
	}

#endif // _DEBUG

} // namespace fms::parse