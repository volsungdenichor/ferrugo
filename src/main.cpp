
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cuchar>
#include <ferrugo/core/ansi.hpp>
#include <ferrugo/core/arrays/array.hpp>
#include <ferrugo/core/format.hpp>
#include <ferrugo/core/optional.hpp>
#include <ferrugo/core/overloaded.hpp>
#include <ferrugo/core/pipeline.hpp>
#include <ferrugo/core/predicates.hpp>
#include <ferrugo/core/quantities.hpp>
#include <ferrugo/core/ranges.hpp>
#include <ferrugo/core/type_name.hpp>
#include <ferrugo/core/type_traits.hpp>
#include <iostream>
#include <list>
#include <map>
#include <numeric>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <tuple>
#include <variant>
#include <vector>

void loop(
    ferrugo::core::area_t& area,
    std::function<void(ferrugo::core::area_t::mut_ref_type&)> render,
    std::function<void()> update)
{
    for (int i = 0; i < 20; ++i)
    {
        update();
        ferrugo::core::area_t::mut_ref_type ref = area.mut_ref();
        ref.fill(ferrugo::core::glyph_t{});
        render(ref);

        std::stringstream ss;
        ferrugo::core::buffer_t buffer{ ss };
        ferrugo::core::output(area.ref(), buffer);
        std::cout << "\033[2J\033[;H" << ss.str() << std::flush;

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void run()
{
    using namespace ferrugo;

    using namespace ferrugo::core;

    static const auto text = [](area_t::mut_ref_type& r, const location_t& loc, const multibyte_string& text, const glyph_style_t& style)
    {
        for (int i = 0; i < text.size(); ++i)
        {
            r[location_t{ loc.x + i, loc.y }] = glyph_t{ text[i], style };
        }
    };

    area_t area{ { 64, 16 } };
    float x = 0.F;
    loop(
        area,
        [&](area_t::mut_ref_type& r)
        {
            static const auto black = true_color_t{ 0, 0, 0 };

            r.fill(glyph_t{ '.' } | fg(true_color_t{ 64, 0, 0 }) | bg(black));
            const auto s = fg(true_color_t{ 255, 128, 0 }) | bg(black);
            r[{ 0, 1 }] = 'A' | s;
            r[{ 1, 1 }] = 'l' | s;
            r[{ 2, 1 }] = 'a' | s;
            r[{ 0, 2 }] = "ü" | s;
            r[{ 4, 1 }] = "▞" | s;
            r[{ 5, 1 }] = "▚" | s;
            r[{ 4, 2 }] = "▚" | s;
            r[{ 5, 2 }] = "▞" | s;

            text(r, location_t{ 15, 5 }, multibyte_string("⠺⠽⠯⠗ ☢"), s);

            r[{ 2 * (int)x, (int)x }] = '@' | fg(true_color_t{ 255, 0, 0 }) | bg(black);
            r[{ (int)x, r.m_extent.height - 1 - (int)x }] = 'O' | fg(true_color_t{ 255, 255, 0 }) | bg(black);
        },
        [&]() { x += 0.5F; });
}

void print_error()
{
    static const auto print_error = ferrugo::core::println(std::cerr, "Error: {}\n");
    using namespace ferrugo;
    try
    {
        std::rethrow_exception(std::current_exception());
    }
    catch (const std::exception& ex)
    {
        print_error(ex.what());
    }
    catch (...)

    {
        print_error("UNKNOWN ERROR");
    }
}

int main(int argc, char const* argv[])
{
    try
    {
        run();
    }
    catch (...)
    {
        print_error();
    }
}
