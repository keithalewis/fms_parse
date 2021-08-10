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
			return 0 == strcmp(buf, s);
		}
		bool equal(const char* s, int n) const
		{
			return 0 == strncmp(buf, s, n);
		}

		char_view eat(char t)
		{
			if (!len or *buf != t) {
				return char_view(__FUNCTION__ ": indigestion", -1);
			}

			drop(1);

			return *this;
		}
		char_view eat(const char* s)
		{
			char_view v_{ *this };

			while (v_ and *s) {
				v_ = v_.eat(*s++);
			}

			// ate everything
			if (v_) {
				operator=(v_);
			}

			return v_ ? *this : v_;
		}
		char_view eat(const char* s, int n)
		{
			char_view v_{ *this };

			while (v_ and n) {
				v_.eat(*s++);
				--n;
			}
			if (n != 0) {
				v_.len = -1;
				v_.buf = __FUNCTION__ ": hungry";
			}
			else if (v_) {
				operator=(v_);
			}

			return v_ ? *this : v_;
		}

		char_view& eatws()
		{
			while (len and std::isspace(*buf)) {
				--len;
				++buf;
			}

			return *this;
		}

		char_view split(char c, char l, char r, char e)
		{
			const char* b = buf;
			
			while (b < buf + len and *b != c) {
				if (*b == l) {
					int level = 1;
					while (++b < buf + len and level) {
						if (*b == r) {
							--level;
						}
						else if (*b == l) {
							++level;
						}
						else if (*b == e) {
							++b;
						}
					}
					if (level != 0) {
						return char_view(__FUNCTION__ ": unmatched delimiters", -1);
					}
				}
				++b;
			}

			int n = static_cast<int>(b - buf);
			char_view v(buf, n);
			drop(n + 1); // drop c too

			return v;
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
				char_view v(" \tabc");
				v.eatws();
				assert(v.equal("abc"));
				v.eatws();
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
	inline char_view eatws(char_view v)
	{
		return v.eatws();
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

	// convert to type X from characters
	template<class X>
	inline X to(char_view& v)
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
			assert(v.equal("abc"));
		}
		{
			char_view v("1.23abc");
			auto x = to<double>(v);
			assert(x == 1.23);
			assert(v.equal("abc"));
		}

		return 0;
	}
#endif // _DEBUG
	class splitable {
		char_view v, _v;
		char c, l, r, e;
		void next()
		{
			v = _v.split(c, l, r, e);
		}
	public:
		using iterator_category = std::input_iterator_tag;
		using value_type = char_view;
		using reference = char_view&;
		using pointer = char_view*;
		using difference_type = ptrdiff_t;

		splitable()
		{ }
		splitable(const char_view& v, char c, char l = 0, char r = 0, char e = 0)
			: _v(eatws(v)), c(c), l(l), r(r), e(e)
		{
			next();
		}
		splitable(const splitable&) = default;
		splitable& operator=(const splitable&) = default;
		~splitable()
		{ }

		splitable& take(int n)
		{
			v.take(n);

			return *this;
		}

		auto operator<=>(const splitable&) const = default;

		explicit operator bool() const
		{
			return !!v;
		}
		auto begin() const
		{
			return *this;
		}
		auto end() const
		{
			return splitable(char_view(v.buf + v.len, 0), c, r, l, e);
		}

		char_view operator*() const
		{
			return v;
		}
		splitable& operator++()
		{
			if (v) {
				next();
			}

			return *this;
		}
		splitable operator++(int)
		{
			splitable v_{ *this };

			operator++();

			return v_;
		}
#ifdef _DEBUG

		static int test() 
		{
			{
				char_view v("a,b,c");
				splitable ss(v, ',');
				char a = 'a';
				for (const auto& s : ss) {
					assert(s.len == 1);
					assert(s.front() == a);
					++a;
				}
			}

			return 0;
		}

#endif // _DEBUG
	};

} // namespace fms
