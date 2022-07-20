// win_mem_view.h - view of memory mapped file
#ifndef WIN_MEM_VIEW_INCLUDED
#define WIN_MEM_VIEW_INCLUDED
#ifdef _DEBUG
#include <cassert>
#include <algorithm>
#include <numeric>
#endif
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <handleapi.h>
#include "fms_view.h"

namespace win {

	/// <summary>
	/// Memory mapped file class
	/// </summary>
	/// <remarks>
	/// Use operating system features to buffer memory.
	/// Default size is 2^20 = 10^6 = 1MB, but does not
	/// allocate memory until used.
	/// </remarks>
	template<class T>
	class mem_view : public fms::view<T> {
		HANDLE h;
	public:
		using fms::view<T>::buf;
		using fms::view<T>::len;

		/// <summary>
		/// Map file or temporary anonymous memory.
		/// </summary>
		/// <param name="h">optional handle to file</param>
		/// <param name="len">maximum size of buffer</param>
		mem_view(HANDLE h_ = INVALID_HANDLE_VALUE, DWORD len = 1 << 20)
			: h(CreateFileMapping(h_, 0, PAGE_READWRITE, 0, len*sizeof(T), nullptr))
		{
			if (h != NULL) {
				len = 0;
				buf = (T*)MapViewOfFile(h, FILE_MAP_ALL_ACCESS, 0, 0, len * sizeof(T));
			}
		}
		mem_view(const mem_view&) = delete;
		mem_view& operator=(const mem_view&) = delete;
		~mem_view()
		{
			UnmapViewOfFile(buf);
			CloseHandle(h);
		}

		explicit operator bool() const
		{
			return h != NULL;
		}

		// Write to buffered memory.
		mem_view& append(const T* s, DWORD n)
		{
			CopyMemory(buf + len, s, n*sizeof(T));
			len += n;

			return *this;
		}
#ifdef _DEBUG

		static int test()
		{
			{
				mem_view<T> v;
				assert(v.len == 0);
				T t[] = { 1,2,3 };
				v.append(t, 3);
				assert(v.len == 3);
				assert(v[0] == 1);
				assert(v[1] == 2);
				assert(v[2] == 3);
			}
			{
				mem_view<T> v;
				assert(v.len == 0);
				v.len = 3;
				std::iota(v.begin(), v.end(), T(1));
				for (int i = 0; i < 3; ++i) {
					assert(v[i] == T(i) + T(1));
				}
			}

			return 0;
		}

#endif // _DEBUG
	};

} // namespace xll

#endif // WIN_MEM_VIEW_INCLUDED
