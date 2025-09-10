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
	
#define FMS_JSON_TYPE(X) \
	X(JSON_NULL) \
	X(JSON_OBJECT) \
	X(JSON_ARRAY) \
	X(JSON_STRING) \
	X(JSON_NUMBER) \
	X(JSON_BOOLEAN) \

#define FMS_JSON_ENUM(x) x,
	enum class type {
	FMS_JSON_TYPE(FMS_JSON_ENUM)
};
#undef FMS_JSON_ENUM


	template<class T>
	constexpr char_view<T> parse_null(char_view<T> v)
	{
		return eat(v, "null", 4);
	}

	template<class T>
	constexpr char_view<T> parse_true(char_view<T> v)
	{
		return eat(v, "true", 4);
	}

	template<class T>
	constexpr char_view<T> parse_false(char_view<T> v)
	{
		return eat(v, "false", 5);
	}

	// String must be class with constructor String(const T*, long n)
	// If n < 0 it should construct and error string.
	// "str\\\"ing" -> str\\\"ing
	template<class T, class String>
	constexpr String parse_string(char_view<T>& v)
	{
		char_view<T> v_(v);

		while (v and *v != '"') {
			if (*v == '\\') { // escape "\\\""
				++v;
				if (v and *v == '"') {
					++v;
				}
			}
			else {
				++v;
			}
		}
		v.eat('"');
		v_.take(v_.len - v.len);
		// else throw???

		return String(v_.buf, v_.len);
	}
	template<class T>
	constexpr double parse_integer(char_view<T>& v) 
	{
		double x = 0;

		while (v and is_digit(*v)) {
			x = 10 * x + (*v - '0');
			++v;
		}

		return x;
	}

	// parse ".ddd"
	template<class T>
	constexpr double parse_fraction(char_view<T>& v) 
	{
		double x = 0, e = 0.1;

		while (v and is_digit(*v)) {
			x += (*v - '0') * e;
			e /= 10;
			++v;
		}

		return x;	
	}

	template<class T>
	constexpr double parse_number(char_view<T>& v)
	{
		static constexpr double NaN = std::numeric_limits<double>::quiet_NaN();
		double sgn = 1, x = NaN;

		//!!!v.ws_trim(); // trim leading, leave trailing whitespace

		if (v and *v == '-') {
			sgn = -1;
			v.eat('-');
		}

		if (v and *v == '0') { // fraction
			v.drop(1);
			x = sgn * 0; // so "-0" is -0
			if (v.eat('.')) {
				x = parse_fraction(v);
			}
		}
		else if (v and *v >= '1' and *v <= '9') {
			x = parse_integer(v);
			if (v and *v == '.') {
				v.drop(1);
				x += parse_fraction(v);
			}
		}
		// exponent
		double e = 1;
		if (v and (*v == 'e' or *v == 'E')) {
			v.drop(1);
			if (v and *v == '+') {
				v.drop(1);
			}
			else if (v and *v == '-') {
				e = -1;
				v.drop(1);
			}
		}
		e *= parse_integer(v);
		double num = sgn * x;
		if (e > 0) {
			while (e-- > 0) {
				num *= 10;
			}
		}
		else if (e < 0) {
			while (e++ < 0) {
				num /= 10;
			}
		}	

		return v and !is_space(*v) and *v != ',' ? NaN : num;
	}

	template<class T, class String, class Value>
	constexpr std::pair<String, Value> parse_member(char_view<T>& v)
	{
		String key;
		Value val;

		v.ws_trim();
		key = parse_string(v);
		if (v.ws_trim() and v.eat(':')) {
			v.ws_trim();
			val = parse_value(v);
		}

		return { key, val };
	}

	template<class T, class Object>
	constexpr Object parse_object(char_view<T>& v)
	{
		Object o;

		o.insert(parse_member(v));
		while (v.ws_trim() and *v == ',') {
			v.eat(',');
			o.insert(parse_member(v));
		}

		return o;
	}
	template<class T, class Array>
	constexpr Array parse_array(char_view<T>& v)
	{
		Array a;

		v.ws_trim();
		if (v) {
			a.push_back(parse_value(v));
			while (v.ws_trim() and v.eat(',')) {
				a.push_back(parse_value(v));
			}
		}

		return a;
	}

	template<class T, class Value>
	constexpr Value parse_value(char_view<T>& v)
	{
		Value val;

		v.ws_trim();
		if (v) {
			if (*v == '{') {
				v.eat('{');
				val = parse_object(v);
				v.ws_trim();
				v.eat('}');
			}
			else if (*v == '[') {
				v.eat('[');
				val = parse_array(v);
				v.ws_trim();
				v.eat(']');
			}
			else if (*v == '"') {
				v.eat('"');
				val = parse_string(v);
				v.eat('"');
			}
			else if (parse_null(v)) {
				; // default object
			}
			else if (parse_true(v)) {
				val = true;
			}
			else if (parse_false(v)) {
				val = false;
			}
			else {
				val = parse_number(v);
			}
		}
		// ensure(!v.ws_trim());

		return val;
	}

#ifdef _DEBUG
	inline int eat_chars_test()
	{
		{
			wchar_t null[] = L"null";
			auto v = char_view(null);
			v = parse_null(v);
			assert(v.len == 0);
		}
		{
			wchar_t null[] = L"Null";
			auto v = char_view(null);
			v = parse_null(v);
			assert(v.is_error());
		}
		{
			wchar_t null[] = L"null foo";
			auto v = char_view(null);
			v = parse_null(v);
			assert(v.equal(" foo"));
		}
		{
			wchar_t null[] = L"nullfoo";
			auto v = char_view(null);
			v = parse_null(v);
			assert(v.equal("foo"));
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

#define PARSE_NUMBER_TEST(a,b) { char_view v(#a); double x = parse_number<const char>(v); assert(!b or a == x); }

	inline int parse_number_test()
	{
		PARSE_NUMBER_DATA(PARSE_NUMBER_TEST);
		{
			char_view v("1x");
			double x = parse_number<const char>(v);
			assert(std::isnan(x));
			assert(v.equal("x"));
		}
		{
			char_view v("1 x");
			double x = parse_number<const char>(v);
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
			assert(s==("foo\""));
			assert(!v);
		}
		{
			char_view v("f\\\"o\"");
			auto s = parse_string<const char, string>(v);
			assert(s== "f\\\"o\"");
		}
		{
			char_view v("f\"o\"");
			string s = parse_string<const char, string>(v);
			assert(s == "f\"");
			assert(v.equal("o\""));
		}

		return 0;
	}
#endif // _DEBUG
}

#endif // FMS_PARSE_JSON_INCLUDED
