#pragma once
#include "arch.h"
#include <objbase.h>
#include <utility>
#include <WinBase.h>
#include <winerror.h>

#include "ttberror.hpp"

class AutoFree {

private:
	template<typename T, class traits>
	class Base {

	private:
		T *m_DataPtr;

		inline Base(void *data) : m_DataPtr(static_cast<T *>(data)) { }

	public:
		inline static Base Alloc(size_t count = 1)
		{
			return traits::alloc(count * sizeof(T));
		}

		inline Base() : m_DataPtr(nullptr) { }

		inline Base(Base &&other)
		{
			if (this != &other)
			{
				m_DataPtr = std::exchange(other.m_DataPtr, nullptr);
			}
		}

		inline Base(const Base &) = delete;

		inline Base &operator =(Base &&other)
		{
			if (this != &other)
			{
				traits::close(m_DataPtr);
				m_DataPtr = std::exchange(other.m_DataPtr, nullptr);
			}

			return *this;
		}

		inline Base &operator =(const Base &) = delete;

		inline T *operator ->()
		{
			return m_DataPtr;
		}

		inline const T *operator ->() const
		{
			return m_DataPtr;
		}

		inline T &operator *()
		{
			return *m_DataPtr;
		}

		inline const T &operator *() const
		{
			return *m_DataPtr;
		}

		inline T &operator [](std::size_t i)
		{
			return m_DataPtr[i];
		}

		inline const T &operator [](std::size_t i) const
		{
			return m_DataPtr[i];
		}

		inline explicit operator bool() const
		{
			return m_DataPtr != nullptr;
		}

		inline T **put()
		{
			traits::close(m_DataPtr);
			return &m_DataPtr;
		}

		inline T *get()
		{
			return m_DataPtr;
		}

		inline const T *get() const
		{
			return m_DataPtr;
		}

		inline ~Base()
		{
			traits::close(m_DataPtr);
		}

	};

	template<bool silent, Error::Level level = Error::Level::Log>
	struct LocalTraits {

		inline static void *alloc(size_t size)
		{
			return LocalAlloc(LPTR, size);
		}

		inline static void close(void *data)
		{
			void *result = LocalFree(data);
			if (result && !silent)
			{
				LastErrorHandle(level, L"Failed to free memory.");
			}
		}

	};

	struct CoTaskMemTraits {

		inline static void *alloc(size_t size)
		{
			return CoTaskMemAlloc(size);
		}

		inline static void close(void *data)
		{
			CoTaskMemFree(data);
		}

	};

	struct GlobalTraits {

		inline static void *alloc(size_t size)
		{
			return GlobalAlloc(GPTR, size);
		}

		inline static void close(void *data)
		{
			void *result = GlobalFree(data);
			if (result)
			{
				LastErrorHandle(Error::Level::Log, L"Failed to free memory.");
			}
		}

	};

public:
	template<typename T = void>
	using CoTaskMem = Base<T, CoTaskMemTraits>;

	template<typename T = void>
	using Local = Base<T, LocalTraits<false>>;

	template<typename T = void>
	using DebugLocal = Base<T, LocalTraits<false, Error::Level::Debug>>;

	// Only use this if you don't want to log it for a very valid reason.
	template<typename T = void>
	using SilentLocal = Base<T, LocalTraits<true>>;

	template<typename T = void>
	using Global = Base<T, GlobalTraits>;

};