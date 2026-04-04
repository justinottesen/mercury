#pragma once

// std::views::concat was voted into C++26 (P2542R8).
// Expose it as future::views::concat; when the standard library ships it,
// the #else branch below can be removed and the using-declaration kept.

#include <ranges>
#include <tuple>
#include <variant>

#ifdef __cpp_lib_ranges_concat

    #warning "std::views::concat is now available; remove the polyfill in <future/concat.hpp>"

namespace future::views { using std::views::concat; }

#else

namespace future::views::detail {

// -- concat_view ---------------------------------------------------------------

template <std::ranges::input_range... Views>
    requires(std::ranges::view<Views> && ...) && (sizeof...(Views) > 0)
class concat_view : public std::ranges::view_interface<concat_view<Views...>> {
    static constexpr std::size_t N = sizeof...(Views);

    std::tuple<Views...> m_views;

    friend class iterator;

public:
    using value_type = std::common_type_t<std::ranges::range_value_t<Views>...>;
    using reference  = std::common_reference_t<std::ranges::range_reference_t<Views>...>;

    // -- iterator --------------------------------------------------------------

    class iterator {
        const concat_view*                              m_parent = nullptr;
        std::variant<std::ranges::iterator_t<Views>...> m_it;

        // Advance past the end of the current sub-range into the next one,
        // recursively skipping over any empty sub-ranges.
        template <std::size_t I = 0>
        constexpr void satisfy() {
            if constexpr (I < N) {
                if (m_it.index() == I) {
                    if constexpr (I < N - 1) {
                        if (std::get<I>(m_it) == std::ranges::end(std::get<I>(m_parent->m_views))) {
                            m_it.template emplace<I + 1>(
                                std::ranges::begin(std::get<I + 1>(m_parent->m_views)));
                            satisfy<I + 1>();
                        }
                    }
                } else {
                    satisfy<I + 1>();
                }
            }
        }

    public:
        using value_type       = concat_view::value_type;
        using difference_type  = std::ptrdiff_t;
        using iterator_concept = std::input_iterator_tag;

        constexpr iterator() = default;

        constexpr iterator(const concat_view*                              parent,
                           std::variant<std::ranges::iterator_t<Views>...> it)
            : m_parent{parent}
            , m_it{std::move(it)} {
            satisfy();
        }

        constexpr auto operator*() const -> reference {
            return std::visit([](auto& i) -> reference { return *i; }, m_it);
        }

        constexpr auto operator++() -> iterator& {
            std::visit([](auto& i) -> void { i = std::next(i); }, m_it);
            satisfy();
            return *this;
        }

        constexpr void operator++(int) { ++*this; }

        constexpr auto operator==(const iterator& other) const noexcept -> bool {
            return m_it == other.m_it;
        }

        [[nodiscard]] constexpr auto at_end() const -> bool {
            if (m_it.index() != N - 1) { return false; }
            return std::get<N - 1>(m_it) == std::ranges::end(std::get<N - 1>(m_parent->m_views));
        }
    };

    // -- sentinel --------------------------------------------------------------

    struct sentinel {
        constexpr sentinel() = default;

        constexpr explicit sentinel(const concat_view* /*unused*/) {}

        constexpr auto operator==(const iterator& it) const -> bool { return it.at_end(); }
    };

    // -- concat_view interface -------------------------------------------------

    constexpr concat_view() = default;

    constexpr explicit concat_view(Views... views)
        : m_views{std::move(views)...} {}

    [[nodiscard]] constexpr auto begin() const -> iterator {
        return iterator{
            this, std::variant<std::ranges::iterator_t<Views>...>{
                                                                  std::in_place_index<0>, std::ranges::begin(std::get<0>(m_views))}
        };
    }

    [[nodiscard]] constexpr auto end() const -> sentinel { return sentinel{this}; }
};

// -- adaptor object ------------------------------------------------------------

struct concat_fn {
    template <std::ranges::viewable_range... Rs>
    constexpr auto operator()(Rs&&... rs) const {
        return concat_view<std::views::all_t<Rs>...>{std::views::all(std::forward<Rs>(rs))...};
    }
};

}    // namespace future::views::detail

namespace future::views { inline constexpr detail::concat_fn concat{}; }

#endif    // __cpp_lib_ranges_concat
