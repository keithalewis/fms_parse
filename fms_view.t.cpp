// fms_view.t.cpp - test fms::view
#include "fms_view.h"

int fms_view_test_char = fms::view<char>::test();
int fms_view_test_const_char = fms::view<const char>::test();
int fms_view_test_double = fms::view<double>::test();
