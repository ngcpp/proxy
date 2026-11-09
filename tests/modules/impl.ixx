export module foo_impl;

export struct MyFoo {
  auto GetFoo() const { return 42; }
};

export int GetQux(const MyFoo&) { return 7; }
