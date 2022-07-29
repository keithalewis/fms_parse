// fms_json.h - Sample JSON implementation
#ifndef FMS_JSON_INCLUDED
#define FMS_JSON_INCLUDED

#include "fms_parse_json.h"
#ifdef NULL
#undef NULL
#endif

namespace fms::json {

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
}

#endif // FMS_JSON_INCLUDED
