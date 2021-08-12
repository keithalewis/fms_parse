// win_mem_view.t.cpp - test mem_view
#ifdef _WIN32
#include "win_mem_view.h"

int win_mem_veiw_int = win::mem_view<int>::test();
int win_mem_veiw_char = win::mem_view<char>::test();
int win_mem_veiw_double = win::mem_view<double>::test();

#endif // _WIN32