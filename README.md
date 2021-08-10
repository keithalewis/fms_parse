# fms_parse

String parsing.

The class `fms::char_view` is a non-owning view of contiguous characters.
Parsing functions take a reference to a view and return a value.
The view is updated to remove the characters used in parsing.
If the value returned is also a view then parse errors are indicated by
the view having a negative length and the buffer points at
the error message. In this case the view argument is unmodified.

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

