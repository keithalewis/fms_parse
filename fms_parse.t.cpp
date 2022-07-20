// fms_parse.t.cpp 
#include <cassert>
#include "fms_parse.h"
/*
int fms_eat_test = fms::eat_test<char>();
int fms_parse_to_test = fms::parse::to_test();
int fms_parse_datetime_test = fms::parse::datetime_test();
int fms_parse_splitable_test = fms::parse::splitable<char>::test();
*/
#ifdef FMS_VIEW_INCLUDED
int test_fms_view_char = fms::view<char>::test();
int test_fms_view_const_char = fms::view<const char>::test();
int test_fms_view_double = fms::view<double>::test();
#endif
/*
#ifdef FMS_CHAR_VIEW_INCLUDED
int fms_char_view_test = fms::char_view<char>::test();
int fms_wchar_view_test = fms::char_view<wchar_t>::test();
#endif
#ifdef WIN_MEM_VIEW_INCLUDED
int win_mem_veiw_int = win::mem_view<int>::test();
int win_mem_veiw_char = win::mem_view<char>::test();
int win_mem_veiw_double = win::mem_view<double>::test();
#endif
*/

int main()
{
	return 0;
}
