#include <ferrugo/either.hpp>
#include <ferrugo/index_sequence.hpp>
#include <ferrugo/invoke.hpp>
#include <ferrugo/iterable.hpp>
#include <ferrugo/iterator_interface.hpp>
#include <ferrugo/operators.hpp>
#include <ferrugo/optional.hpp>
#include <ferrugo/range_ref.hpp>
#include <ferrugo/std_ostream.hpp>
#include <ferrugo/type_name.hpp>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <vector>

ferrugo::iterable<int> create_vec()
{
    return std::vector<int>{ 2, 5, 9 };
}

void run()
{
    for (auto x : create_vec())
    {
        std::cout << x << std::endl;
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
