#pragma once

#if defined(__cplusplus)
#include <stdlib.h>
#include <stdint.h>
#include <exception>
#include <gsl/gsl>
#endif

#ifdef USE_DLMALLOC

#include <dlmalloc/dlmalloc-2.8.6.h>

#if defined(__cplusplus)

#define nb_malloc(size) ::dlcalloc(1, size)
#define nb_calloc(count, size) ::dlcalloc(count, size)
#define nb_realloc(ptr, size) ::dlrealloc(ptr, size)

template<typename T>
void nb_free(const T *ptr) { ::dlfree(reinterpret_cast<void *>(const_cast<T *>(ptr))); }

#else // #if defined(__cplusplus)

#define nb_malloc(size) dlcalloc(1, size)
#define nb_calloc(count, size) dlcalloc(count, size)
#define nb_realloc(ptr, size) dlrealloc(ptr, size)

#define nb_free(ptr) dlfree((void *)(ptr))

#endif // if defined(__cplusplus)

#else // #ifdef USE_DLMALLOC

#if defined(__cplusplus)

#define nb_malloc(size) ::malloc(size)
#define nb_calloc(count, size) ::calloc(count, size)
#define nb_realloc(ptr, size) ::realloc(ptr, size)

template<typename T>
void nb_free(const T *ptr) { ::free(reinterpret_cast<void *>(const_cast<T *>(ptr))); }

#else // #if defined(__cplusplus)

#define nb_malloc(size) malloc(size)
#define nb_calloc(count, size) calloc(count, size)
#define nb_realloc(ptr, size) realloc(ptr, size)

#define nb_free(ptr) free((void *)(ptr))

#endif // if defined(__cplusplus)

#endif // ifdef USE_DLMALLOC

#if defined(_MSC_VER)
#if (_MSC_VER < 1900)

#ifndef noexcept
#define noexcept throw()
#endif

#endif
#endif

#ifndef STRICT
#define STRICT 1
#endif

#if defined(__cplusplus)

namespace nb {

template<typename T>
inline T calloc(size_t count, size_t size) { return static_cast<T>(nb_calloc(count, size)); }
template<typename T>
inline T realloc(T ptr, size_t size) { return static_cast<T>(nb_realloc(ptr, size)); }

inline char *chcalloc(size_t size) { return calloc<char *>(1, size); }
inline wchar_t *wchcalloc(size_t size) { return calloc<wchar_t *>(1, size * sizeof(wchar_t)); }

inline void *operator_new(size_t size)
{
  void *p = nb::calloc<void *>(1, size);
  /*if (!p)
  {
    static std::bad_alloc badalloc;
    throw badalloc;
  }*/
  return p;
}

inline void operator_delete(void *p)
{
  nb_free(p);
}

} // namespace nb

#endif // if defined(__cplusplus)

#ifdef USE_DLMALLOC
/// custom memory allocation
#define DEF_CUSTOM_MEM_ALLOCATION_IMPL            \
  public:                                         \
  void * operator new(size_t sz)                  \
  {                                               \
    return nb::operator_new(sz);                  \
  }                                               \
  void operator delete(void * p, size_t)          \
  {                                               \
    nb::operator_delete(p);                       \
  }                                               \
  void * operator new[](size_t sz)                \
  {                                               \
    return nb::operator_new(sz);                  \
  }                                               \
  void operator delete[](void * p, size_t)        \
  {                                               \
    nb::operator_delete(p);                       \
  }                                               \
  void * operator new(size_t, void * p)           \
  {                                               \
    return p;                                     \
  }                                               \
  void operator delete(void *, void *)            \
  {                                               \
  }                                               \
  void * operator new[](size_t, void * p)         \
  {                                               \
    return p;                                     \
  }                                               \
  void operator delete[](void *, void *)          \
  {                                               \
  }

#ifdef _DEBUG
#define CUSTOM_MEM_ALLOCATION_IMPL DEF_CUSTOM_MEM_ALLOCATION_IMPL \
  void * operator new(size_t sz, const char * /*lpszFileName*/, int /*nLine*/) \
  { \
    return nb::operator_new(sz); \
  } \
  void * operator new[](size_t sz, const char * /*lpszFileName*/, int /*nLine*/) \
  { \
    return nb::operator_new(sz); \
  } \
  void operator delete(void * p, const char * /*lpszFileName*/, int /*nLine*/) \
  { \
    nb::operator_delete(p); \
  } \
  void operator delete[](void * p, const char * /*lpszFileName*/, int /*nLine*/) \
  { \
    nb::operator_delete(p); \
  }
#else
#define CUSTOM_MEM_ALLOCATION_IMPL DEF_CUSTOM_MEM_ALLOCATION_IMPL
#endif // ifdef _DEBUG

#else
#define CUSTOM_MEM_ALLOCATION_IMPL
#endif // ifdef USE_DLMALLOC

#if defined(__cplusplus)

#pragma warning(push)
#pragma warning(disable: 4100) // unreferenced formal parameter

namespace nballoc
{
  inline void destruct(char *) {}
  inline void destruct(wchar_t *) {}
  template <typename T>
  inline void destruct(T * t) { t->~T(); }
} // namespace nballoc

#pragma warning(pop)

template <typename T> struct custom_nballocator_t;

template <> struct custom_nballocator_t<void>
{
public:
  typedef void *pointer;
  typedef const void *const_pointer;
  // reference to void members are impossible.
  typedef void value_type;
  template <class U>
  struct rebind { typedef custom_nballocator_t<U> other; };
};

template <typename T>
struct custom_nballocator_t
{
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef T *pointer;
  typedef const T *const_pointer;
  typedef T &reference;
  typedef const T &const_reference;
  typedef T value_type;

  template <class U> struct rebind { typedef custom_nballocator_t<U> other; };
  inline custom_nballocator_t() noexcept {}
  inline custom_nballocator_t(const custom_nballocator_t &) noexcept {}

  template <class U> custom_nballocator_t(const custom_nballocator_t<U> &) noexcept {}

  ~custom_nballocator_t() noexcept {}

  pointer address(reference x) const { return &x; }
  const_pointer address(const_reference x) const { return &x; }

  pointer allocate(size_type s, void const * = 0)
  {
    if (0 == s)
      return nullptr;
    pointer temp = nb::calloc<pointer>(s, sizeof(T));
#if !defined(__MINGW32__)
    if (temp == nullptr)
      throw std::bad_alloc();
#endif
    return temp;
  }

  void deallocate(pointer p, size_type)
  {
    nb_free(p);
  }

  size_type max_size() const noexcept
  {
    // return std::numeric_limits<size_t>::max() / sizeof(T);
    return size_t(-1) / sizeof(T);
  }

  void construct(pointer p, const T &val)
  {
    new(reinterpret_cast<void *>(p)) T(val);
  }

  void destroy(pointer p)
  {
    nballoc::destruct(p);
  }
};

template <typename T, typename U>
inline bool operator==(const custom_nballocator_t<T> &, const custom_nballocator_t<U> &)
{
  return true;
}

template <typename T, typename U>
inline bool operator!=(const custom_nballocator_t<T> &, const custom_nballocator_t<U> &)
{
  return false;
}

template <typename T>
bool CheckNullOrStructSize(const T *s) { return !s || (s->StructSize >= sizeof(T)); }
template <typename T>
bool CheckStructSize(const T *s) { return s && (s->StructSize >= sizeof(T)); }

#ifdef _DEBUG
#define SELF_TEST(code) \
struct SelfTest {       \
  SelfTest() {          \
  code;                 \
}                       \
} _SelfTest;
#else
#define SELF_TEST(code)
#endif

#define NB_DISABLE_COPY(Class) \
private: \
  Class(const Class &); \
  Class & operator=(const Class &);

#define NB_STATIC_ASSERT(Condition, Message) \
  static_assert(bool(Condition), Message)

#define NB_MAX_PATH (32 * 1024)
#define NPOS static_cast<intptr_t>(-1)

#endif // if defined(__cplusplus)

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif  //_WIN32_WINNT

#ifndef _WIN32_IE
#define _WIN32_IE 0x0501
#endif  //_WIN32_IE

// winnls.h
#ifndef NORM_STOP_ON_NULL
#define NORM_STOP_ON_NULL 0x10000000
#endif

#ifndef HIDESBASE
#define HIDESBASE
#endif

#ifdef __GNUC__
#ifndef nullptr
#define nullptr NULL
#endif
#endif

#if defined(_MSC_VER) && _MSC_VER<1600
#define nullptr NULL
#endif

#ifndef NB_CONCATENATE
#define NB_CONCATENATE_IMPL(s1, s2) s1 ## s2
#define NB_CONCATENATE(s1, s2) NB_CONCATENATE_IMPL(s1, s2)
#endif
// #define __removed / ## /
#define __removed NB_CONCATENATE(/, /)

#undef __property
// #define __property / ## /
#define __property NB_CONCATENATE(/, /)
