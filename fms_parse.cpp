// fms_parse.cpp 
#include "fms_parse.h"

int fms_parse_view_test_ = fms::parse::view<char>::test();
int fms_parse_char_view_test_ = fms::parse::char_view::test();
int fms_parse_eat_test_ = fms::parse::eat_test();
int fms_parse_to_test_ = fms::parse::to_test();
//int fms_parse_token_test_ = fms::parse::token_test();
int fms_parse_skipable_test_ = fms::parse::chopable::test();
int fms_parse_items_test_ = fms::parse::items::test();
int fms_parse_csv_test_ = fms::parse::csv::test();

int main()
{
    return 0;
}
