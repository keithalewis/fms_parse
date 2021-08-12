// fms_char_view.h - view of contiguous characters
#pragma once
#include "fms_view.h"
#include <cctype>

namespace fms {

	template<class T> // require std::is_same_v<char,remove_const<T>::value>> || wchar_t
	struct char_view : public view<T> {
		// bring in all constructors
		using view<T>::view;
		using view<T>::buf;
		using view<T>::len;

		char_view()
		{ }
		// chop null terminator
		template<size_t N>
		char_view(T(&buf)[N])
			: view<T>(buf, static_cast<int>(N - 1))
		{ }
		char_view(const char_view&) = default;
		char_view& operator=(const char_view&) = default;
		~char_view()
		{ }

		bool equal(T const* s) const
		{
			T const* b = buf;

			while (*s and b < buf + len) {
				if (*s++ != *b++) {
					return false;
				}
			}

			return *s == 0 and b == buf + len;
		}

		// eat t or return error with view unchanged
		bool eat(T t)
		{
			if (!len or *buf != t) {
				return false;
			}

			view<T>::drop(1);

			return true;
		}
		// eat null terminated s or return error with view unchanged
		bool eat(T const* s)
		{
			char_view v_{ *this };

			while (*s and v_) {
				if (!v_.eat(*s++)) {
					return false;
				}
			}

			if (*s != 0) {
				return false;
			}
			else {
				operator=(v_);
			}

			return true;
		}
		// eat n characters of s or return error with view unchanged
		bool eat(T const* s, int n)
		{
			char_view v_{ *this };

			while (v_ and n) {
				if (!v_.eat(*s++)) {
					return false;
				}
				--n;
			}

			if (n != 0) {
				return false;
			}
			else {
				operator=(v_);
			}

			return true;
		}

		// remove white space from beginning
		char_view& wstrim()
		{
			while (len and std::isspace(view<T>::front())) {
				view<T>::drop(1);
			}

			return *this;
		}
		// remove white space from end
		char_view& trimws()
		{
			while (len and std::isspace(view<T>::back())) {
				view<T>::drop(-1);
			}

			return *this;
		}


#ifdef _DEBUG
		static int test()
		{
			{
				char_view<const char> v("abc");
				assert(v.eat(v.front()));
				assert(v);
				assert(v.equal("bc"));
				assert(!v.eat('c'));
				assert(v.equal("bc"));
			}
			{
				char_view<const wchar_t> v(L"abc");
				assert(v.eat(v.front()));
				assert(v);
				assert(v.equal(L"bc"));
				assert(!v.eat(L'c'));
				assert(v.equal(L"bc"));
			}
			{
				char_view<const char> v(" \tabc\n");
				v.wstrim();
				assert(v.equal("abc\n"));
				v.wstrim();
				assert(v.equal("abc\n"));
				v.trimws();
				assert(v.equal("abc"));
				v.trimws();
				assert(v.equal("abc"));
			}

			return 0;
		}
#endif // _DEBUG
	};

	template<class T>
	inline char_view<T> eat(char_view<T> v, T t)
	{
		return v.eat(t);
	}
	template<class T>
	inline char_view<T> eat(char_view<T> v, const T* s, int n = 0)
	{
		return v.eat(s, n);
	}
	template<class T>
	inline char_view<T> wstrim(char_view<T> v)
	{
		return v.wstrim();
	}

#ifdef _DEBUG

	template<class T>
	inline int eat_test()
	{
		{
			char_view<const T> v("abc");
			assert(v.eat("ab"));
			assert(v.equal("c"));
		}
		{
			char_view<const T> v("abc");
			assert(!v.eat("ac"));
			assert(v.equal("abc"));
		}
		{
			char_view<const T> v("abc");
			assert(v.eat("ac", 1));
			assert(v);
			assert(v.equal("bc"));
		}

		return 0;
	}

#endif // _DEBUG


} // namespace fms
