#pragma once

#include <memory>
#include <string>

#ifdef __GNUG__
#include <cxxabi.h>

#include <cstdlib>
#include <memory>

inline std::string demangle(const char* name)
{
    int status = -4;
    std::unique_ptr<char, void (*)(void*)> res{ abi::__cxa_demangle(name, NULL, NULL, &status), std::free };
    return (status == 0) ? res.get() : name;
}

#else

inline std::string demangle(const char* name)
{
    return name;
}

#endif  // #ifdef __GNUG__

template <class T>
std::string type_name()
{
    return demangle(typeid(T).name());
}

template <class T>
std::string type_name(const T&)
{
    return type_name<T>();
}
