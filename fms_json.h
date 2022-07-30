// fms_json.h - Sample JSON implementation
#ifndef FMS_JSON_INCLUDED
#define FMS_JSON_INCLUDED

#include "fms_parse_json.h"
#ifdef NULL
#undef NULL
#endif

namespace fms::json {

#define FMS_JSON_TYPE(X) X(NULL) X(OBJECT) X(ARRAY) X(STRING) X(NUMBER) X(BOOLEAN)

#define FMS_JSON_ENUM(x) x,
	enum class type {
		FMS_JSON_TYPE(FMS_JSON_ENUM)
	};
#undef FMS_JSON_ENUM

	struct value;
	using member = std::pair<std::string, value>;
	using object = std::map<std::string, value>;
	using array = std::vector<value>;
	using string = std::string;
	using number = double;
	using boolean = bool;
	struct null {
		bool operator==(const null&) const { return true; }
	};

	// JSON value
	struct value : public std::variant<null, object, array, string, number, boolean> {
		using var = std::variant<null, object, array, string, number, boolean>;
		using var::var;
		constexpr value() noexcept
			: var(null{})
		{ }
		constexpr value(const char* s)
			: var{ string(s) }
		{ }
		constexpr value(const char_view<const char>& v)
			: var{ string(v.buf, v.buf + v.len) }
		{ }
		explicit operator std::string& ()
		{
			return std::get<string>(*this);
		}
		explicit operator const std::string& () const
		{
			return std::get<string>(*this);
		}
		explicit constexpr value(const number& n)
			: var{ n }
		{ }
		explicit constexpr value(const int& n)
			: var{ 1.*n }
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

		enum class type type() const
		{
			return fms::json::type(this->index());
		}

		bool operator==(const value& v) const
		{
			if (type() != v.type()) {
				return false;
			}
#define FMS_JSON_CASE(x) case type::x : \
	return std::get<static_cast<std::size_t>(type::x)>(*this) == std::get<static_cast<std::size_t>(type::x)>(v);
			switch (type()) {
				FMS_JSON_TYPE(FMS_JSON_CASE)
			}
#undef FMS_JSON_CASE
			return true;
		}

		// object access
		value& operator[](const string& key)
		{
			return std::get<object>(*this)[key];
		}
		value& operator[](const char* key)
		{
			return std::get<object>(*this)[key];
		}
		const value& operator[](const string& key) const
		{
			return std::get<object>(*this).at(key);
		}
		const value& operator[](const char* key) const
		{
			return std::get<object>(*this).at(key);
		}

		// array access
		value& operator[](std::size_t i)
		{
			return std::get<array>(*this)[i];
		}
		const value& operator[](std::size_t i) const
		{
			return std::get<array>(*this)[i];
		}
	};

#ifdef _DEBUG
	inline int value_test()
	{
		{
			value v;
			assert(type::NULL == v.type());
			auto v2{ v };
			assert(v == v2);
			v = v2;
			assert(!(v2 != v));
		}
		{
			value v(true);
			auto v2{ v };
			assert(v == v2);
			v = v2;
			assert(!(v2 != v));
			assert(type::BOOLEAN == v.type());
			assert(true == std::get<bool>(v));
			assert(true == std::get<(std::size_t)type::BOOLEAN>(v));
		}
		{
			value v(1.);
			auto v2{ v };
			assert(v == v2);
			v = v2;
			assert(!(v2 != v));
			assert(type::NUMBER == v.type());
			assert(1 == std::get<double>(v));
			assert(1 == std::get<number>(v));
		}
		{
			value v(1);
			auto v2{ v };
			assert(v == v2);
			v = v2;
			assert(!(v2 != v));
			assert(type::NUMBER == v.type());
			assert(1 == std::get<double>(v));
			assert(1 == std::get<number>(v));
		}
		{
			value v("string");
			auto v2{ v };
			assert(v == v2);
			v = v2;
			assert(!(v2 != v));
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
			value v(array{ value(false), value(1.2), value("str") });
			auto v2{ v };
			assert(v == v2);
			v = v2;
			assert(!(v2 != v));
			assert(type::ARRAY == v.type());
			assert(!std::get<array>(v)[0]);
			assert(1.2 == std::get<array>(v)[1]);
			assert(std::get<array>(v)[2] == "str");
		}
		{
			value v(object({
				{ "a", value(1.2) },
				{ "b", value(false) },
				{ "c", value(object({{"d", value("foo")}})) },
				{ "e", value(array({value(1), value(true), value("baz")}))}
				}));
			auto va = std::get<object>(v)["a"];
			assert(1.2 == va);
			assert(1.2 == v["a"]);
			v["a"] = "bar";
			assert(v["a"] == "bar");
			assert(v["c"]["d"] == "foo");
			v["c"]["d"] = v;
			assert(v["c"]["d"]["a"] == "bar");
			assert(v["e"][2] == "baz");
		}

		return 0;
	}
#endif // _DEBUG
}

#endif // FMS_JSON_INCLUDED
