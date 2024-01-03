// testnetbox_06.cpp
// How to run:
// testnetbox_06.exe dyncast01
//------------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <time.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <functional>
#include <type_traits>

#include "testutils.h"

#include <UnicodeString.hpp>
#include <GUIConfiguration.h>
#include <CoreMain.h>

#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_CPP11_NO_SHUFFLE
#include <catch/catch.hpp>

// stubs
/*
bool AppendExceptionStackTraceAndForget(TStrings *& MoreMessages)
{
  return false;
}

TCustomFarPlugin * FarPlugin = nullptr;
*/
/*******************************************************************************
            test suite
*******************************************************************************/

class base_fixture_t
{
public:
};

TEST_CASE_METHOD(base_fixture_t, "testUnicodeString01", "netbox")
{
  SECTION("UnicodeString01")
  {
    UnicodeString a("1");
    CHECK(a == "1");
    CHECK(a != "2");
    CHECK(L"1" == a);
    CHECK(L"2" != a);
  }
  SECTION("UTF8String01")
  {
    UTF8String a("ab");
    CHECK(a == L"ab");
    CHECK(a != L"abc");
    CHECK(L"ab" == a);
    CHECK(L"abc" != a);
  }
  SECTION("AnsiString01")
  {
    AnsiString a("ab");
    CHECK(a == "ab");
    CHECK(a != "abc");
    CHECK("ab" == a);
    CHECK("abc" != a);
  }
  SECTION("RawByteString01")
  {
    RawByteString a("ab");
    CHECK(a == "ab");
    CHECK(a != "abc");
    CHECK(L"ab" == a);
    CHECK(L"abc" != a);
  }
}

TEST_CASE_METHOD(base_fixture_t, "testUnicodeString02", "netbox")
{
  UnicodeString a("1");
  UnicodeString b(a);
  a.Unique();
  CHECK(a == "1");
  a = "2";
  b.Unique();
  CHECK(b == "1");
  b = "3";
  CHECK(a == "2");
  CHECK(b == "3");
}

TEST_CASE_METHOD(base_fixture_t, "testIsA01", "netbox")
{
  TObject obj1;
  TPersistent per1;
  SECTION("isa")
  {
    CHECK(rtti::isa<TObject>(&obj1));
    CHECK(rtti::isa<TObject>(&per1));
    CHECK(rtti::isa<TPersistent>(&per1));
    CHECK(!rtti::isa<TPersistent>(&obj1));
  }
  SECTION("dyn_cast")
  {
    TObject *obj2 = rtti::dyn_cast_or_null<TObject>(&obj1);
    CHECK(obj2 != nullptr);
    TPersistent *per2 = rtti::dyn_cast_or_null<TPersistent>(&per1);
    CHECK(per2 != nullptr);
    TObject *obj3 = rtti::dyn_cast_or_null<TObject>(per2);
    CHECK(obj3 != nullptr);
    TPersistent *per3 = rtti::dyn_cast_or_null<TPersistent>(obj3);
    CHECK(per3 != nullptr);
    TPersistent *per4 = rtti::dyn_cast_or_null<TPersistent>(&obj1);
    CHECK(per4 == nullptr);
  }
}

TEST_CASE_METHOD(base_fixture_t, "tryfinally01", "netbox")
{
  SECTION("nothrows")
  {
    int a = 1;
    INFO("before try__finally");
    // printf("a = %d\n", a);
    try__finally
    {
      INFO("in try__finally");
      a = 2;
      // printf("a = %d\n", a);
      // throw std::runtime_error("error in try block");
    }
    __finally
    {
      INFO("in __finally");
      a = 3;
    } end_try__finally
    INFO("after try__finally");
    // printf("a = %d\n", a);
    CHECK(a == 3);
  }
  SECTION("throws")
  {
    int a = 1;
    {
      auto throws = [&]()
        try__finally
        {
          INFO("in try__finally");
          a = 2;
          // printf("a = %d\n", a);
          throw std::runtime_error("error in try block");
        }
        __finally
        {
          INFO("in __finally");
          a = 3;
        } end_try__finally;
      REQUIRE_THROWS_AS(throws(), std::runtime_error);
    }
    INFO("after try__finally");
    // printf("a = %d\n", a);
    CHECK(a == 3);
  }
  SECTION("throws2")
  {
    int a = 1;
    try
    {
      try__finally
      {
        INFO("in try__finally");
        a = 2;
        // printf("a = %d\n", a);
        throw std::runtime_error("error in try block");
      }
      __finally
      {
        INFO("in __finally");
        a = 3;
      } end_try__finally
    }
    catch(std::runtime_error &)
    {
      INFO("in catch");
    }
    // printf("a = %d\n", a);
    CHECK(a == 3);
  }
}

namespace ro2 {

template <typename T, typename Owner>
class ROProperty2
{
CUSTOM_MEM_ALLOCATION_IMPL
private:
//  typedef fu2::function<T() const> TGetValueFunctor;
//  typedef fastdelegate::FastDelegate0<T> TGetValueFunctor;
  using TGetValueFunctor = TransientFunction<T(Owner* owner)>; // 16 bytes
  Owner* owner_;
  TGetValueFunctor getter_;
public:
  ROProperty2() = delete;
  ROProperty2(Owner* owner, const TGetValueFunctor& Getter) :
    owner_(owner),
    getter_(Getter)
  {
    Expects(owner_ != nullptr);
    Expects(getter_.m_Target != nullptr);
  }
  ROProperty2(const ROProperty2&) = default;
  ROProperty2(ROProperty2&&) noexcept = default;
  ROProperty2& operator=(const ROProperty2&) = default;
  ROProperty2& operator=(ROProperty2&&) noexcept = default;
//  ROProperty2(const T& in) : data(in) {}
//  ROProperty2(T&& in) : data(std::forward<T>(in)) {}
  constexpr T operator()() const
  {
    return getter_(owner_);
  }
  constexpr operator T() const
  {
    return getter_(owner_);
  }
  constexpr const T operator->() const
  {
    return getter_(owner_);
  }
  T operator->()
  {
    return getter_(owner_);
  }
  constexpr decltype(auto) operator*() const { return *getter_(owner_); }

  friend bool inline operator==(const ROProperty2 &lhs, const ROProperty2 &rhs)
  {
    return lhs.getter_(lhs.owner_) == rhs.getter_(rhs.owner_);
  }
  friend bool inline operator==(const ROProperty2 &lhs, const T &rhs)
  {
    return lhs.getter_(lhs.owner_) == rhs;
  }
  friend bool inline operator!=(const ROProperty2 &lhs, const ROProperty2 &rhs)
  {
    return lhs.getter_(lhs.owner_) != rhs.getter_(rhs.owner_);
  }
  friend bool inline operator!=(ROProperty2 &lhs, const T &rhs)
  {
    return lhs.getter_(lhs.owner_) != rhs;
  }
};

} // namespace ro2

namespace prop_01 {

template <typename T, typename Owner>
class ROProperty2
{
CUSTOM_MEM_ALLOCATION_IMPL
private:
//  typedef fu2::function<T() const> TGetValueFunctor;
//  typedef fastdelegate::FastDelegate1<T, Owner*> TGetValueFunctor;
  using TGetValueFunctor = TransientFunction<T(Owner* owner)>; // 16 bytes
  Owner* owner_;
  TGetValueFunctor getter_;
public:
  ROProperty2() = delete;
  explicit ROProperty2(Owner* owner, TGetValueFunctor&& Getter) :
    owner_(owner),
    getter_(Getter)
  {
    Expects(owner_ != nullptr);
    Expects(getter_.m_Target != nullptr);
  }
  ROProperty2(const ROProperty2&) = default;
  // ROProperty2(ROProperty2&&) noexcept = default;
  // ROProperty2& operator=(const ROProperty2&) = default;
  ROProperty2& operator=(ROProperty2&&) noexcept = default;
//  ROProperty2(const T& in) : data(in) {}
//  ROProperty2(T&& in) : data(std::forward<T>(in)) {}
  constexpr T operator()() const
  {
    return getter_(owner_);
  }
  constexpr operator T() const
  {
    return getter_(owner_);
  }
  constexpr const T operator->() const
  {
    return getter_(owner_);
  }
  T operator->()
  {
    return getter_(owner_);
  }
  constexpr decltype(auto) operator*() const { return *getter_(owner_); }

  friend bool inline operator==(const ROProperty2 &lhs, const ROProperty2 &rhs)
  {
    return lhs.getter_(lhs.owner_) == rhs.getter_(rhs.owner_);
  }
  friend bool inline operator==(const ROProperty2 &lhs, const T &rhs)
  {
    return lhs.getter_(lhs.owner_) == rhs;
  }
  friend bool inline operator!=(const ROProperty2 &lhs, const ROProperty2 &rhs)
  {
    return lhs.getter_(lhs.owner_) != rhs.getter_(rhs.owner_);
  }
  friend bool inline operator!=(ROProperty2 &lhs, const T &rhs)
  {
    return lhs.getter_(lhs.owner_) != rhs;
  }
};

} // namespace prop_01

class TBase
{
public:
  // prop_01::ROProperty2<int, TBase> Data{ this, [](TBase* self)->int { return self->GetData(); } };
  ROProperty<int> Data{nb::bind(&TBase::GetData, this) };
//  prop_01::ROProperty2<int, TBase> Data{ this, nb::bind([](TBase* self)->int { return self->GetData(); }, this); };
//  ROProperty<int> Data2{ fastdelegate::FastDelegate0<int>(std::bind([this]()->int { return FData; }) ) };
//  ROProperty2<int, TBase> Data2{ this, [](TBase* self)->int { return self->FData; } };
//  ROProperty2<int, TBase> Data2_1{ this, nb::bind(&TBase::GetData, this) };
//  ROProperty2<bool, TBase> AutoSort{ this, [](TBase* self)->bool { return self->FAutoSort; } };
//  ROProperty2<UnicodeString, TBase> Data3{ this, [](TBase* self)->UnicodeString { return self->FString; } };

  void Modify()
  {
    FData = 2;
    FString = "43";
    FAutoSort = false;
  }
protected:
  virtual int GetData() const { return FData; }
private:
  int FData = 1;
  UnicodeString FString = "42";
  bool FAutoSort = true;
};

//template<int s> struct Wow;
//Wow<sizeof(TBase)> wow;

class TDerived : public TBase
{
protected:
  virtual int GetData() const override { return FData; }
private:
  int FData = 3;
};

TEST_CASE_METHOD(base_fixture_t, "properties01", "netbox")
{
  SECTION("TBase")
  {
    TBase obj1;
    CHECK(obj1.Data == 1);
    /*CHECK(obj1.Data2 == 1);
    bool x = obj1.AutoSort;
    CHECK(x == true);
    CHECK(obj1.AutoSort == true);
    CHECK(true == obj1.AutoSort);
    CHECK("42" == obj1.Data3);
    CHECK(obj1.Data3() == "42");
    obj1.Modify();
    CHECK(obj1.AutoSort == false);
    CHECK(false == obj1.AutoSort);
    CHECK("43" == obj1.Data3);
    CHECK(obj1.Data3() == "43");*/
  }
  SECTION("TDerived")
  {
    TDerived d1;
    CHECK(d1.Data == 3);
    CHECK(3 == d1.Data);
    // CHECK(1 == d1.Data2);
    d1.Modify();
    CHECK(3 == d1.Data);
    // CHECK(2 == d1.Data2);
  }
}

/*class TBase2
{
public:
  RWProperty<int> Data{ nb::bind(&TBase2::GetData, this), nb::bind(&TBase2::SetData, this) };
  RWProperty<int> Data1{ [&]()->int { return GetData(); }, [&](int Value) { SetData(Value); } };
  RWProperty<int> Data2{ [&]()->int { return FData; }, [&](int Value) { FData = Value; } };
  RWProperty<bool> AutoSort{ [&]()->bool { return FAutoSort; }, [&](bool Value) { FAutoSort = Value; } };
  RWProperty<UnicodeString> Data3{ [&]()->UnicodeString { return FString; }, [&](const UnicodeString Value) { FString = Value; } };

  RWProperty<int> Data4{ nb::bind(&TBase2::GetData, this), [&](int Value) { FData = Value; } };
  RWProperty<int> Data4_1{ [&]()->int { return GetData(); }, [&](int Value) { FData = Value; } };
  RWProperty<int> Data5{ [&]()->int { return FData; }, nb::bind(&TBase2::SetData, this) };

  void Modify()
  {
    FData = 2;
    FString = "43";
    FAutoSort = false;
  }
protected:
  virtual int GetData() const { return FData; }
  virtual void SetData(int Value) { FData = Value; }
private:
  int FData = 1;
  UnicodeString FString = "42";
  bool FAutoSort = true;
};*/

//template<int s> struct Wow;
//Wow<sizeof(TBase2)> wow;

TEST_CASE_METHOD(base_fixture_t, "properties02", "netbox")
{
  /*SECTION("TBase2")
  {
    TBase2 obj1;
    CHECK(obj1.Data == 1);
    CHECK(obj1.Data1 == 1);
    obj1.Data = 2;
    CHECK(obj1.Data1 == 2);
    CHECK(obj1.Data == 2);
    CHECK(obj1.Data2 == 2);
    obj1.Data2 = 3;
    CHECK(obj1.Data2 == 3);
    CHECK(3 == obj1.Data2);
    CHECK(true == obj1.AutoSort);
    obj1.AutoSort = false;
    CHECK(false == obj1.AutoSort);
    CHECK("42" == obj1.Data3());
    obj1.Data3 = "43";
    CHECK("43" == obj1.Data3());
  }*/
}

TEST_CASE_METHOD(base_fixture_t, "properties03", "netbox")
{
  /*SECTION("TBase2::Data4")
  {
    TBase2 obj1;
    CHECK(1 == obj1.Data4);
    obj1.Data4 = 4;
    CHECK(4 == obj1.Data4);
    CHECK(4 == obj1.Data4_1);
    obj1.Data4_1 = 41;
    CHECK(41 == obj1.Data4_1);
    CHECK(41 == obj1.Data4);
  }
  SECTION("TBase2::Data5")
  {
    TBase2 obj2;
    CHECK(1 == obj2.Data5);
    obj2.Data5 = 5;
    CHECK(5 == obj2.Data5);
    CHECK(5 == obj2.Data4);
  }*/
}

/*namespace prop_01 {

template <typename T>
class ROProp
{
  using TProp = TransientFunction<const T()>;
  TProp getter_;
public:
  ROProp(const TProp& Getter) : getter_(Getter) {}

  friend bool inline operator==(const ROProp& lhs, const T& rhs)
  {
    return lhs.getter_() == rhs;
  }
  friend bool inline operator==(const T& lhs, const ROProp& rhs)
  {
    return rhs.getter_() == lhs;
  }
};

class TBase1
{
public:
//  ROProp<int> Data1{ nb::bind(&TBase1::GetData1, this) };
//  ROProp<int> Data1{ nb::bind(&TBase1::GetData1, this) };
  ROProp<int> Data2{ [&]() { return GetData1(); } };
private:
  int GetData1() { return 42; }
};

} // namespace prop_01
*/

namespace tf {

template<typename>
struct TransientFunction; // intentionally not defined

template<typename R, typename ...Args>
struct TransientFunction<R(Args...)>
{
  using Dispatcher = R(*)(void*, Args...);

  Dispatcher m_Dispatcher{nullptr}; // A pointer to the static function that will call the
                           // wrapped invokable object
  void* m_Target{nullptr}; // A pointer to the invokable object

  // Dispatch() is instantiated by the TransientFunction constructor,
  // which will store a pointer to the function in m_Dispatcher.
  template<typename S, typename ...Args1>
  static R Dispatch(void* target, Args1... args)
  {
    return (*(S*)target)(args...);
  }

  template<typename T>
  TransientFunction(T&& target)
    : m_Dispatcher(&Dispatch<typename std::decay<T>::type>)
    , m_Target(&target)
  {
  }

  // Specialize for reference-to-function, to ensure that a valid pointer is
  // stored.
  using TargetFunctionRef = const R(Args...);
  TransientFunction(TargetFunctionRef target)
    : m_Dispatcher(Dispatch<TargetFunctionRef>)
  {
    static_assert(sizeof(void*) == sizeof(target),
    "It will not be possible to pass functions by reference on this platform. "
    "Please use explicit function pointers i.e. foo(target) -> foo(&target)");
    m_Target = (void*)target;
  }

  template<typename ...Args2>
  R operator()(Args2... args) const
  {
    return m_Dispatcher(m_Target, args...);
  }
};

} // namespace tf

namespace prop_02 {

template <typename T>
class ROProp : tf::TransientFunction<const T()> // 16 bytes
{
  typedef tf::TransientFunction<const T()> base_t;
public:
  using base_t::base_t;

  friend bool inline operator==(const ROProp& lhs, const T& rhs)
  {
    return lhs() == rhs;
  }
  friend bool inline operator==(const T& lhs, const ROProp& rhs)
  {
    return rhs() == lhs;
  }
};

template <typename T>
class WOProp : tf::TransientFunction<void(const T)> // 16 bytes
{
  typedef tf::TransientFunction<void(const T)> base_t;
public:
  using base_t::base_t;

  /*friend bool inline operator==(const WOProp& lhs, const T& rhs)
  {
    return lhs() == rhs;
  }
  friend bool inline operator==(const T& lhs, const WOProp& rhs)
  {
    return rhs() == lhs;
  }*/
  base_t& operator=(const T Value)
  {
    base_t::operator()(Value);
    return *this;
  }
};

template <typename T>
class RWProp // : TransientFunction<const T()>, TransientFunction<void(const T&)> // 32 bytes
{
  typedef TransientFunction<const T()> ro_base_t;
  typedef TransientFunction<void(const T)> rw_base_t;
  ro_base_t getter_;
  rw_base_t setter_;
public:
  RWProp() = delete;
  template<typename T1, typename T2>
  RWProp(T1& getter, T2& setter) : getter_(getter), setter_(setter) {}
  RWProp(const ro_base_t getter, const rw_base_t setter) : getter_(getter), setter_(setter) {}

  void operator=(const T& Value)
  {
    setter_.operator()(Value);
  }

  friend bool inline operator==(const RWProp& lhs, const T& rhs)
  {
    return lhs.getter_() == rhs;
  }
  friend bool inline operator==(const T& lhs, const RWProp& rhs)
  {
    return rhs.getter_() == lhs;
  }
};

class TBase1
{
public:
//  ROProp<int> Data1{ [&]() { return GetData1(); } };
  ROProperty<int> Data1{ nb::bind(&TBase1::GetData1, this) };
//  ROProp<const UnicodeString> StrData1{ [&]() { return GetStrData1(); } };
  ROProperty<const UnicodeString> StrData1{ nb::bind(&TBase1::GetStrData1, this) };
//  ROProp<UnicodeString> StrData2{ [&]() { return GetStrData2(); } };
  ROProperty<UnicodeString> StrData2{ nb::bind(&TBase1::GetStrData2, this) };
//  WOProp<const UnicodeString> WOStrData2{ nb::bind(&TBase1::SetWOStrData2, this) };
//  WOProp<const UnicodeString> WOStrData2{ [&](const UnicodeString Value) { SetWOStrData2(Value); } };
//  RWProp<int> RWData1{ [&]() { return GetRWData1(); }, [&](const int& Value) { SetRWData1(Value); }};
  RWProperty<int> RWData1{ nb::bind(&TBase1::GetRWData1, this), nb::bind(&TBase1::SetRWData1, this) };
//  RWProp<UnicodeString> RWStrData1{ [&]() { return GetRWStrData1(); }, [&](const UnicodeString& Value) { SetRWStrData1(Value); }};
  RWProperty<UnicodeString> RWStrData1{ nb::bind(&TBase1::GetRWStrData1, this), nb::bind(&TBase1::SetRWStrData1, this) };
public:
  void SetWOStrData2(const UnicodeString Value) { FStrData2 = Value; }
  UnicodeString GetWOStrData2() const { return FStrData2; }
private:
  int GetData1() const { return FIntData1; }
  const UnicodeString GetStrData1() const { return FStrData1; }
  UnicodeString GetStrData2() { return FStrData2; }
  int GetRWData1() const { return FIntData1; }
  void SetRWData1(const int & Value) { FIntData1 = Value; }
  UnicodeString GetRWStrData1() { return FStrData1; }
  void SetRWStrData1(const UnicodeString & Value) { FStrData1 = Value; }

  int FIntData1{1};
  UnicodeString FStrData1{"test"};
  UnicodeString FStrData2{"test2"};
};

//template<int s> struct CheckSizeT;
//CheckSizeT<sizeof(ROProp<UnicodeString>)> checkSize;

} // namespace prop_02

TEST_CASE_METHOD(base_fixture_t, "properties04", "netbox")
{
  SECTION("TBase1::Data1")
  {
    prop_02::TBase1 obj;
    {
      bool res = (obj.Data1 == 1);
      CHECK(res);
    }
    {
      bool res = (1 == obj.Data1);
      CHECK(res);
    }
  }
  SECTION("TBase1::StrData1")
  {
    prop_02::TBase1 obj;
    {
      bool res = (obj.StrData1 == "test");
      CHECK(res);
    }
    {
      bool res = (L"test" == obj.StrData1);
      CHECK(res);
    }
  }
  SECTION("TBase1::StrData2")
  {
    prop_02::TBase1 obj;
    {
      bool res = (obj.StrData2 == "test2");
      CHECK(res);
    }
    {
      bool res = (L"test2" == obj.StrData2);
      CHECK(res);
    }
  }
  SECTION("TBase1::WOStrData1")
  {
    prop_02::TBase1 obj;
    {
      bool res = (obj.GetWOStrData2() == "test2");
      CHECK(res);
//      obj.WOStrData2 = UnicodeString("42");
//      res = (obj.GetWOStrData2() == "42");
      CHECK(res);
    }
  }

  SECTION("TBase1::RWData1")
  {
    prop_02::TBase1 obj;
    {
      bool res = (obj.RWData1 == 1);
      CHECK(res);
    }
    {
      bool res = (1 == obj.RWData1);
      CHECK(res);
    }
    {
      obj.RWData1 = 2;
      bool res = (2 == obj.RWData1);
      CHECK(res);
    }
    {
      bool res = (L"test" == obj.RWStrData1);
      CHECK(res);
      obj.RWStrData1 = "42";
      res = (L"42" == obj.RWStrData1);
      CHECK(res);
      res = (obj.RWStrData1 == "42");
      CHECK(res);
    }
  }
  SECTION("TBase1::Data1")
  {
    /*prop_01::TBase1 obj;
    {
      bool res = (obj.Data1 == 42);
      CHECK(res);
    }
    {
      bool res = (42 == obj.Data1);
      CHECK(res);
    }*/
  }
}

namespace prop_03 {

//template<typename>
//struct RWProp; // intentionally not defined

template <typename T>
class RWProp : TransientFunction<const T()>
{
  typedef TransientFunction<const T()> base_t;
public:
  using base_t::base_t;

//  typedef const T (GetterFn)(); // void* target
//  typedef void (SetterFn)(const T& Value); // , void* target
//  GetterFn getter_;
//  SetterFn setter_;
  /*using Dispatcher = T(*)(void*);
  Dispatcher m_Dispatcher{nullptr}; // A pointer to the static function that will call the
                                    // wrapped invokable object
  void* m_Target{nullptr}; // A pointer to the invokable object

  template<typename S>
  static T Dispatch(void* target)
  {
    return (*(S*)target)();
  }*/
public:
//  RWProp() = delete;
  // template<typename T1, typename T2>
//  RWProp(GetterFn getter, SetterFn setter) : getter_(getter), setter_(setter) {}
//  template<typename T1> //, typename T2>
//  explicit RWProp(T1&& t1) : // , T2&& t2) :
//    m_Dispatcher(&Dispatch<typename std::decay<T1>::type>),
//    m_Target(&t1)
//  {
//    /*auto closure = [](void *target) -> const T
//    {
//      return (*(T1*)target)();
//    };*/
////    getter_ = closure;
//  }

  // Specialize for reference-to-function, to ensure that a valid pointer is
  // stored.
//  using TargetFunctionRef = const T(Args...);
//  RWProp(TargetFunctionRef target) :
//    m_Dispatcher(Dispatch<TargetFunctionRef>)
//  {
//    static_assert(sizeof(void*) == sizeof(target),
//    "It will not be possible to pass functions by reference on this platform. "
//    "Please use explicit function pointers i.e. foo(target) -> foo(&target)");
//    m_Target = (void*)target;
//  }

//  T operator()() const
//  {
//    return base_t::operator()();
//  }

  /*void operator=(const T& Value)
  {
    // setter_(Value);
  }*/

  friend bool inline operator==(const RWProp& lhs, const T& rhs)
  {
    return lhs() == rhs;
  }
  friend bool inline operator==(const T& lhs, const RWProp& rhs)
  {
    return rhs() == lhs;
  }
};

class TBase1
{
public:
//  RWProp<int> RWData1{ [&]() { return GetRWData1(); }, [&](int Value) { SetRWData1(Value); } };
  RWProperty<int> RWData1{ nb::bind(&TBase1::GetRWData1, this), nb::bind(&TBase1::SetRWData1, this),  };
//  RWProp<UnicodeString> RWStrData1{ [&]() { return GetRWStrData1(); }, [&](const UnicodeString& Value) { SetRWStrData1(Value); }};
private:
  int GetData1() { return 42; }
  UnicodeString GetStrData1() const { return "42"; }
  UnicodeString GetStrData2() { return "42"; }
  int GetRWData1() const { return FIntData1; }
  void SetRWData1(const int & Value) { FIntData1 = Value; }
  UnicodeString GetRWStrData1() { return FStrData1; }
  void SetRWStrData1(const UnicodeString Value) { FStrData1 = Value; }

  int FIntData1{1};
  UnicodeString FStrData1{"test"};
};

} // namespace prop_03

TEST_CASE_METHOD(base_fixture_t, "properties05", "netbox")
{
  SECTION("TBase1::RWData1")
  {
    prop_03::TBase1 obj;
    {
      bool res = (obj.RWData1 == 1);
      CHECK(res);
    }
    {
      bool res = (1 == obj.RWData1);
      CHECK(res);
    }
  }
}

namespace prop_04 {

// 32 bytes
template <typename T>
class RWProperty
{
CUSTOM_MEM_ALLOCATION_IMPL
private:
//  typedef fu2::function<T() const> TGetValueFunctor;
//  typedef fu2::function<void(T)> TSetValueFunctor;
  typedef fastdelegate::FastDelegate0<const T> TGetValueFunctor;
  typedef fastdelegate::FastDelegate1<void, T> TSetValueFunctor;
//  using TGetValueFunctor = TransientFunction<T()>; // 16 bytes
//  using TSetValueFunctor = TransientFunction<void(T)>; // 16 bytes
  TGetValueFunctor _getter;
  TSetValueFunctor _setter;
public:
  RWProperty() = delete;
  explicit RWProperty(TGetValueFunctor& Getter, TSetValueFunctor& Setter) :
    _getter(Getter),
    _setter(Setter)
  {
//    Expects(_getter.m_Target != nullptr);
//    Expects(_setter.m_Target != nullptr);
  }
  RWProperty(const T&) noexcept = delete;
  RWProperty(const RWProperty&) = default;
  RWProperty(RWProperty&&) noexcept = default;
  RWProperty& operator=(const RWProperty&) = default;
  RWProperty& operator=(RWProperty&&) noexcept = default;
//  RWProperty(const T& in) : data(in) {}
//  RWProperty(T&& in) : data(std::forward<T>(in)) {}
//  T const& get() const {
//      return data;
//  }

//  T&& unwrap() && {
//      return std::move(data);
//  }
  constexpr T operator()() const
  {
    return _getter();
  }
  constexpr operator T() const
  {
    return _getter();
  }
  /*operator T&() const
  {
    Expects(_getter);
    return _getter();
  }*/
  constexpr T operator->() const
  {
    return _getter();
  }
  constexpr decltype(auto) operator*() const { return *_getter(); }
  void operator()(const T &Value)
  {
    _setter(Value);
  }
  void operator=(const T& Value)
  {
    _setter(Value);
  }
//  bool operator==(T Value) const
//  {
//    return _getter() == Value;
//  }
  template <typename T1>
  friend bool inline operator==(const RWProperty& lhs, const T1& rhs)
  {
    return lhs._getter() == rhs;
  }
  template <typename T1>
  friend bool inline operator==(const T1& lhs, const RWProperty<T>& rhs)
  {
    return rhs._getter() == lhs;
  }
  template <typename T1>
  friend bool inline operator!=(const RWProperty<T>& lhs, const T1& rhs)
  {
    return lhs._getter() != rhs;
  }
  template <typename T1>
  friend bool inline operator!=(const T1& lhs, const RWProperty<T>& rhs)
  {
    return rhs._getter() != lhs;
  }
};

class TBase1
{
public:
  RWProperty<int64_t> RWData1{nb::bind(&TBase1::GetRWData1, this), nb::bind(&TBase1::SetRWData1, this)};
  RWProperty<UnicodeString> RWStrData1{nb::bind(&TBase1::GetRWStrData1, this), nb::bind(&TBase1::SetRWStrData1, this)};

private:
  const int64_t GetRWData1() const { return FRWData1; }
  void SetRWData1(int64_t Value) { FRWData1 = Value; }
  const UnicodeString GetRWStrData1() const { return FStrRWData1; }
  void SetRWStrData1(const UnicodeString Value) { FStrRWData1 = Value; }

  int64_t FRWData1{42};
  UnicodeString FStrRWData1{"test"};
};

//template<int s> struct CheckSizeT;
//CheckSizeT<sizeof(prop_04::RWProperty<int64_t>)> checkSize;


} // namespace prop_04

TEST_CASE_METHOD(base_fixture_t, "properties06", "netbox")
{
  SECTION("TBase1::RWData1")
  {
    prop_04::TBase1 obj;
    {
      bool res = (obj.RWData1 == 42);
      CHECK(res);
      res = (42 == obj.RWData1);
      CHECK(res);
    }
    {
      obj.RWData1 = 43;
      bool res = (43 == obj.RWData1);
      CHECK(res);
    }
    {
      prop_04::TBase1 obj2 = obj;
      bool res = (43 == obj2.RWData1);
      CHECK(res);
    }
    {
      prop_04::TBase1 obj2(obj);
      bool res = (43 == obj2.RWData1);
      CHECK(res);
    }
  }
  SECTION("TBase1::RWStrData1")
  {
    prop_04::TBase1 obj;
    bool res;
    {
      res = (obj.RWStrData1 == "test2");
      CHECK(!res);
      res = (obj.RWStrData1() == "test2");
      CHECK(!res);
      res = ("test2" == obj.RWStrData1);
      CHECK(!res);
      res = ("test" != obj.RWStrData1);
      CHECK(!res);
      res = (obj.RWStrData1 != "test");
      CHECK(!res);
      res = ("test" == obj.RWStrData1);
      CHECK(res);
      res = (obj.RWStrData1 == "test");
      CHECK(res);
    }
    {
      res = (obj.RWStrData1() == L"test2");
      CHECK(!res);
      res = (L"test2" == obj.RWStrData1);
      CHECK(!res);
      res = (L"test" != obj.RWStrData1);
      CHECK(!res);
      res = (obj.RWStrData1 != L"test");
      CHECK(!res);
      res = (L"test" == obj.RWStrData1);
      CHECK(res);
      res = (obj.RWStrData1 == L"test");
      CHECK(res);
    }
    {
      UnicodeString str1("test");
      UnicodeString str2("test2");
      bool res = (obj.RWStrData1() == str2);
      CHECK(!res);
      res = (str2 == obj.RWStrData1);
      CHECK(!res);
      res = (str1 != obj.RWStrData1);
      CHECK(!res);
      res = (obj.RWStrData1 != str1);
      CHECK(!res);
      res = (str1 == obj.RWStrData1);
      CHECK(res);
      res = (obj.RWStrData1 == str1);
      CHECK(res);
    }
    {
      obj.RWStrData1 = "43";
      res = ("43" == obj.RWStrData1);
      CHECK(res);
      res = (obj.RWStrData1 == "43");
      CHECK(res);
      res = (obj.RWStrData1 == L"43");
      CHECK(res);
      res = (obj.RWStrData1 == UnicodeString(L"43"));
      CHECK(res);
      res = (obj.RWStrData1 != UnicodeString(L"43"));
      CHECK(!res);
      res = (obj.RWStrData1 != UnicodeString(L"42"));
      CHECK(res);
      res = (obj.RWStrData1 == UnicodeString(L"42"));
      CHECK(!res);
      res = (UnicodeString(L"42") == obj.RWStrData1);
      CHECK(!res);
      res = (UnicodeString(L"431") == obj.RWStrData1);
      CHECK(!res);
    }
    {
      prop_04::TBase1 obj2 = obj;
      bool res = ("43" == obj2.RWStrData1);
      CHECK(res);
    }
    {
      prop_04::TBase1 obj2(obj);
      bool res = ("43" == obj2.RWStrData1);
      CHECK(res);
    }
  }
  auto [A, F] = std::make_pair(1, "42");
  CHECK(A == 1);
  CHECK(strcmp(F, "42") == 0);
}

TEST_CASE_METHOD(base_fixture_t, "strings01", "netbox")
{
  UnicodeString Text = L"text, text text, text text1\ntext text text, text text2\n";
  TStringList Lines;
  Lines.SetCommaText(Text);
  // DEBUG_PRINTF("Lines.GetCount(): %d", Lines.GetCount());
  CHECK(Lines.GetCount() == 5);

  const UnicodeString Instructions = L"Using keyboard authentication.\x0A\x0A\x0APlease enter your password.";
  UnicodeString Instructions2 = ReplaceStrAll(Instructions, L"\x0D\x0A", L"\x01");
  Instructions2 = ReplaceStrAll(Instructions2, L"\x0A\x0D", L"\x01");
  Instructions2 = ReplaceStrAll(Instructions2, L"\x0A", L"\x01");
  Instructions2 = ReplaceStrAll(Instructions2, L"\x0D", L"\x01");
  Instructions2 = ReplaceStrAll(Instructions2, L"\x01", L"\x0D\x0A");
  CHECK(wcscmp(Instructions2.c_str(), UnicodeString(L"Using keyboard authentication.\x0D\x0A\x0D\x0A\x0D\x0APlease enter your password.").c_str()) == 0);

  UTF8String UtfS("123");
  char C = UtfS[1];
  CHECK(C == '1');
}

// #undef DEBUG_PRINTF
template<typename... Args>
UnicodeString debug_printf(const wchar_t * format, Args &&... args)
{
  // UnicodeString Result = nb::Sprintf("Plugin: [%s:%d] %s: ", args...);
  UnicodeString Fmt = format; // TODO: add filename, line info
  UnicodeString Result = fmt::sprintf(Fmt.c_str(), std::forward<Args>(args)...).c_str();
  OutputDebugStringW(Result.c_str());
  return Result;
}

#define DEBUG_PRINTF2(format, ...) OutputDebugStringW(nb::Sprintf("Plugin: [%s:%d] %s: " format L"\n", Sysutils::ExtractFilename(__FILEW__, Backslash), __LINE__, ::MB2W(__FUNCTION__), __VA_ARGS__).c_str())

TEST_CASE_METHOD(base_fixture_t, "debugprintf01", "netbox")
{
  DEBUG_PRINTF2("1");
  debug_printf(L"1: %s", L" ");
  UnicodeString Result = debug_printf(L"1");
  INFO(Result);
  CHECK(Result == "1");
}
