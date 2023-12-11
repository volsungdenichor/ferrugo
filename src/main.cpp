#include <ferrugo/either.hpp>
#include <ferrugo/index_sequence.hpp>
#include <ferrugo/invoke.hpp>
#include <ferrugo/iterator_interface.hpp>
#include <ferrugo/operators.hpp>
#include <ferrugo/optional.hpp>
#include <ferrugo/owning_range.hpp>
#include <ferrugo/iterator_range.hpp>
#include <ferrugo/range_ref.hpp>
#include <ferrugo/std_ostream.hpp>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <vector>

ferrugo::owning_range<std::vector<int>> create_vec()
{
    static const auto v = std::vector<int>{ 2, 5, 7, -1 };
    return ferrugo::make_owning_range(v);
}

void print(ferrugo::subrange<std::vector<int>> sub)
{
    for (auto item : sub)
    {
        std::cout << item << std::endl;
    }
}

void run()
{
    for (auto item : create_vec())
    {
        std::cout << item << std::endl;
    }

    std::vector<int> v = { 9, 9, 9 };
    print(v);
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
