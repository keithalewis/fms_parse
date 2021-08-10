// fms_char_view.h - view of contiguous characters
#pragma once
#include "fms_view.h"
#include <cctype>

namespace fms {

	struct char_view : public view<const char> {
		// bring in all constructors
		using view<const char>::view;

		char_view()
		{ }
		// chop null terminator
		template<size_t N>
		char_view(const char(&buf)[N])
			: view(buf, static_cast<int>(N - 1))
		{ }
		char_view(const char_view&) = default;
		char_view& operator=(const char_view&) = default;
		~char_view()
		{ }

		bool equal(const char* s) const
		{
			const char* b = buf;

			while (*s and b < buf + len) {
				if (*s++ != *b++) {
					return false;
				}
			}

			return *s == 0 and b == buf + len;
		}
		bool equal(const char* s, int n) const
		{
			return len == n and 0 == strncmp(buf, s, n);
		}

		// eat t or return error with view unchanged
		char_view eat(char t)
		{
			if (!len or *buf != t) {
				return char_view(__FUNCTION__ ": indigestion", -1);
			}

			drop(1);

			return *this;
		}
		// eat null terminated s or return error with view unchanged
		char_view eat(const char* s)
		{
			char_view v_{ *this };

			while (v_ and *s) {
				v_ = v_.eat(*s++);
			}

			if (v_) {
				if (*s != 0) {
					v_.len = -1;
					v_.buf = __FUNCTION__ ": still hungry";
				}
				else {
					// ate everything
					operator=(v_);
				}
			}

			return v_ ? *this : v_;
		}
		// eat n characters of s or return error with view unchanged
		char_view eat(const char* s, int n)
		{
			char_view v_{ *this };

			while (v_ and n) {
				v_.eat(*s++);
				--n;
			}

			if (v_) {
				if (n != 0) {
					v_.len = -1;
					v_.buf = __FUNCTION__ ": still hungry";
				}
				else {
					operator=(v_);
				}
			}

			return v_ ? *this : v_;
		}

		// remove white space from beginning
		char_view& wstrim()
		{
			while (len and std::isspace(front())) {
				drop(1);
			}

			return *this;
		}
		// remove white space from end
		char_view& trimws()
		{
			while (len and std::isspace(back())) {
				drop(-1);
			}

			return *this;
		}


#ifdef _DEBUG
		static int test()
		{
			{
				char_view v("abc");
				v.eat(v.front());
				assert(v);
				assert(v.equal("bc"));
				assert(!v.eat('c'));
				assert(v.equal("bc"));
			}
			{
				char_view v(" \tabc\n");
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

	inline char_view eat(char_view v, char t)
	{
		return v.eat(t);
	}
	inline char_view eat(char_view v, const char* s, int n = 0)
	{
		return v.eat(s, n);
	}
	inline char_view wstrim(char_view v)
	{
		return v.wstrim();
	}

#ifdef _DEBUG

	inline int eat_test()
	{
		{
			char_view v("abc");
			assert(v.eat("ab"));
			assert(v.equal("c"));
		}
		{
			char_view v("abc");
			assert(!v.eat("ac"));
			assert(v.equal("abc"));
		}
		{
			char_view v("abc");
			v = eat(v, "ac", 1);
			assert(v);
			assert(v.equal("bc"));
		}

		return 0;
	}

#endif // _DEBUG


} // namespace fms
