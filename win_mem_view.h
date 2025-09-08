// win_mem_view.h - view of memory mapped file
#ifndef WIN_MEM_VIEW_INCLUDED
#define WIN_MEM_VIEW_INCLUDED
#ifdef _DEBUG
#include <cassert>
#include <algorithm>
#include <numeric>
#include <stdexcept>
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
		fms::view<T> init, cur;
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
			if (h != nullptr) {
				len = 0;
				buf = static_cast<T*>(MapViewOfFile(h, FILE_MAP_ALL_ACCESS, 0, 0, len * sizeof(T)));
			}
			else {
				DWORD err = GetLastError();
				char* buf = nullptr;

				if (FormatMessageA(
					FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					nullptr,
					err,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					buf,
					0, nullptr)) 
				{
					std::string msg(buf);
					LocalFree(buf);
					throw std::runtime_error(msg);
				}
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
			return !!*this;
		}

		mem_view& reset()
		{
			return *this = init;
		}
		mem_view& push()
		{
			cur = *this;
			return *this;
		}
		mem_view& pop()
		{
			if (cur) {
				*this = cur;
				cur = fms::view<T>{};
			}
			return *this;
		}

		// Write to buffered memory.
		mem_view& append(const T* s, DWORD n)
		{
			CopyMemory(buf + len, s, n*sizeof(T));
			len += n;

			return *this;
		}
		mem_view& append(const std::initializer_list<T>& il)
		{
			return append(il.begin(), static_cast<DWORD>(il.size()));
		}
#ifdef _DEBUG

		static int test()
		{
			{
				mem_view<T> v;
				assert(v.len == 0);
				T t[] = { 1,2,3 };
				v.append(t, 3);
				assert(v.equal({ 1,2,3 }));
			}
			{
				mem_view<T> v;
				assert(v.len == 0);
				v.len = 3;
				std::iota(v.begin(), v.end(), T(1));
				assert(v.equal({ 1,2,3 }));
			}

			return 0;
		}

#endif // _DEBUG
	};

} // namespace xll

#endif // WIN_MEM_VIEW_INCLUDED
