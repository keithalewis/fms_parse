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
#include <iterator>
#include "fms_char_view.h"

namespace fms::parse {

	/*
	// skip matching left and right delimiters ignoring escaped
	inline int next(const char_view& v, char l = 0, char r = 0, char e = 0)
	{
		int n = 1;

		if (v and v.front() == l) {
			int level = 1;
			while (level and n < v.len) {
				if (v[n] == e) {
					++n;
				}
				else if (v[n] == r) {
					--level;
				}
				else if (v[n] == l) {
					++level;
				}
				++n;
			}
			if (level != 0) {
				n = -1;
			}
		}

		return n;
	}

	class chopable {
		char_view v;
		char l, r, e;
		int n;
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = char_view;
		using reference = char_view&;
		using pointer = char_view*;
		using difference_type = ptrdiff_t;

		chopable()
		{ }
		chopable(const char_view& v, char l = 0, char r = 0, char e = 0)
			: v(eatws(v)), l(l), r(r), e(e), n(next(eatws(v), l, r, e))
		{ }
		chopable(const chopable&) = default;
		chopable& operator=(const chopable&) = default;
		~chopable()
		{ }

		chopable& take(int n)
		{
			v.take(n);

			return *this;
		}

		auto operator<=>(const chopable&) const = default;

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
			return chopable(char_view(v.buf + v.len, 0), r, l, e);
		}

		char_view operator*() const
		{
			char_view v_{ v };

			v_.take(n);

			return v_;
		}
		chopable& operator++()
		{
			if (n > 0 and v) {
				v.drop(n);
				v.eatws();
				n = next(v, l, r, e);
			}

			return *this;
		}
#ifdef _DEBUG
		static int test()
		{
			{
				char_view v("a{bd}de");
				auto vi = chopable(v, '{', '}');
				assert((*vi).equal("a"));
				++vi;
				assert((*vi).equal("{bd}"));
				++vi;
				assert((*vi).equal("d"));
				++vi;
				assert((*vi).equal("e"));
				++vi;
				assert(!vi);
			}
			{
				char_view v("\ta bc\r\nd");
				chopable vi(v);
				char a = 'a';
				for (const auto& i : vi) {
					assert(i.len == 1);
					assert(i.front() == a);
					++a;
				}
			}

			return 0;
		}
#endif // _DEBUG
	};

	// split chopable on separator
	class items {
		chopable v, _v;
		char sep;
		void next()
		{
			v = _v;

			while (_v) {
				if ((*_v).len == 1 and (*_v).front() == sep) {
					break;
				}
				++_v;
			}

			v.take(static_cast<int>((*_v).buf - (*v).buf));

			if (_v) {
				++_v; // skip sep
			}
		}
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = chopable;
		using reference = chopable&;
		using pointer = chopable*;
		using difference_type = ptrdiff_t;

		items()
			: v{}, _v{}, sep(0)
		{ }
		items(const chopable& v, char sep)
			: _v(v), sep(sep)
		{
			next();
		}
		items(const items&) = default;
		items& operator=(const items&) = default;
		~items()
		{ }

		auto operator<=>(const items&) const = default;
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
			return items(_v.end(), sep);
		}

		chopable operator*() const
		{
			return v;
		}
		chopable& operator*()
		{
			return v;
		}
		items& operator++()
		{
			if (v) {
				next();
			}

			return *this;
		}
#ifdef _DEBUG

		static int test()
		{
			{
				char_view v("a,b,c");
				items i(v, ',');
				assert((*(*i)).equal("a"));
				++i;
				assert((*(*i)).equal("b"));
				++i;
				assert((*(*i)).equal("c"));
				++i;
				assert(!i);
			}
			{
				char_view v("a,b,c");
				items is(v, ',');
				char a = 'a';
				for (const auto& i : is) {
					assert(**i == a);
					++a;
				}
			}
			{
				char_view v(" a,\tb\n;\rc, d");
				items is(v, ';');
				std::string s;
				for (const auto& i : is) {
					auto js = items(i, ',');
					for (const auto& j : js) {
						s.append((*j).buf, (*j).len);
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

	class csv {
		chopable v;
		char fs, rs;
	public:
		csv(const chopable& v, char fs, char rs)
			: v(v), fs(fs), rs(rs)
		{ }
		items records() const
		{
			return items(v, rs);
		}
		items fields(const chopable& record) const
		{
			return items(*record, fs);
		}
#ifdef _DEBUG

		static int test()
		{
			{
				char_view v("a,b;c,d");
				csv t(v, ',', ';');
				std::string s;
				for (const auto& record : t.records()) {
					for (const auto& field : t.fields(record)) {
						s.append((*field).buf, (*field).len);
						s.append("\t");
					}
					s.append("\n");
				}
				assert(s == "a\tb\t\nc\td\t\n");
			}
			{
				char_view v("a, \"b c\"; foo, bar");
				csv t(chopable(v, '"', '"'), ',', ';');
				std::string s;
				for (const auto& record : t.records()) {
					for (const auto& field : t.fields(record)) {
						s.append((*field).buf, (*field).len);
						s.append("\t");
					}
					s.append("\n");
				}
			}

			return 0;
		}

#endif // _DEBUG
	};
	*/
} // namespace fms::parse