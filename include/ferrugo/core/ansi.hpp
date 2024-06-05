#pragma once

#include <cstdint>
#include <ferrugo/core/format_utils.hpp>
#include <ferrugo/core/overloaded.hpp>
#include <ferrugo/core/type_traits.hpp>
#include <functional>
#include <iostream>
#include <map>
#include <variant>
#include <vector>

namespace ferrugo
{
namespace core
{

struct multibyte
{
    std::array<char, 4> m_data;
    std::size_t m_size;

    multibyte() : m_data{}, m_size{ 0 }
    {
    }

    multibyte(char32_t ch) : m_data{}, m_size{}
    {
        auto state = std::mbstate_t{};
        m_size = std::c32rtomb(m_data.data(), ch, &state);
        if (m_size == std::size_t(-1))
            throw std::runtime_error{ "u32_to_mb: error in conversion" };
    }

    multibyte(const char* b, const char* e) : m_data{}, m_size{ std::size_t(e - b) }
    {
        assert(m_size <= 4);
        std::copy(b, e, m_data.begin());
    }

    operator char32_t() const
    {
        auto result = char32_t{};
        auto mb_state = std::mbstate_t{};
        auto const error = std::mbrtoc32(&result, m_data.data(), 4, &mb_state);
        if (error == std::size_t(-1))
            throw std::runtime_error{ "mb_to_u32: bad byte sequence" };
        if (error == std::size_t(-2))
            throw std::runtime_error{ "mb_to_u32: incomplete byte sequence" };
        return result;
    }

    std::size_t size() const
    {
        return m_size;
    }

    const char* begin() const
    {
        return m_data.data();
    }

    const char* end() const
    {
        return begin() + size();
    }

    friend std::ostream& operator<<(std::ostream& os, const multibyte& item)
    {
        for (std::size_t i = 0; i < item.m_size; ++i)
        {
            os << item.m_data[i];
        }
        return os;
    }
};

struct multibyte_string : private std::vector<multibyte>
{
    using base_type = std::vector<multibyte>;

    using base_type::empty;
    using base_type::size;
    using base_type::operator[];
    using base_type::at;
    using base_type::begin;
    using base_type::end;

    multibyte_string(const std::string& str)
    {
        std::setlocale(LC_ALL, "en_US.utf8");
        char32_t c32;
        const char* ptr = str.data();
        const char* end = str.data() + str.size();
        std::mbstate_t state{};
        while (std::size_t rc = std::mbrtoc32(&c32, ptr, end - ptr, &state))
        {
            assert(rc != (std::size_t)-3);
            if (rc == (std::size_t)-1)
                break;
            if (rc == (std::size_t)-2)
                break;
            this->emplace_back(ptr, ptr + rc);
            ptr += rc;
        }
    }

    operator std::string() const
    {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

    friend std::ostream& operator<<(std::ostream& os, const multibyte_string& item)
    {
        for (const auto& mb : item)
        {
            os << mb;
        }
        return os;
    }
};

enum class mode
{
    none = 0,
    standout = 1,
    bold = 2,
    dim = 4,
    italic = 8,
    underline = 16,
    blink = 32,
    inverse = 64,
    hidden = 128,
    crossed_out = 256,
    double_underline = 512,
};

template <class E, class = std::enable_if_t<std::is_enum_v<E>>>
class bitmask
{
public:
    using enum_type = E;
    using underlying_type = std::underlying_type_t<E>;

    bitmask() : m_value{ 0 }
    {
    }

    bitmask(enum_type v) : m_value{ to_underlying(v) }
    {
    }

    bool contains(bitmask b) const
    {
        return (m_value & b.m_value) != 0;
    }

    bitmask& set(bitmask b)
    {
        m_value |= b.m_value;
        return *this;
    }

    bitmask& reset(bitmask b)
    {
        m_value &= ~b.m_value;
        return *this;
    }

    friend bitmask operator|(bitmask lhs, bitmask rhs)
    {
        return lhs.set(rhs);
    }

    friend bitmask operator|(bitmask lhs, enum_type rhs)
    {
        return lhs.set(rhs);
    }

    friend bitmask operator|(enum_type lhs, bitmask rhs)
    {
        return bitmask(lhs) | rhs;
    }

    friend bitmask& operator|=(bitmask& lhs, bitmask rhs)
    {
        return lhs.set(rhs);
    }

    friend bool operator&(bitmask lhs, bitmask rhs)
    {
        return lhs.contains(rhs);
    }

    friend bool operator==(bitmask lhs, bitmask rhs)
    {
        return lhs.m_value == rhs.m_value;
    }

    friend bool operator!=(bitmask lhs, bitmask rhs)
    {
        return !(lhs == rhs);
    }

private:
    static underlying_type to_underlying(E v)
    {
        return static_cast<underlying_type>(v);
    }

    underlying_type m_value;
};

using modes_t = bitmask<mode>;

inline modes_t operator|(mode lhs, mode rhs)
{
    return modes_t(lhs) | rhs;
}

inline std::ostream& operator<<(std::ostream& os, modes_t item)
{
    bool init = true;
    const auto separator = [&]()
    {
        if (!init)
        {
            os << " ";
        }
        init = false;
    };
    os << "[";
#define CASE(v)                 \
    if (item.contains(mode::v)) \
    {                           \
        separator();            \
        os << #v;               \
    }

    CASE(standout)
    CASE(bold)
    CASE(dim)
    CASE(italic)
    CASE(underline)
    CASE(blink)
    CASE(inverse)
    CASE(hidden)
    CASE(crossed_out)
    CASE(double_underline)
#undef CASE
    os << "]";
    return os;
}

struct true_color_t
{
    std::uint8_t red = 0;
    std::uint8_t green = 0;
    std::uint8_t blue = 0;

    friend bool operator==(const true_color_t& lhs, const true_color_t& rhs)
    {
        return std::tie(lhs.red, lhs.green, lhs.blue) == std::tie(rhs.red, rhs.green, rhs.blue);
    }

    friend bool operator!=(const true_color_t& lhs, const true_color_t& rhs)
    {
        return !(lhs == rhs);
    }

    friend std::ostream& operator<<(std::ostream& os, const true_color_t& item)
    {
        return os << "(rgb " << static_cast<int>(item.red) << " " << static_cast<int>(item.green) << " "
                  << static_cast<int>(item.blue) << ")";
    }
};

struct default_color_t
{
    friend bool operator==(const default_color_t&, const default_color_t&)
    {
        return true;
    }

    friend bool operator!=(const default_color_t&, const default_color_t&)
    {
        return false;
    }

    friend std::ostream& operator<<(std::ostream& os, const default_color_t&)
    {
        return os << "default";
    }
};

struct palette_color_t
{
    std::uint8_t index;

    friend bool operator==(const palette_color_t& lhs, const palette_color_t& rhs)
    {
        return lhs.index == rhs.index;
    }

    friend bool operator!=(const palette_color_t& lhs, const palette_color_t& rhs)
    {
        return !(lhs == rhs);
    }

    friend std::ostream& operator<<(std::ostream& os, const palette_color_t& item)
    {
        return os << "(palette " << static_cast<int>(item.index) << ")";
    }
};

enum class basic_color_t
{
    black = 0,
    dark_red = 1,
    dark_green = 2,
    dark_yellow = 3,
    dark_blue = 4,
    dark_magenta = 5,
    dark_cyan = 6,
    gray = 7,
    dark_gray = 60,
    red = 61,
    green = 62,
    yellow = 63,
    blue = 64,
    magenta = 65,
    cyan = 66,
    white = 67,
};

inline std::ostream& operator<<(std::ostream& os, basic_color_t item)
{
#define CASE(v) \
    case basic_color_t::v: return os << #v
    switch (item)
    {
        CASE(black);
        CASE(dark_red);
        CASE(dark_green);
        CASE(dark_yellow);
        CASE(dark_blue);
        CASE(dark_magenta);
        CASE(dark_cyan);
        CASE(gray);
        CASE(dark_gray);
        CASE(red);
        CASE(green);
        CASE(yellow);
        CASE(blue);
        CASE(magenta);
        CASE(cyan);
        CASE(white);
        default: throw std::runtime_error{ "unknown basic_color_t" };
    }
    return os;
}

using color_t = std::variant<palette_color_t, true_color_t, default_color_t, basic_color_t>;

inline bool operator==(const color_t& lhs, const color_t& rhs)
{
    return std::visit(
        ferrugo::core::overloaded{ [](const true_color_t& lhs, const true_color_t& rhs) { return lhs == rhs; },
                                   [](const palette_color_t& lhs, const palette_color_t& rhs) { return lhs == rhs; },
                                   [](const default_color_t& lhs, const default_color_t& rhs) { return lhs == rhs; },
                                   [](const basic_color_t& lhs, const basic_color_t& rhs) { return lhs == rhs; },
                                   [](const auto& lhs, const auto& rhs) { return false; } },
        lhs,
        rhs);
}

inline bool operator!=(const color_t& lhs, const color_t& rhs)
{
    return !(lhs == rhs);
}

inline std::ostream& operator<<(std::ostream& os, const color_t& item)
{
    std::visit([&](const auto& c) { os << c; }, item);
    return os;
}

struct glyph_style_t
{
    color_t foreground = default_color_t{};
    color_t background = default_color_t{};
    modes_t mode_value = modes_t{ mode::none };

    friend bool operator==(const glyph_style_t& lhs, const glyph_style_t& rhs)
    {
        static const auto tie
            = [](const glyph_style_t& item) { return std::tie(item.foreground, item.background, item.mode_value); };
        return tie(lhs) == tie(rhs);
    }

    friend bool operator!=(const glyph_style_t& lhs, const glyph_style_t& rhs)
    {
        return !(lhs == rhs);
    }

    friend std::ostream& operator<<(std::ostream& os, const glyph_style_t& item)
    {
        return os << "[" << item.foreground << " " << item.background << " " << item.mode_value << "]";
    }
};

using character_t = multibyte;

struct glyph_t
{
    character_t character = character_t{ ' ' };
    glyph_style_t style = {};

    glyph_t(character_t character, glyph_style_t style = {}) : character{ std::move(character) }, style{ std::move(style) }
    {
    }

    glyph_t() : glyph_t(' ')
    {
    }

    glyph_t& operator=(glyph_t other)
    {
        std::swap(character, other.character);
        std::swap(style, other.style);
        return *this;
    }

    friend std::ostream& operator<<(std::ostream& os, const glyph_t& item)
    {
        return os << "[\"" << item.character << "\" " << item.style << "]";
    }
};

struct location_t
{
    int x = 0;
    int y = 0;
};

struct extent_t
{
    int width = 0;
    int height = 0;
};

struct bounds_t
{
    location_t location = {};
    extent_t extent = {};
};

struct area_t
{
    struct ref_type
    {
        explicit ref_type(const glyph_t* ptr, extent_t e) : m_ptr{ ptr }, m_extent{ e }
        {
        }

        const glyph_t& operator[](location_t loc) const
        {
            return *(m_ptr + to_index(loc));
        }

        std::size_t to_index(location_t loc) const
        {
            if (!((0 <= loc.x < m_extent.width) && (0 <= loc.y < m_extent.height)))
            {
                throw std::runtime_error{ "location out of bounds" };
            }
            return m_extent.width * loc.y + loc.x;
        }

        const glyph_t* m_ptr;
        extent_t m_extent;
    };

    struct mut_ref_type
    {
        explicit mut_ref_type(glyph_t* ptr, extent_t e) : m_ptr{ ptr }, m_extent{ e }
        {
        }

        glyph_t& operator[](location_t loc) const
        {
            return *(m_ptr + to_index(loc));
        }

        std::size_t to_index(location_t loc) const
        {
            if (!((0 <= loc.x < m_extent.width) && (0 <= loc.y < m_extent.height)))
            {
                throw std::runtime_error{ "location out of bounds" };
            }
            return m_extent.width * loc.y + loc.x;
        }

        void fill(glyph_t value)
        {
            for (int y = 0; y < m_extent.height; ++y)
            {
                for (int x = 0; x < m_extent.width; ++x)
                {
                    (*this)[location_t{ x, y }] = value;
                }
            }
        }

        glyph_t* m_ptr;
        extent_t m_extent;
    };

    explicit area_t(extent_t e) : m_extent{ e }, m_data(e.width * e.height, glyph_t{})
    {
    }

    ref_type ref() const
    {
        return ref_type{ m_data.data(), m_extent };
    }

    mut_ref_type mut_ref()
    {
        return mut_ref_type{ m_data.data(), m_extent };
    }

    extent_t m_extent;
    std::vector<glyph_t> m_data;
};

enum class ground_type_t
{
    foreground,
    background
};

struct buffer_t
{
    std::ostream& m_os;
    glyph_style_t m_prev_style = {};
    using args_t = std::vector<int>;

    explicit buffer_t(std::ostream& os) : m_os{ os }
    {
        // reset();
    }

    buffer_t& set_style(const glyph_style_t& style)
    {
        static const std::map<mode, std::tuple<args_t, args_t>> map = {
            { mode::bold, { args_t{ 1 }, args_t{ 21 } } },   { mode::dim, { args_t{ 2 }, args_t{ 22 } } },
            { mode::italic, { args_t{ 3 }, args_t{ 23 } } }, { mode::underline, { args_t{ 4 }, args_t{ 24 } } },
            { mode::blink, { args_t{ 5 }, args_t{ 25 } } },  { mode::inverse, { args_t{ 7 }, args_t{ 27 } } },
            { mode::hidden, { args_t{ 8 }, args_t{ 28 } } }, { mode::crossed_out, { args_t{ 9 }, args_t{ 29 } } },
        };
        for (const mode m : { mode::bold,
                              mode::dim,
                              mode::italic,
                              mode::underline,
                              mode::blink,
                              mode::inverse,
                              mode::crossed_out,
                              mode::standout })
        {
            if (style.mode_value.contains(m) != m_prev_style.mode_value.contains(m))
            {
                const auto iter = map.find(m);
                if (iter != map.end())
                {
                    const auto& [on_set, on_reset] = iter->second;
                    if (style.mode_value.contains(m))
                    {
                        escape(on_set);
                    }
                    else
                    {
                        escape(on_reset);
                    }
                }
            }
        }

        if (style.foreground != m_prev_style.foreground || style.background != m_prev_style.background)
        {
            write_color(ground_type_t::foreground, style.foreground);
            write_color(ground_type_t::background, style.background);
        }
        m_prev_style = style;
        return *this;
    }

    buffer_t& write(const multibyte& character)
    {
        m_os << character;
        return *this;
    }

    buffer_t& new_line()
    {
        m_prev_style = {};
        escape({ 0 });
        m_os << '\n';
        return *this;
    }

    void escape(const args_t& args)
    {
        m_os << "\033[" << delimit(args, ";") << "m";
    }

    void write_color(ground_type_t type, const color_t& col)
    {
        const args_t args = std::visit(
            ferrugo::core::overloaded{ [&](const true_color_t& c) -> args_t {
                                          return { type == ground_type_t::foreground ? 38 : 48, 2, c.red, c.green, c.blue };
                                      },
                                       [&](const palette_color_t& c) -> args_t {
                                           return { type == ground_type_t::foreground ? 38 : 48, 5, c.index };
                                       },
                                       [&](const default_color_t& c) -> args_t
                                       { return { type == ground_type_t::foreground ? 39 : 49 }; },
                                       [&](const basic_color_t& c) -> args_t
                                       { return { static_cast<int>(c) + (type == ground_type_t::foreground ? 30 : 40) }; } },
            col);
        escape(args);
    }

    void reset()
    {
        escape({});
    }
};

inline void output(const area_t::ref_type& area_t, buffer_t& buf)
{
    for (int y = 0; y < area_t.m_extent.height; ++y)
    {
        for (int x = 0; x < area_t.m_extent.width; ++x)
        {
            const glyph_t& g = area_t[location_t{ x, y }];
            buf.set_style(g.style).write(g.character);
        }
        buf.new_line();
    }
    buf.reset();
}

struct glyph_style_applier_t
{
    using applier_type = std::function<void(glyph_style_t&)>;

    applier_type applier;

    explicit glyph_style_applier_t(applier_type applier) : applier{ std::move(applier) }
    {
    }

    glyph_style_applier_t(modes_t m) : glyph_style_applier_t{ [=](glyph_style_t& s) { s.mode_value |= m; } }
    {
    }

    glyph_style_applier_t(mode m) : glyph_style_applier_t{ [=](glyph_style_t& s) { s.mode_value |= m; } }
    {
    }

    glyph_style_t& operator()(glyph_style_t& s) const
    {
        applier(s);
        return s;
    }

    glyph_t& operator()(glyph_t& g) const
    {
        (*this)(g.style);
        return g;
    }

    operator glyph_style_t() const
    {
        glyph_style_t result{};
        applier(result);
        return result;
    }
};

inline glyph_style_applier_t operator|(glyph_style_applier_t lhs, glyph_style_applier_t rhs)
{
    return glyph_style_applier_t{ [=](glyph_style_t& s)
                                  {
                                      lhs(s);
                                      rhs(s);
                                  } };
}

inline glyph_style_t& operator|=(glyph_style_t& g, const glyph_style_applier_t& applier)
{
    applier(g);
    return g;
}

inline glyph_style_t operator|(glyph_style_t g, const glyph_style_applier_t& applier)
{
    applier(g);
    return g;
}

inline glyph_t operator|(glyph_t g, const glyph_style_applier_t& applier)
{
    applier(g);
    return g;
}

inline glyph_t operator|(character_t ch, const glyph_style_applier_t& applier)
{
    glyph_t g{ ch };
    applier(g);
    return g;
}

inline glyph_t operator|(const char* ch, const glyph_style_applier_t& applier)
{
    glyph_t g{ multibyte_string(ch)[0] };
    applier(g);
    return g;
}

inline glyph_style_applier_t fg(const color_t& col)
{
    return glyph_style_applier_t{ [=](glyph_style_t& s) { s.foreground = col; } };
}

inline glyph_style_applier_t bg(const color_t& col)
{
    return glyph_style_applier_t{ [=](glyph_style_t& s) { s.background = col; } };
}

}  // namespace core
}  // namespace ferrugo