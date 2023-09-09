// fms_parse_json.h - parse JSON https://www.json.org/
#ifndef FMS_PARSE_JSON_INCLUDED
#define FMS_PARSE_JSON_INCLUDED

#include <cstdlib>
#include <cmath>
//#include <concepts>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include "fms_char_view.h"

namespace fms::json {

	// eat characters and ensure whitespace terminated
	template<class T>
	inline bool eat_chars(char_view<T>& v, const char* s, int n)
	{
		while (v and n--) {
			if (*v++ != *s++) {
				return false;
			}
		}
		if (v and !::isspace(*v)) {
			return false;
		}

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

	// "str\\\"ing" -> str\\\"ing
	template<class T, class String>
	inline String parse_string(char_view<T>& v)
	{
		char_view<T> v_(v);

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
		if (v_ and *v_ == '"') {
			std::swap(v, v_);
			v_.take(static_cast<int>(v.buf - v_.buf));
		}
		// else throw

		return String(v_.buf, v_.len);
	}

	template<class T, class Number>
	inline Number parse_number(char_view<T>& v)
	{
		static constexpr double NaN = std::numeric_limits<double>::quiet_NaN();
		double sgn = 1, x = NaN;

		auto fraction = [](char_view<T>& v) {
			double x = 0, e = 0.1;

			while (v and ::isdigit(*v)) {
				x += (*v - '0') * e;
				e /= 10;
				++v;
			}

			return x;
		};
		auto integer = [](char_view<T>& v) {
			double x = 0;

			while (v and ::isdigit(*v)) {
				x = 10 * x + (*v - '0');
				++v;
			}

			return x;
		};

		v.wstrim(); // trim leading, leave trailing whitespace

		if (v and *v == '-') {
			sgn = -1;
			v.eat('-');
		}

		if (v and *v == '0') { // fraction
			v.drop(1);
			x = sgn * 0; // so "-0" is -0
			if (v.eat('.')) {
				x = fraction(v);
			}
		}
		else if (v and *v >= '1' and *v <= '9') {
			x = integer(v);
			if (v and *v == '.') {
				v.drop(1);
				x += fraction(v);
			}
		}
		// exponent
		double e = 1;
		if (v and ::toupper(*v) == 'E') {
			v.drop(1);
			if (v and *v == '+') {
				v.drop(1);
			}
			else if (v and *v == '-') {
				e = -1;
				v.drop(1);
			}
		}
		e *= integer(v);

		return Number(v and !::isspace(*v) ? NaN : sgn * x * std::pow(10, e));
	}

	template<class T, class String, class Value>
	inline std::pair<String, Value> parse_member(char_view<T>& v)
	{
		String key;
		Value val;

		v.wstrim();
		key = parse_string(v);
		if (v.wstrim() and v.eat(':')) {
			v.wstrim();
			val = parse_value(v);
		}

		return { key, val };
	}

	template<class T, class Object>
	inline Object parse_object(char_view<T>& v)
	{
		Object o;

		o.insert(parse_member(v));
		while (v.wstrim() and *v == ',') {
			v.eat(',');
			o.insert(parse_member(v));
		}

		return o;
	}
	template<class T, class Array>
	inline Array parse_array(char_view<T>& v)
	{
		Array a;

		v.wstrim();
		if (v) {
			a.push_back(parse_value(v));
			while (v.wstrim() and v.eat(',')) {
				a.push_back(parse_value(v));
			}
		}

		return a;
	}

	template<class T, class Value>
	inline Value parse_value(char_view<T>& v)
	{
		Value val;

		v.wstrim();
		if (v) {
			if (*v == '{') {
				v.eat('{');
				val = parse_object(v);
				v.wstrim();
				v.eat('}');
			}
			else if (*v == '[') {
				v.eat('[');
				val = parse_array(v);
				v.wstrim();
				v.eat(']');
			}
			else if (*v == '"') {
				v.eat('"');
				val = parse_string(v);
				v.eat('"');
			}
			else if (is_null(v)) {
				; // default object
			}
			else if (is_true(v)) {
				val = true;
			}
			else if (is_false(v)) {
				val = false;
			}
			else {
				val = parse_number(v);
			}
		}
		// ensure(!v.wstrim());

		return val;
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
			//assert(v.equal(null));
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
			//assert(v.equal(L"nullfoo"));
		}

		return 0;
	}
#endif // _DEBUG


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

#define PARSE_NUMBER_TEST(a,b) { char_view v(#a); double x = parse_number<const char,double>(v); assert(!b or a == x); }

	inline int parse_number_test()
	{
		PARSE_NUMBER_DATA(PARSE_NUMBER_TEST);
		{
			char_view v("1x");
			double x = parse_number<const char, double>(v);
			assert(std::isnan(x));
			assert(v.equal("x"));
		}
		{
			char_view v("1 x");
			double x = parse_number<const char, double>(v);
			assert(1 == x);
			assert(v.equal(" x"));
		}

		return 0;
	}
#undef PARSE_NUMBER_DATA
#undef PARSE_NUMBER_TEST
#endif // _DEBUG

#ifdef _DEBUG
	inline int parse_string_test()
	{
		using string = std::string;
		{
			char_view v("foo\"");
			string s = parse_string<const char,string>(v);
			assert(s==("foo"));
			assert(v and *v == '"');
		}
		{
			char_view v("f\\\"o\"");
			auto s = parse_string<const char, string>(v);
			assert(s==("f\\\"o"));
			assert(v and *v == '"');
		}
		{
			char_view v("f\"o\"");
			string s = parse_string<const char, string>(v);
			assert(v.equal("\"o\""));
			//assert(s.is_error());
		}
		{
			char_view v("f\"\to\"");
			auto s = parse_string<const char, string>(v);
			assert(v.equal("\"\to\""));
			assert(s==("f"));
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
}

#endif // FMS_PARSE_JSON_INCLUDED
