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

		view& swap(view& v) noexcept
		{
			std::swap(len, v.len);
			std::swap(buf, v.buf);

			return *this;
		}

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

	struct char_view : public view<const char> {
		// bring in all constructors
		using view<const char>::view;

		template<size_t N>
		char_view(const char(&buf)[N])
			: view(buf, static_cast<int>(N - 1))
		{ }

		char_view& eat(char t)
		{
			if (len and *buf == t) {
				drop(1);
			}
			else {
				len = -1;
				buf = __FUNCTION__ ": indigestion";
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
#ifdef _DEBUG
		static int test()
		{
			{
				char_view v("abc");
				v.eat('a');
				assert(v);
				assert(v.equal(char_view("bc")));
				v.eat('c');
				assert(!v);
			}
			{
				char_view v(" \tabc");
				v.eatws();
				assert(v.equal(char_view("abc")));
				v.eatws();
				assert(v.equal(char_view("abc")));
			}

			return 0;
		}
#endif // _DEBUG
	};

	// noop if *v != t
	inline char_view eat(char_view& v, char t)
	{
		char_view v_(v);

		v_.eat(t);
		if (v_) {
			v = v_;
		}

		return v_ ? v : v_;
	}
	// noop if entire string not eaten
	inline char_view eat(char_view& v, const char* s, int len = 0)
	{
		char_view v_{v};

		if (len) {
			while (v_ and len--) {
				v_.eat(*s++);
			}
		}
		else {
			while (v_ and *s) {
				v_.eat(*s++);
			}
		}

		// ate everything
		if (v_) {
			v = v_;
		}

		return v_ ? v : v_;
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
			assert(eat(v, "ab"));
			assert(v.equal(char_view("c")));
		}
		{
			char_view v("abc");
			assert(!eat(v, "ac"));
			assert(v.equal(char_view("abc")));
		}
		{
			char_view v("abc");
			assert(eat(v, "ac", 1));
			assert(v.equal(char_view("bc")));
		}

		return 0;
	}

#endif // _DEBUG

	// convert to type from characters
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
				assert((*vi).equal(char_view("a")));
				++vi;
				assert((*vi).equal(char_view("{bd}")));
				++vi;
				assert((*vi).equal(char_view("d")));
				++vi;
				assert((*vi).equal(char_view("e")));
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
				assert((*(*i)).equal(char_view("a")));
				++i;
				assert((*(*i)).equal(char_view("b")));
				++i;
				assert((*(*i)).equal(char_view("c")));
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

} // namespace fms::parse