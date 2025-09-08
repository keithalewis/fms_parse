// fms_parse_split.h - split iterator
#pragma once
#include <compare>
#include <iterator>
#include "fms_char_view.h"

namespace fms::parse {

	// return view up to c that is not quoted and advance v
	// l, r, e are left, right, and escape characters
	// if l is encountered then parse until r is encountered
	// ignoring c and keeping track of nesting level
	template<class T>
	constexpr char_view<T> split(char_view<T> v, T c, T l, T r, T e = 0)
	{
		char_view<T> v_{ v };

		while (v_ and *v_ and *v_ != c) {
			if (*v_ == l) {
				int level = 1;
				while (++v_ and *v_ and level) {
					if (*v_ == r) {
						--level;
					}
					else if (*v_ == l) {
						++level;
					}
					else if (*v_ == e) {
						++v_;
					}
				}
				if (level != 0) {
					v_.len = -1;
					//v_.buf points to last char parsed;
				}
			}
			++v_;
		}

		if (!v_.is_error()) {
			int n = static_cast<int>(v_.buf - v.buf);
			std::swap(v.len, v_.len);
			std::swap(v.buf, v_.buf);
			v_.take(n);
			v.drop(1); // drop c
		}

		return v_;
	}

	// split iterator
	template<class T>
	class splitable {
		char_view<T> v, v_;
		T c, l, r, e;
		void incr()
		{
			if (!std::isspace(l)) {
				v_.ws_trim();
			}
			v = split<T>(v_, c, l, r, e);
			if (!std::isspace(r)) {
				v.trim_ws();
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
			: v_(v), c(c), l(l), r(r), e(e)
		{
			incr();
		}
		splitable(const splitable&) = default;
		splitable& operator=(const splitable&) = default;
		~splitable()
		{ }

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
			return splitable(char_view<T>(v_.buf + v_.len, 0), c, r, l, e);
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
	};

#endif // _DEBUG
		
		// keep track of number of increments
		template<class I>
		struct counted_iterable {
			I i;
			int n;
			counted_iterable(I i)
				: i(i), n(0)
			{ }

			bool operator==(counted_iterable&) const
			{
				return n == n and i == i;
			}

			explicit operator bool() const
			{
				return i;
			}
			std::iter_value_t<I> operator*() const
			{
				return *i;
			}
			counted_iterable& operator++()
			{
				if (i) {
					++i;
					++n;
				}
		
				return *this;
			}
			counted_iterable operator++(int)
			{
				counted_iterable tmp{ *this };

				operator++();

				return tmp;
			}
		};

		// At most n elements of i.
		template<class I>
		class finite_iterable {
			I i;
			size_t n;
		public:
			finite_iterable(I i, size_t n)
				: i(i), n(n)
			{ }

			bool operator==(finite_iterable&) const
			{
				return n == n and i == i;
			}

			explicit operator bool() const
			{
				return n and i;
			}
			std::iter_value_t<I> operator*() const
			{
				return *i;
			}
			finite_iterable& operator++()
			{
				if (n and i) {
					++i;
					--n;
				}

				return *this;
			}
			finite_iterable operator++(int)
			{
				finite_iterable tmp{ *this };

				operator++();

				return tmp;
			}
		};
		// Return finite_iterable of v split by c, l, r, and e.
		/*
		template<class I, class T>
		class spliterable {
			I buf;
			int len;
			T c, l, r, e;
			
			I escape(I i)
			{
				if (i and *i == e) {
					++i;
					if (i and (*i == c or *i == l or *i == r or *i == e)) {
						++i;
					}
				}

				return i;
			}
			constexpr finite_iterable<I> split()
			{
				counted_iterable<I> i = buf;
				
				while(i and *i != c) {
					i = escape(i);
					if (*i == l) {
						int level = 1;
						while (++i and i and level) {
							i = escape(i);
							if (*i == r) {
								--level;
							}
							else if (*i == l) {
								++level;
							}
						}
						if (level != 0) {
							len = -1;
							// i points to last char parsed;
						}
					}
					++i;
				}

				return finite_iterable<I>(buf, i.n);
			}
		public:
			using value_type = finite_iterable<I>;

			spliterable(I i, T c, T l, T r, T e)
				: buf(i), len(0), c(c), l(l), r(r), e(e)
			{ }
			spliterable(const spliterable&) = default;
			spliterable& operator=(const spliterable&) = default;
			~spliterable()
			{ }

			bool operator==(const spliterable& s) const = default;
		};
		*/

} // namespace fms::parse
