// fms_parse.h - parse contiguous data
// View based parsing. Return parsed value and advance view.
// If len < 0 then view is an error with buf pointing at the error message.
// The view is unchanged if there is an error.
#ifndef FMS_PARSE_INCLUDED
#define FMS_PARSE_INCLUDED

#include <cstdlib>
#include "fms_char_view.h"
#include "fms_parse_split.h"
#include "fms_json.h"
#ifdef _MSC_VER
#include "win_mem_view.h"
#endif

#endif // FMS_PARSE_INCLUDED
