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
    using ferrugo::fn;
    std::vector<int> vect = { 1, 2, 3, 5 };
    const auto f = 10                                  //
                   | fn([](int x) { return x + 1; })   //
                   | fn([](int x) { return 9 * x; })   //
                   | fn([](int x) { return x - 98; })  //
                   | fn([&](int x) -> const int& { return vect[x]; });

    std::cout << f << std::endl;
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
