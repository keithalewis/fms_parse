// fms_parse.h - parse contiguous data
// View based parsing. Return parsed value and advance view.
// If len < 0 then view is an error with buf pointing at the error message.
// The view is unchanged if there is an error.
#ifndef FMS_PARSE_INCLUDED
#define FMS_PARSE_INCLUDED

#include "fms_view.h"

#if 0
#ifdef _DEBUG
#include <cassert>
#include <string>
#endif
#include <ctime>
#include <algorithm>
#include <charconv>
#include <iterator>
#include "fms_char_view.h"

namespace fms::parse {

	// convert to type X from characters
	template<class X>
	inline X to(char_view<const char>& v)
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

	using ymd = std::tuple<int, int, int>;

	inline ymd to_ymd(char_view<const char>& v)
	{
		int y, m, d;

		if (v) {
			y = to<int>(v);
			if (v) {
				char c = v.front();
				if (c != '-' and c != '/') {
					v.len = -1;
					v.buf = __FUNCTION__ ": invalid year-month separator";
				}
				else {
					v.eat(c);
					m = to<int>(v);
					if (v) {
						if (v.front() != c) {
							v.len = -1;
							v.buf = __FUNCTION__ ": invalid month-day separator";
						}
						else {
							v.eat(c);
							d = to<int>(v);
						}
					}
				}
			}
		}

		return { y, m, d };
	}

	using hms = std::tuple<int, int, double>;

	inline hms to_hms(char_view<const char>& v)
	{
		int h, m;
		double s;

		if (v) {
			h = to<int>(v);
			if (v) {
				if (v.front() != ':') {
					v.len = -1;
					v.buf = __FUNCTION__ ": invalid hour:minute separator";
				}
				else {
					v.eat(':');
					m = to<int>(v);
					if (v) {
						if (v.front() != ':') {
							v.len = -1;
							v.buf = __FUNCTION__ ": invalid minute:second separator";
						}
						else {
							v.eat(':');
							s = to<double>(v);
						}
					}
				}
			}
		}

		return { h, m, s };
	}

	using off = std::tuple<int, int>;

	inline off to_off(char_view<const char>& v)
	{
		int h = 0, m = 0;

		if (v) {
			char sgn = v.front();

			if (sgn == 'Z') {
				v.eat('Z'); // Zulu
			}
			else {
				if (sgn != '+' and sgn != '-') {
					v.len = -1;
					v.buf = __FUNCTION__ ": offset must start with + or -";
				}
				else {
					v.eat(sgn);
					h = to<int>(v);
					if (v) {
						if (v.front() != ':') {
							v.len = -1;
							v.buf = __FUNCTION__ ": invalid hour:minute offset separator";
						}
						else {
							v.eat(':');
							m = to<int>(v);
						}
					}
				}

				if (sgn == '-') {
					h = -h;
					m = -m;
				}
			}
		}

		return { h, m };
	}

	// ISO 8601 date
	inline std::tuple<ymd, hms, off> to_datetime(char_view<const char>& v)
	{
		ymd ymd;
		hms hms;
		off off;

		if (v) {
			ymd = to_ymd(v);
			if (v) {
				char c = v.front();
				if (c != 'T' and c != ' ') {
					v.len = -1;
					v.buf = __FUNCTION__ ": ymd hms separator must be 'T' or ' '";
				}
				else {
					v.eat(c);
					if (v) {
						hms = to_hms(v);
						if (v) {
							off = to_off(v);
						}
					}
				}
			}
		}

		return { ymd, hms, off };
	}

#ifdef _DEBUG
	inline int datetime_test()
	{
		{
			char_view v("1-2-3");
			auto [y, m, d] = to_ymd(v);
			assert(!v);
			assert(v.len == 0);
			assert(y == 1);
			assert(m == 2);
			assert(d == 3);
		}
		{
			char_view v("1/2/3");
			auto [y, m, d] = to_ymd(v);
			assert(!v);
			assert(v.len == 0);
			assert(y == 1);
			assert(m == 2);
			assert(d == 3);
		}
		{
			char_view v("1/2-3");
			auto [y, m, d] = to_ymd(v);
			assert(!v);
			assert(v.is_error());
		}
		{
			char_view v("1:2:3");
			auto [h, m, s] = to_hms(v);
			assert(!v);
			assert(v.len == 0);
			assert(h == 1);
			assert(m == 2);
			assert(s == 3);
		}
		{
			char_view v("-01:02");
			auto [h, m] = to_off(v);
			assert(!v);
			assert(v.len == 0);
			assert(h == -1);
			assert(m == -2);
		}
		{
			char_view v("2001-01-02T12:34:56.7-01:30");
			auto [ymd, hms, off] = to_datetime(v);
			assert(ymd == std::make_tuple(2001, 1, 2));
			assert(hms == std::make_tuple(12, 34, 56.7));
			assert(off == std::make_tuple(-1, -30));
		}

		return 0;
	}
#endif // _DEBUG

	// return view up to c that is not quoted and advance v
	template<class T>
	inline char_view<T> split(char_view<T>& v, T c, T l, T r, T e)
	{
		char_view<T> _v{ v };

		while (_v and *_v != c) {
			if (*_v == l) {
				int level = 1;
				while (++_v and level) {
					if (*_v == r) {
						--level;
					}
					else if (*_v == l) {
						++level;
					}
					else if (*_v == e) {
						++_v;
					}
				}
				if (level != 0) {
					_v.len = -1;
					//_v.buf points to last char parsed;
				}
			}
			++_v;
		}

		if (!_v.is_error()) {
			int n = static_cast<int>(_v.buf - v.buf);
			std::swap(v.len, _v.len);
			std::swap(v.buf, _v.buf);
			_v.take(n);
			v.drop(1); // drop c
		}

		return _v;
	}

	template<class T>
	class splitable {
		char_view<T> v, _v;
		T c, l, r, e;
		void incr()
		{
			if (!std::isspace(l)) {
				_v.wstrim();
			}
			v = split<T>(_v, c, l, r, e);
			if (!std::isspace(r)) {
				v.trimws();
			}
		}
	public:
		using iterator_category = std::input_iterator_tag;
		using value_type = char_view<T>;
		using reference = char_view<T>&;
		using pointer = char_view<T>*;
		using difference_type = ptrdiff_t;

		splitable()
		{ }
		splitable(const char_view<T>& v, T c, T l = 0, T r = 0, T e = 0)
			: _v(v), c(c), l(l), r(r), e(e)
		{
			incr();
		}
		splitable(const splitable&) = default;
		splitable& operator=(const splitable&) = default;
		~splitable()
		{ }

		// needed ???
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
			return splitable(char_view<T>(_v.buf + _v.len, 0), c, r, l, e);
		}

		value_type operator*() const
		{
			return v;
		}
		splitable& operator++()
		{
			if (v) {
				incr();
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
				char buf[] = "a,b,c";
				char_view v(buf);
				splitable ss(v, ',');
				char a = 'a';
				for (const auto& s : ss) {
					assert(s.len == 1);
					assert(s.front() == a);
					++a;
				}
			}
			{
				char buf[] = " a\t,\rb, c\n";
				char_view v(buf);
				splitable ss(v, ',');
				char a = 'a';
				for (const auto& s : ss) {
					assert(s.len == 1);
					assert(s.front() == a);
					++a;
				}
			}
			{
				char buf[] = "a\tb\tc";
				char_view v(buf);
				splitable ss(v, '\t');
				char a = 'a';
				for (const auto& s : ss) {
					assert(s.len == 1);
					assert(s.front() == a);
					++a;
				}
			}
			{
				char buf[] = "a{,}b,c ";
				char_view v(buf);
				splitable ss(v, ',', '{', '}');
				assert((*ss).equal("a{,}b"));
				++ss;
				assert((*ss).equal("c"));
				++ss;
				assert(!ss);
			}
			{
				char buf[] = "a{\\}}b,c ";
				char_view v(buf);
				splitable ss(v, ',', '{', '}', '\\');
				assert((*ss).equal("a{\\}}b"));
				++ss;
				assert((*ss).equal("c"));
				++ss;
				assert(!ss);
			}
			{
				// csv parsing
				char buf[] = "a,b;c,d";
				char_view v(buf);
				std::string s;
				for (const auto& r : splitable(v, ';')) { // records
					for (const auto& f : splitable(r, ',')) { // fields
						s.append(f.buf, f.len);
						s.append("\t");
					}
					s.append("\n");
				}
				assert(s == "a\tb\t\nc\td\t\n");
			}

			return 0;
		}

#endif // _DEBUG
	};

} // namespace fms::parse
#endif // 0
#endif // FMS_PARSE_INCLUDED
