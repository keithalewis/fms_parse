// fms_parse_split.h - split iterator
#ifndef FMS_PARSE_SPLIT_INCLUDED
#define FMS_PARSE_SPLIT_INCLUDED

namespace fms::parse {

	// return view up to c that is not quoted and advance v
	template<class T>
	inline char_view<T> split(char_view<T>& v, T c, T l, T r, T e)
	{
		char_view<T> v_{ v };

		while (v_ and *v_ != c) {
			if (*v_ == l) {
				int level = 1;
				while (++v_ and level) {
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
				v_.wstrim();
			}
			v = split<T>(v_, c, l, r, e);
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
			: v_(v), c(c), l(l), r(r), e(e)
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

#endif // _DEBUG
	};

} // namespace fms::parse

#endif // FMS_PARSE_SPLIT_INCLUDED
