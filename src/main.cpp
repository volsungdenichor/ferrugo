#include <ferrugo/either.hpp>
#include <ferrugo/index_sequence.hpp>
#include <ferrugo/invoke.hpp>
#include <ferrugo/iterable.hpp>
#include <ferrugo/iterator_interface.hpp>
#include <ferrugo/operators.hpp>
#include <ferrugo/optional.hpp>
#include <ferrugo/pipeline.hpp>
#include <ferrugo/range_ref.hpp>
#include <ferrugo/seq/filter.hpp>
#include <ferrugo/seq/transform.hpp>
#include <ferrugo/std_ostream.hpp>
#include <ferrugo/type_name.hpp>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <vector>

void run()
{
    using namespace ferrugo;

    static const auto mul_10 = [](int x) { return 10 * x; };
    static const auto str = [](int x) { return std::to_string(x); };
    static const auto is_even = [](int x) { return x % 2 == 0; };

    std::vector<int> v{ 11, 12, 13, 18, 19 };

    const auto pipe = seq::filter(is_even) | seq::transform(mul_10) | seq::transform(str);

    std::cout << type_name(pipe) << std::endl;

    for (auto&& item : v | pipe)
    {
        std::cout << item << std::endl;
    }
    std::cout << "---" << std::endl;

    for (auto&& item : pipe(v))
    {
        std::cout << item << std::endl;
    }
    std::cout << "---" << std::endl;
}

int main(int argc, char const* argv[])
{
    try
    {
        run();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Error: "
                  << "UNKNOWN ERROR" << std::endl;
    }
}
