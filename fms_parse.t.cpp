// fms_parse.t.cpp 
#include <cassert>
#include <string>
#include "fms_parse.h"

#ifdef FMS_VIEW_INCLUDED
int test_fms_view_char = fms::view<char>::test();
int test_fms_view_const_char = fms::view<const char>::test();
int test_fms_view_double = fms::view<double>::test();
#endif
#ifdef FMS_CHAR_VIEW_INCLUDED
int test_fms_eat = fms::eat_test<char>();
int test_fms_char_view = fms::char_view<char>::test();
int test_fms_wchar_view = fms::char_view<wchar_t>::test();
#endif
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
