// fms_char_view.h - view of contiguous characters
// Works for T = char or wchar_t.
#ifndef FMS_CHAR_VIEW_INCLUDED
#define FMS_CHAR_VIEW_INCLUDED
#include "fms_view.h"
#include <cctype>

namespace fms {

	// Length of null terminated string.
	template<class T>
	constexpr long length(const T* s)
	{
		long n = 0;
		while (s and *s++) {
			++n;
		}
		return n;
	}
	static_assert(length("abc") == 3);

	template<class T>
	constexpr T abs(T x)
	{
		return x < 0 ? -x : x;
	}
	static_assert(abs(1) == 1);
	static_assert(abs(0) == 0);
	static_assert(abs(-1) == 1);

	// Is a base b digit.
	template<class T>
	constexpr bool is_digit(T c, size_t b)
	{
		return (c >= '0' and c <= '9') or (c >= 'a' and c < 'a' + (b - 10)) or (c >= 'A' and c < 'A' + (b - 10));
	}
#ifdef _DEBUG
	static_assert(is_digit('0',10));
	static_assert(is_digit('a', 11));
	static_assert(!is_digit('b', 11));
	static_assert(is_digit(L'0', 10));
	static_assert(!is_digit(L'a', 10));
#endif // _DEBUG	
	template<class T>
	constexpr bool is_digit(T c)
	{
		return is_digit(c, 10);
	}
#ifdef _DEBUG
	static_assert(is_digit('0'));
	static_assert(!is_digit('a'));
	static_assert(is_digit(L'0'));
	static_assert(!is_digit(L'a'));
#endif // _DEBUG	

	template<class T>
	constexpr bool is_xdigit(T c)
	{
		return is_digit(c, 16);
	}
#ifdef _DEBUG
	static_assert(is_xdigit('0'));
	static_assert(is_xdigit('a'));
	static_assert(is_xdigit(L'0'));
	static_assert(!is_digit(L'G'));
#endif // _DEBUG	

	// Is a JSON space character, form feed, or vertical tab.
	template<class T>
	constexpr bool is_space(T c)
	{
		return c == ' ' or c == '\t' or c == '\n' or c == '\r' or c == '\f' or c == '\v';
	}
#ifdef _DEBUG
	static_assert(is_space(' '));
	static_assert(is_space('\t'));
	static_assert(is_space('\n'));
	static_assert(is_space('\r'));
	static_assert(is_space('\f'));
	static_assert(is_space('\v'));
	static_assert(!is_space('a'));
#endif // _DEBUG	


	// std::integral T ???
	template<class T> 
	struct char_view : public view<T> {
		using view<T>::view; // bring in all constructors
		using view<T>::buf;
		using view<T>::len;
		using view<T>::equal;

		constexpr char_view()
		{ }
		// Construct from counted (len > 0) or null terminated string.		 
		constexpr char_view(T* buf, int len = 0)
            : view<T>(buf, len ? len : length(buf))
        { }
		// chop null terminator
		template<size_t N>
		constexpr char_view(T(&buf)[N])
			: view<T>(buf, static_cast<int>(N - 1))
		{ }
		constexpr char_view(const char_view&) = default;
		constexpr char_view& operator=(const char_view&) = default;
		constexpr ~char_view()
		{ }

		// Indicate error using negative length.
		constexpr char_view error() const noexcept
		{
			return char_view(buf, -abs(len));
		}
		// Return view of error contents
		constexpr char_view error_view() const noexcept
		{
			return char_view(buf, abs(len));
		}
		constexpr bool is_error() const noexcept
		{
			return len < 0;
		}

		// Equal contents with first n characters of s or until null if n = 0
		constexpr bool equal(const char* s, long n = 0) const noexcept
		{
			return equal(char_view<const char>(s, n ? n : length(s)));
		}

		// eat t or return error with view unchanged
		constexpr char_view& eat(T t) noexcept
		{
			if (!len or *buf != t) {
				*this = error();
			}
			else {
				view<T>::drop(1);
			}

			return *this;
		}
		// eat n characters or until null of s 
		constexpr char_view& eat(const T* s, long n = 0)
		{
			if (n > len) {
				return *this = error();
			}
			if (n == 0) {
				n = length(s);
			}
			while (len and n-- and !is_error()) {
				eat(*s++);
			}
	
			return *this;
		}

		// remove white space from beginning
		constexpr char_view& ws_trim()
		{
			while (len and is_space(view<T>::front())) {
				view<T>::drop(1);
			}

			return *this;
		}
		// remove white space from end
		constexpr char_view& trim_ws()
		{
			while (len and is_space(view<T>::back())) {
				view<T>::drop(-1);
			}

			return *this;
		}
		constexpr char_view& trim()
		{
			return ws_trim().trim_ws();
		}
	};
#ifdef _DEBUG
	static_assert(char_view<int>().len == 0);
	static_assert(!char_view<char>());
	static_assert(char_view("abc").len == 3);
	static_assert(length(char_view("abc").buf) == 3);
	static_assert(!char_view(L"A").is_error());
	static_assert(char_view(L"A").error().is_error());
	static_assert(char_view<char>().equal(""));
	//static_assert(!char_view<char>("").equal("a"));
	static_assert(char_view("a").equal("a"));
	static_assert(!char_view("a").equal("b", 0));
	static_assert(char_view("abc").equal("abc", 0));
	static_assert(!char_view("abc").equal("abx", 0));
	static_assert(!char_view("abc").equal("abcd", 0));
	static_assert(char_view("abc").equal("abd", 2));
	static_assert(!char_view("abc").equal("ac", 2));
	static_assert(char_view(L"abc").equal("abc", 0));
	static_assert(!char_view(L"abc").equal("abx"));
	static_assert(char_view(L"abc").equal("ab", 2));
	static_assert(!char_view(L"abc").equal("ac", 2));
	// Simple success: eat one char
	static_assert([]() constexpr {
		fms::char_view<const char> v("abc");
		v.eat('a');
		return v.equal("bc");
		}());

	// Eat a string (auto-length)
	static_assert([]() constexpr {
		fms::char_view<const char> v("abc");
		v.eat("ab");
		return v.equal("c");
		}());

	// Error path: wrong char sets error and preserves error_view()
	static_assert([]() constexpr {
		fms::char_view<const char> v("abc");
		v.eat('x');
		return v.is_error() && v.error_view().equal("abc");
		}());

	// Bounded eat: consume only first of "ac"
	static_assert([]() constexpr {
		fms::char_view<const char> v("abc");
		v.eat("ac", 1);
		return v.equal("bc") && !v.is_error();
		}());

	// Wide-char variant
	static_assert([]() constexpr {
		fms::char_view<const wchar_t> v(L"abc");
		v.eat(L'a');
		return v.equal("bc");
		}());
#endif // _DEBUG

	// Function versions
	template<class T>
	constexpr char_view<T> eat(char_view<T> v, T t)
	{
		return v.eat(t);
	}
	template<class T>
	constexpr char_view<T> eat(char_view<T> v, const T* s, int n = 0)
	{
		return v.eat(s, n);
	}
	// T might not be char or wchar_t
	template<class T>
	constexpr char_view<T> eat(char_view<T> v, const char* s, int n = 0)
	{
		return v.eat(s, n);
	}
	template<class T>
	constexpr char_view<T> ws_trim(char_view<T> v)
	{
		return v.ws_trim();
	}
	template<class T>
	constexpr char_view<T> trim_ws(char_view<T> v)
	{
		return v.trim_ws();
	}

#ifdef _DEBUG

	static_assert([] {
		char buf[] = "123";
		fms::char_view<char> v(buf, 3);
		return v.equal(char_view("123"))
			&& !v.equal(char_view("12"))
			&& !v.equal(char_view("124"))
			&& !v.equal(char_view("1234"))
			&& v.equal("12", 2)
	//		&& !v.equal(char_view("13"), 2)
			&& v.equal({ '1', '2', '3' })
			&& v.equal({ '0' + 1, '0' + 2, '0' + 3 })
			&& !v.equal({ '1', '2' });
		}());


	template<class T>
	inline int eat_test()
	{
		// Block 1
		static_assert([]() constexpr {
			char buf[] = "abc";
			fms::char_view<char> v(buf);
			v.eat(v.front());
			if (!v.equal("bc")) return false;
			v.eat('c');
			if (!v.is_error()) return false;
			return v.error_view().equal("bc");
		}());

		// Block 2
		static_assert([]() constexpr {
			fms::char_view<const wchar_t> v(L"abc");
			if (!static_cast<bool>(v.eat(v.front()))) return false;
			if (!static_cast<bool>(v)) return false;
			if (!v.equal("bc")) return false;
			if (static_cast<bool>(v.eat('c'))) return false;
			return v.error_view().equal("bc");
		}());

		// Block 3
		static_assert([]() constexpr {
			fms::char_view<const char> v(" \tabc\n");
			v.ws_trim();
			if (!v.equal("abc\n")) return false;
			v.ws_trim();
			if (!v.equal("abc\n")) return false;
			v.trim_ws();
			if (!v.equal("abc")) return false;
			v.trim_ws();
			return v.equal("abc");
		}());

		// Block 4
		static_assert([]() constexpr {
			fms::char_view<const T> v("abc");
			v.eat("ab");
			return v.equal("c");
		}());

		// Block 5
		static_assert([]() constexpr {
			fms::char_view<const T> v("abc");
			v.eat("ac");
			return v.is_error() && v.error_view().equal("bc");
		}());

		// Block 6
		static_assert([]() constexpr {
			fms::char_view<const T> v("abc");
			v.eat("ac", 1);
			return !v.is_error() && v.error_view().equal("bc");
		}());

		return 0;
	}

#endif // _DEBUG


} // namespace fms

#endif // FMS_CHAR_VIEW_INCLUDED