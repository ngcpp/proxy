module;

#include <proxy/proxy_macros.h>

#include <tuple>
#include <type_traits>
#include <utility>

export module foo;

import proxy.v5;

extern "C++" {
PRO_ENABLE_MEMBER(GetFoo);
PRO_ENABLE_FREE(GetQux);
PRO_DEF_FREE_DISPATCH(FreeGetBar, GetBar);
PRO_DEF_FREE_AS_MEM_DISPATCH(FreeGetBaz, GetBaz);
}

export struct Foo
    : pro::facade_builder::add_convention<pro::member_dispatch<"GetFoo">,
                                          int() const>::build {};

export struct Qux
    : pro::facade_builder::add_convention<pro::free_dispatch<"GetQux">,
                                          int() const>::build {};

export struct Bar
    : pro::facade_builder::add_convention<FreeGetBar, int() const>::build {};

export struct Baz
    : pro::facade_builder::add_convention<FreeGetBaz, int() const>::build {};
