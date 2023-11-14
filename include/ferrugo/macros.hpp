#define RETURN(...)         \
    decltype(__VA_ARGS__)   \
    {                       \
        return __VA_ARGS__; \
    }
