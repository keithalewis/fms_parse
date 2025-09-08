# fms_parse

String parsing.

The class `fms::view` is similar to `std::span` except it always has static extent
with a size that is not part of its type. Instead of `first` and `last` member
functions it has [`take`](./fms_view.h:~:text=T%20take) and 
[`drop`](./fms_view.h:~:text=T%20drop) that take signed integer arguments that are
guaranteed to return a sub-view of the original view.

It also implements const and non-const `operator*` to return a value and reference, respectively,
to the first element of the view, and operator pre and post increment to drop the first element.
It also implements `operator bool() const` to return true if the view is non-empty.
This makes a view _iterable_.

The member function `equal` works like `std::equal` while `operator==()` 
returns true if and only if the views have the same buffer pointer and length.

All member functions are `constexpr ... const noexcept`.

The class `fms::char_view` is a non-owning view of contiguous characters
that publicly inherits from `fms::view<char>` and adds string parsing functions.
Parsing functions take a reference to a view and return a value.
The view is updated to remove the characters used in parsing.
If the value returned is also a view then parse errors are indicated by
the view having a negative length and the buffer points at
the error message.

These conventions allow streamlined code for parsing.
```
	char_view v("123 4.56 foo");

	int i = to<int>(v);
	v || throw std::runtime_error(v.buf);
	assert(i == 123);
	
	v.eat(' ') || throw std::runtime_error(v.buf);
	
	double d = to<double>(v);
	v || throw std::runtime_error(v.buf);
	assert(d == 4.56);
	
	assert(v.equal(" foo"));

	auto e = v.eat('x');
	assert(!e); // indigestion
	assert(v.equal(" foo")); // v is unchanged
```

To split a view on a character that is not quoted or escaped
use `split(c, l, r, e)` where `c` is the character to split on,
`l` is the left quote, `r` is the right quote, and `e` is
the escape character. A view upto the character is
returned and the view is advanced to the next character after that.

```
	char_view v("a,bc,def");
	char_view f = v.split(',');
	assert(f.equal("a"));
	assert(v.equal("bc,def"));
```

The `splitable` class is an iterator over views.
```
	char_view v("a,bc");
	splitable f(v, ',');
	assert((*f).equal("a"));
	++f;
	assert((*f).equal("bc"));
	++f;
	assert(!f);
```

