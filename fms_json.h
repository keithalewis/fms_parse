// fms_json.h - parse JSON https://www.json.org/
#ifndef FMS_JSON_INCLUDED
#define FMS_JSON_INCLUDED

#include <cstdlib>
#include <map>
#include <vector>
#include <variant>
#include "fms_char_view.h"

namespace fms::json {

	template<class T, class V>
	inline V parse(char_view<T>& v)
	{
		v.wstrim();

		if (!v) {
			return V{};
		}
		if (*v == '{') {
			v.eat('{');
			v.wstrim();
			return V(parse_object(v));
		}
		if (*v == '[') {
			return V(parse_array(v));
		}
		if (*v == '"') {
			return V(parse_string(v));
		}
		if (is_null(v)) {
			return V{};
		}
		if (is_true(v)) {
			return V(true);
		}
		if (is_false(v)) {
			return V(false);
		}

		return V(parse_number(v));
	}

#ifdef NULL
#undef NULL
#endif
	enum class type { NULL, OBJECT, ARRAY, STRING, NUMBER, BOOLEAN };

	struct value;
	using member = std::pair<std::string, value>;
	using object = std::map<std::string, value>;
	using array = std::vector<value>;
	using string = std::string;
	using number = double;
	using boolean = bool;
	using null = struct {};

	// JSON value
	struct value : public std::variant<null, object, array, string, number, boolean> {
		using var = std::variant<null, object, array, string, number, boolean>;
		constexpr value() noexcept
			: var(null{})
		{ }
		constexpr value(const object& o) // noexcept(...o... exceptions?)
			: var{ o }
		{ }
		constexpr value(const array& a)
			: var{ a }
		{ }
		constexpr value(const string& s)
			: var{ s }
		{ }
		constexpr value(const char* s)
			: var{ string(s) }
		{ }
		constexpr value(const char_view<const char>& v)
			: var{ string(v.buf, v.buf + v.len) }
		{ }
		bool operator==(const string& s) const
		{
			return type::STRING == type() and std::get<string>(*this) == s;
		}
		bool operator==(const char* s) const
		{
			return type::STRING == type() and std::get<string>(*this) == s;
		}
		explicit operator std::string& ()
		{
			return std::get<string>(*this);
		}
		explicit operator const std::string& () const
		{
			return std::get<string>(*this);
		}
		constexpr value(const number& n)
			: var{ n }
		{ }
		bool operator==(const number& n) const
		{
			return type::NUMBER == type() and std::get<number>(*this) == n;
		}
		explicit operator number& ()
		{
			return std::get<number>(*this);
		}
		explicit operator const number& () const
		{
			return std::get<number>(*this);
		}
		constexpr value(const boolean& b)
			: var{ b }
		{ }
		bool operator==(const bool& b) const
		{
			return type::BOOLEAN == type() and std::get<boolean>(*this) == b;
		}
		explicit operator boolean& ()
		{
			return std::get<boolean>(*this);
		}
		explicit operator const boolean& () const
		{
			return std::get<boolean>(*this);
		}
		value(const value&) = default;
		value(value&&) = default;
		value& operator=(const value&) = default;
		value& operator=(value&&) = default;
		~value() = default;

		enum type type() const
		{
			return fms::json::type(var::index());
		}
	};

#ifdef _DEBUG
	inline int value_test()
	{
		{
			value v;
			assert(type::NULL == v.type());
		}
		{
			value v(true);
			assert(type::BOOLEAN == v.type());
			assert(true == std::get<bool>(v));
			assert(true == std::get<type::BOOLEAN>(v));
		}
		{
			value v(1.);
			assert(type::NUMBER == v.type());
			assert(1 == std::get<double>(v));
			assert(1 == std::get<number>(v));
		}
		{
			value v("string");
			assert(type::STRING == v.type());
			assert(std::get<string>(v) == "string");
		}
		{
			char_view s("string");
			value v(s);
			assert(type::STRING == v.type());
			assert(std::get<string>(v) == "string");
		}
		{
			value v(std::vector{ value(false), value(1.2), value("str") });
			assert(type::ARRAY == v.type());
			assert(!std::get<array>(v)[0]);
			assert(1.2 == std::get<array>(v)[1]);
			assert(std::get<array>(v)[2] == "str");
		}
		{
			value v(std::vector<value>{ false, 1.2, "str" });
		}

		return 0;
	}
#endif // _DEBUG

	// eat characters and ensure whitespace terminated
	template<class T>
	inline bool eat_chars(char_view<T>& v, const char* s, int n)
	{
		char_view<T> v_(v);

		while (v_ and n--) {
			if (*v_++ != *s++) {
				return false;
			}
		}
		if (v_ and !::isspace(*v_)) {
			return false;
		}
		v = v_;

		return true;
	}

	template<class T>
	inline bool is_null(char_view<T>& v)
	{
		return eat_chars(v, "null", 4);
	}

	template<class T>
	inline bool is_true(char_view<T>& v)
	{
		return eat_chars(v, "true", 4);
	}

	template<class T>
	inline bool is_false(char_view<T>& v)
	{
		return eat_chars(v, "false", 5);
	}

#ifdef _DEBUG
	inline int eat_chars_test()
	{
		{
			wchar_t null[] = L"null";
			auto v = char_view(null);
			assert(is_null(v));
			assert(!v);
		}
		{
			wchar_t null[] = L"Null";
			auto v = char_view(null);
			assert(!is_null(v));
			assert(v.equal(null));
		}
		{
			wchar_t null[] = L"null foo";
			auto v = char_view(null);
			assert(is_null(v));
			assert(v.equal(L" foo"));
		}
		{
			wchar_t null[] = L"nullfoo";
			auto v = char_view(null);
			assert(!is_null(v));
			assert(v.equal(L"nullfoo"));
		}

		return 0;
	}
#endif // _DEBUG

	template<class T>
	inline double parse_number(char_view<T>& v)
	{
		static constexpr double NaN = std::numeric_limits<double>::quiet_NaN();
		double sgn = 1, x = NaN;
		char_view<T> v_(v);

		v_.wstrim(); // trim leading, leave trailing whitespace

		auto fraction = [](char_view<T>& v_) {
			std::remove_cv_t<T> d;
			double x = 0, e = 0.1;

			while (v_ and ::isdigit(d = *v_)) {
				x += (d - '0') * e;
				e /= 10;
				++v_;
			}

			return x;
		};
		auto integer = [](char_view<T>& v_) {
			std::remove_cv_t<T> d;
			double x = 0;

			while (v_ and ::isdigit(d = *v_)) {
				x = 10 * x + (d - '0');
				++v_;
			}

			return x;
		};

		if (v_ and *v_ == '-') {
			sgn = -1;
			v_.eat('-');
		}

		if (v_ and *v_ == '0') { // fraction
			v_.drop(1);
			x = sgn*0;
			if (!v_.eat('.')) {
				return NaN;
			}
			x = fraction(v_);
			if (v_ and !::isspace(*v_)) {
				return NaN;
			}
			v = v_;
		}
		else if (v_ and *v_ >= '1' and *v_ <= '9') {
			x = integer(v_);
			if (v_ and *v_ == '.') {
				v_.drop(1);
				x += fraction(v_);
			}
		}
		double e = 1;
		if (v_ and (*v_ == 'e' or *v_ == 'E')) {
			v_.drop(1);
			if (v_ and *v_ == '+') {
				v_.drop(1);
			}
			else if (v_ and *v_ == '-') {
				e = -1;
				v_.drop(1);
			}
		}
		e *= integer(v_);
		if (v_ and !::isspace(*v_)) {
			return NaN;
		}
		v = v_;

		return sgn * x * pow(10, e);
	}

#ifdef _DEBUG

#define PARSE_NUMBER_DATA(X) \
	X(1, true ) \
	X(12, true) \
	X(12.5, true) \
	X(-123, true) \
	X(0.25, true) \
	X(.24, false) \
	X(1.25e2, true) \
	X(1.25E-2, true) \
	X(-1.25E-2, true) \

#define PARSE_NUMBER_TEST(a,b) { char_view v(#a); double x = parse_number(v); assert(!b or a == x); }

	inline int parse_number_test()
	{
		PARSE_NUMBER_DATA(PARSE_NUMBER_TEST);
		{
			char_view v("1x");
			double x = parse_number(v);
			assert(isnan(x));
			assert(v.equal("1x"));
		}
		{
			char_view v("1 x");
			double x = parse_number(v);
			assert(1 == x);
			assert(v.equal(" x"));
		}

		return 0;
	}
#undef PARSE_NUMBER_DATA
#undef PARSE_NUMBER_TEST
#endif // _DEBUG

	// "str\\\"ing" -> str\\\"ing
	template<class T>
	inline char_view<T> parse_string(char_view<T>& v)
	{
		char_view<T> v_(v);
		
		v_.wstrim();
		if (!v_.eat('"')) {
			return char_view<T>(__FUNCTION__ ": string does not start with \"", -1);
		}

		while (v_ and *v_ != '"') {
			if (*v_ == '\\') {
				++v_;
				if (v_ and *v_ == '"') {
					++v_;
				}
			}
			else {
				++v_;
			}
		}
		if (!v_.eat('"')) {
			return char_view<T>(__FUNCTION__ ": string does not end with \"", -1);
		}
		if (v_ and !::isspace(*v_)) {
			return char_view<T>(__FUNCTION__ ": string is not followed by whitespace", -1);
		}

		std::swap(v, v_);
		v_.take(v.buf - v_.buf - 1).drop(1);

		return v_;
	}
#ifdef _DEBUG
	inline int parse_string_test()
	{
		{
			char_view v("\"foo\"");
			auto s = parse_string(v);
			assert(s.equal("foo"));
			assert(!v);
		}
		{
			char_view v("\"f\\\"o\"");
			auto s = parse_string(v);
			assert(s.equal("f\\\"o"));
			assert(!v);
		}
		{
			char_view v("\"f\"o\"");
			auto s = parse_string(v);
			assert(v.equal("\"f\"o\""));
			assert(s.is_error());
		}
		{
			char_view v("\"f\"\to\"");
			auto s = parse_string(v);
			assert(v.equal("\to\""));
			assert(s.equal("f"));
		}
		/*
		{
			value o;
			char_view v("1.23");
			o = parse<value>(v);
		}
		*/

		return 0;
	}
#endif // _DEBUG
	// "string" : value
	template<class T>
	inline member parse_member(char_view<T>& v)
	{
		char_view<T> v_(v);

		auto key = parse_string(v_);
		if (!key.is_error()) {
			v_.wstrim();
			if (v_.eat(':')) {
				v_.wstrim();
				auto val = parse(v_);
				v = v_;
				return { std::string(key.buf, key.buf + key.len), val };
			}
		}

		return member{};
	}
	template<class T>
	inline object parse_object(char_view<T>& v)
	{
		object o;

		o.insert(parse_member(v));
		while (v.wstrim() and *v == ',') {
			v.eat(',');
			o.insert(parse_member(v));
		}

		return o;
	}
	template<class T>
	inline array parse_array(char_view<T>& v)
	{
		return array{};
	}
}

#endif // FMS_JSON_INCLUDED
