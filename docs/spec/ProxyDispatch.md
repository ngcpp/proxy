# Named requirements: *ProxyDispatch*

A type `D` meets the *ProxyDispatch* requirements of types `T` and  `S` if `S` meets the [*ProxySignature* requirements](ProxySignature.md), and the following expressions are well-formed and have the specified semantics (let `R` be return type of `S`, `Args...` be the argument types of `S`. `args...` denotes values of type `Args...`, `v` denotes a value of type `T`, `cv` denotes a value of type `const T`).

| Expressions | Semantics                                       |
| ----------- | ----------------------------------------------- |
| `D()`       | Creates an object of type `D`, shall not throw. |

| Definitions of `S`            | Expressions                                                  | Semantics                                                    |
| ----------------------------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| `R(Args...)`                  | [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D(), v, std::forward<Args>(args)...)` | Invokes dispatch type `D` with an lvalue reference of type `T` and `args...`, may throw. |
| `R(Args...) noexcept`         | [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D(), v, std::forward<Args>(args)...)` | Invokes dispatch type `D` with an lvalue reference of type `T` and `args...`, shall not throw. |
| `R(Args...) &`                | [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D(), v, std::forward<Args>(args)...)` | Invokes dispatch type `D` with an lvalue reference of type `T` and `args...`, may throw. |
| `R(Args...) & noexcept`       | [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D(), v, std::forward<Args>(args)...)` | Invokes dispatch type `D` with an lvalue reference of type `T` and `args...`, shall not throw. |
| `R(Args...) &&`               | [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D(), std::move(v), std::forward<Args>(args)...)` | Invokes dispatch type `D` with a rvalue reference of type `T` and `args...`, may throw. |
| `R(Args...) && noexcept`      | [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D(), std::move(v), std::forward<Args>(args)...)` | Invokes dispatch type `D` with a rvalue reference of type `T` and `args...`, shall not throw. |
| `R(Args...) const`            | [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D(), cv, std::forward<Args>(args)...)` | Invokes dispatch type `D` with a const reference of type `T` and `args...`, may throw. |
| `R(Args...) const noexcept`   | [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D(), cv, std::forward<Args>(args)...)` | Invokes dispatch type `D` with a const reference of type `T` and `args...`, shall not throw. |
| `R(Args...) const&`           | [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D(), cv, std::forward<Args>(args)...)` | Invokes dispatch type `D` with a const reference of type `T` and `args...`, may throw. |
| `R(Args...) const& noexcept`  | [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D(), cv, std::forward<Args>(args)...)` | Invokes dispatch type `D` with a const reference of type `T` and `args...`, shall not throw. |
| `R(Args...) const&&`          | [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D(), std::move(cv), std::forward<Args>(args)...)` | Invokes dispatch type `D` with a const rvalue reference of type `T` and `args...`, may throw. |
| `R(Args...) const&& noexcept` | [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D(), std::move(cv), std::forward<Args>(args)...)` | Invokes dispatch type `D` with a const rvalue reference of type `T` and `args...`, shall not throw. |

## See Also

- [*ProxyOperation* requirements](ProxyOperation.md)
