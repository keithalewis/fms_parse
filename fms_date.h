// fms_date.h - date functions
#pragma once
#ifdef _DEBUG
#include <cassert>
#endif
#include <tuple>
#include "fms_parse.h"

namespace fms::date {

	using ymd = std::tuple<int, int, int>;

	inline ymd to_ymd(parse::char_view& v)
	{
		int y, m, d;

		if (v) {
			y = parse::to<int>(v);
			if (v) {
				char c = v.front();
				if (c != '-' and c != '/') {
					v.len = -1;
					v.buf = __FUNCTION__ ": invalid year-month separator";
				}
				else {
					v.eat(c);
					m = fms::parse::to<int>(v);
					if (v) {
						if (v.front() != c) {
							v.len = -1;
							v.buf = __FUNCTION__ ": invalid month-day separator";
						}
						else {
							v.eat(c);
							d = fms::parse::to<int>(v);
						}
					}
				}
			}
		}

		return { y, m, d };
	}

	using hms = std::tuple<int, int, double>;

	inline hms to_hms(parse::char_view& v)
	{
		int h, m;
		double s;

		if (v) {
			h = parse::to<int>(v);
			if (v) {
				if (v.front() != ':') {
					v.len = -1;
					v.buf = __FUNCTION__ ": invalid hour:minute separator";
				}
				else {
					v.eat(':');
					m = parse::to<int>(v);
					if (v) {
						if (v.front() != ':') {
							v.len = -1;
							v.buf = __FUNCTION__ ": invalid minute:second separator";
						}
						else {
							v.eat(':');
							s = parse::to<double>(v);
						}
					}
				}
			}
		}

		return { h, m, s };
	}

	using off = std::tuple<int, int>;

	inline off to_off(parse::char_view& v)
	{
		int h = 0, m = 0;

		if (v) {
			char sgn = v.front();

			if (sgn == 'Z') {
				v.eat('Z'); // Zulu
			}
			else {
				if (sgn != '+' and sgn != '-') {
					v.len = -1;
					v.buf = __FUNCTION__ ": offset must start with + or -";
				}
				else {
					v.eat(sgn);
					h = parse::to<int>(v);
					if (v) {
						if (v.front() != ':') {
							v.len = -1;
							v.buf = __FUNCTION__ ": invalid hour:minute offset separator";
						}
						else {
							v.eat(':');
							m = parse::to<int>(v);
						}
					}
				}

				if (sgn == '-') {
					h = -h;
					m = -m;
				}
			}
		}

		return { h, m };
	}

	// ISO 8601 date
	inline std::tuple<ymd, hms, off> to_date(parse::char_view& v)
	{
		ymd ymd;
		hms hms;
		off off;

		if (v) {
			ymd = to_ymd(v);
			if (v) {
				char c = v.front();
				if (c != 'T' and c != ' ') {
					v.len = -1;
					v.buf = __FUNCTION__ ": ymd hms separator must be 'T' or ' '";
				}
				else {
					v.eat(c);
					if (v) {
						hms = to_hms(v);
						if (v) {
							off = to_off(v);
						}
					}
				}
			}
		}

		return { ymd, hms, off };
	}

#ifdef _DEBUG
	inline int test()
	{
		{
			parse::char_view v("1-2-3");
			auto [y, m, d] = to_ymd(v);
			assert(!v);
			assert(v.len == 0);
			assert(y == 1);
			assert(m == 2);
			assert(d == 3);
		}
		{
			parse::char_view v("1/2/3");
			auto [y, m, d] = to_ymd(v);
			assert(!v);
			assert(v.len == 0);
			assert(y == 1);
			assert(m == 2);
			assert(d == 3);
		}
		{
			parse::char_view v("1/2-3");
			auto [y, m, d] = to_ymd(v);
			assert(!v);
			assert(v.is_error());
		}
		{
			parse::char_view v("1:2:3");
			auto [h, m, s] = to_hms(v);
			assert(!v);
			assert(v.len == 0);
			assert(h == 1);
			assert(m == 2);
			assert(s == 3);
		}
		{
			parse::char_view v("-01:02");
			auto [h, m] = to_off(v);
			assert(!v);
			assert(v.len == 0);
			assert(h == -1);
			assert(m == -2);
		}
		{
			parse::char_view v("2001-01-02T12:34:56.7-01:30");
			auto [ymd, hms, off] = to_date(v);
			assert(ymd == std::make_tuple(2001, 1, 2));
			assert(hms == std::make_tuple(12, 34, 56.7));
			assert(off == std::make_tuple(-1, -30));
		}

		return 0;
	}
#endif // _DEBUG

} // namespace fms::date
