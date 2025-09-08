// fms_parse.t.cpp 
#include <cassert>
#include <string>
#include "fms_parse.h"
#include "fms_json.h"

int test_fms_view_char = fms::view<char>::test();
int test_fms_view_const_char = fms::view<const char>::test();
//int test_fms_view_double = fms::view<double>::test();

int test_fms_eat = fms::eat_test<char>();
int test_fms_char_view = fms::char_view<char>::test();
int test_fms_wchar_view = fms::char_view<wchar_t>::test();

//int test_fms_json_value = fms::json::value_test();
int test_fms_json_eat_chars = fms::json::eat_chars_test();
int test_fms_json_parse_number = fms::json::parse_number_test();
int test_fms_json_parse_string = fms::json::parse_string_test();
#ifdef WIN_MEM_VIEW_INCLUDED
int test_win_mem_veiw_int = win::mem_view<int>::test();
int test_win_mem_veiw_char = win::mem_view<char>::test();
int test_win_mem_veiw_double = win::mem_view<double>::test();
#endif
#ifdef FMS_PARSE_SPLIT_INCLUDED
int test_fms_parse_splitable = fms::parse::splitable<char>::test();
#endif

int main()
{
	return 0;
}
