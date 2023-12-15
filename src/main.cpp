#include <ferrugo/either.hpp>
#include <ferrugo/index_sequence.hpp>
#include <ferrugo/invoke.hpp>
#include <ferrugo/iterable.hpp>
#include <ferrugo/iterator_interface.hpp>
#include <ferrugo/operators.hpp>
#include <ferrugo/optional.hpp>
#include <ferrugo/pipeline.hpp>
#include <ferrugo/range_ref.hpp>
#include <ferrugo/std_ostream.hpp>
#include <ferrugo/type_name.hpp>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <vector>

void run()
{
    std::vector<int> v{ 11, 13, 19 };
    ferrugo::random_access_iterable<int> x{ v };
    for (auto v : x)
    {
        std::cout << v << std::endl;
    }
    for (std::size_t i = 0; i < x.size(); ++i)
    {
        std::cout << i << " " << x[i] << std::endl;
    }
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
