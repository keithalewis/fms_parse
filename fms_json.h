// fms_json.h - Sample JSON implementation
#ifndef FMS_JSON_INCLUDED
#define FMS_JSON_INCLUDED
#include <string>
#include <vector>
#include "fms_parse_json.h"

namespace fms::json {

	using Null = std::monostate;
	using True = std::true_type;
	using False = std::false_type;
	using Number = double;
	using String = std::string;

	template<class Value>
	using Member = std::pair<String, Value>;

	template<class Value>
	using Object = std::vector<Member<Value>>;

	template<class Value>
	using Array = std::vector<Value>;

	template<class JSON>
	using Value = std::variant<Null, True, False, Number, String, Object<JSON>, Array<JSON>>;

}

#endif // FMS_JSON_INCLUDED
