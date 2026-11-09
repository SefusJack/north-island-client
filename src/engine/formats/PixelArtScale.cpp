#include "formats/PixelArtScale.h"

#include "core/Point.h"

#include <array>
#include <cstdlib>
#include <vector>

namespace formats {

    namespace {

        /// @brief A pixel as the distances weigh it: luma, the two colour differences and alpha.
        struct Yuva {
            int y = 0;
            int u = 0;
            int v = 0;
            int a = 0;
        };

        /// @brief Converts a colour to luma, colour differences and alpha (every transparent pixel is all zero).
        /// @param c Source colour.
        /// @return The weighted pixel.
        Yuva yuvaOf(const core::Color& c) {
            if (c.a == 0) {
                return {};
            }
            // BT.601 weights in thousandths: Y = .299R + .587G + .114B; U = -.169R - .331G + .5B; V = .5R - .419G - .081B
            return {(299 * c.r + 587 * c.g + 114 * c.b) / 1000, (-169 * c.r - 331 * c.g + 500 * c.b) / 1000, (500 * c.r - 419 * c.g - 81 * c.b) / 1000, c.a};
        }

        /// @brief xBR's colour distance between two pixels.
        /// @param p First pixel.
        /// @param q Second pixel.
        /// @return The weighted distance.
        int distance(const Yuva& p, const Yuva& q) {
            // xBR weights: luma 48, U 7, V 6, alpha 48
            return 48 * std::abs(p.y - q.y) + 7 * std::abs(p.u - q.u) + 6 * std::abs(p.v - q.v) + 48 * std::abs(p.a - q.a);
        }

        /// @brief Whether two colours count as the same (all transparent pixels are one colour).
        /// @param p First colour.
        /// @param q Second colour.
        /// @return True when equal.
        bool same(const core::Color& p, const core::Color& q) { return (p.a == 0 && q.a == 0) || p == q; }

        /// @brief The picture's pixels with the edge ones repeated outwards.
        class Grid {
            public:
                /// @brief Builds the grid and the weighted copy of the picture.
                /// @param image Source picture (kept by reference).
                explicit Grid(
                    const Image& image
                )
                    : m_image(image) {
                    m_yuva.reserve(image.pixels().size());
                    for (const core::Color& pixel : image.pixels()) {
                        m_yuva.push_back(yuvaOf(pixel));
                    }
                }

                /// @brief The colour at a position, clamped to the picture.
                /// @param x Column.
                /// @param y Row.
                /// @return The colour.
                const core::Color& color(int x, int y) const { return m_image.at(clampX(x), clampY(y)); }

                /// @brief The weighted pixel at a position, clamped to the picture.
                /// @param x Column.
                /// @param y Row.
                /// @return The weighted pixel.
                const Yuva& yuva(int x, int y) const {
                    return m_yuva[static_cast<std::size_t>(clampY(y)) * static_cast<std::size_t>(m_image.width()) + static_cast<std::size_t>(clampX(x))];
                }

            private:
                /// @brief Clamps a column into the picture.
                /// @param x Column.
                /// @return The clamped column.
                int clampX(int x) const { return x < 0 ? 0 : x >= m_image.width() ? m_image.width() - 1 : x; }

                /// @brief Clamps a row into the picture.
                /// @param y Row.
                /// @return The clamped row.
                int clampY(int y) const { return y < 0 ? 0 : y >= m_image.height() ? m_image.height() - 1 : y; }

                const Image& m_image;
                std::vector<Yuva> m_yuva;
        };

        /// @brief Scales one pixel to a 2x2 block (AdvMAME2x).
        /// @param grid Source grid.
        /// @param x Source column.
        /// @param y Source row.
        /// @param out Destination picture.
        void scale2x(const Grid& grid, int x, int y, Image& out) {
            const core::Color& e = grid.color(x, y);
            const core::Color& b = grid.color(x, y - 1);
            const core::Color& d = grid.color(x - 1, y);
            const core::Color& f = grid.color(x + 1, y);
            const core::Color& h = grid.color(x, y + 1);
            const bool rounds = !same(b, h) && !same(d, f);
            out.at(2 * x, 2 * y) = rounds && same(d, b) ? d : e;
            out.at(2 * x + 1, 2 * y) = rounds && same(b, f) ? f : e;
            out.at(2 * x, 2 * y + 1) = rounds && same(d, h) ? d : e;
            out.at(2 * x + 1, 2 * y + 1) = rounds && same(h, f) ? f : e;
        }

        /// @brief Scales one pixel to a 3x3 block (AdvMAME3x).
        /// @param grid Source grid.
        /// @param x Source column.
        /// @param y Source row.
        /// @param out Destination picture.
        void scale3x(const Grid& grid, int x, int y, Image& out) {
            const core::Color& a = grid.color(x - 1, y - 1);
            const core::Color& b = grid.color(x, y - 1);
            const core::Color& c = grid.color(x + 1, y - 1);
            const core::Color& d = grid.color(x - 1, y);
            const core::Color& e = grid.color(x, y);
            const core::Color& f = grid.color(x + 1, y);
            const core::Color& g = grid.color(x - 1, y + 1);
            const core::Color& h = grid.color(x, y + 1);
            const core::Color& i = grid.color(x + 1, y + 1);
            std::array<core::Color, 9> block; // 3x3 output block
            block.fill(e);
            if (!same(b, h) && !same(d, f)) {
                block[0] = same(d, b) ? d : e;
                block[1] = (same(d, b) && !same(e, c)) || (same(b, f) && !same(e, a)) ? b : e;
                block[2] = same(b, f) ? f : e;
                block[3] = (same(d, b) && !same(e, g)) || (same(d, h) && !same(e, a)) ? d : e;
                block[5] = (same(b, f) && !same(e, i)) || (same(h, f) && !same(e, c)) ? f : e;
                block[6] = same(d, h) ? d : e;
                block[7] = (same(d, h) && !same(e, i)) || (same(h, f) && !same(e, g)) ? h : e;
                block[8] = same(h, f) ? f : e;
            }
            for (int row = 0; row < 3; ++row) {
                for (int column = 0; column < 3; ++column) {
                    out.at(3 * x + column, 3 * y + row) = block[static_cast<std::size_t>(row * 3 + column)];
                }
            }
        }

        /// @brief The shape of an edge across E's bottom right corner.
        enum class Edge { Diagonal, Shallow, Steep, Both };

        /// @brief How much of each part of E lies past an edge at its bottom right corner.
        /// @param edge Edge shape.
        /// @param factor Parts per side.
        /// @return factor x factor coverage fractions, row by row.
        std::vector<double> coverage(Edge edge, int factor) {
            constexpr int kSamples = 16; // sample points per cell side (16 x 16 per part) used to measure edge coverage
            std::vector<double> parts(static_cast<std::size_t>(factor * factor));
            for (int row = 0; row < factor; ++row) {
                for (int column = 0; column < factor; ++column) {
                    int inside = 0;
                    for (int sy = 0; sy < kSamples; ++sy) {
                        for (int sx = 0; sx < kSamples; ++sx) {
                            // sample points at sub-cell centres; edges: shallow y > 1 - x/2, steep x > 1 - y/2, 45 degree x + y > 1.5
                            const double px = (column + (sx + 0.5) / kSamples) / factor;
                            const double py = (row + (sy + 0.5) / kSamples) / factor;
                            const bool shallow = py > 1 - 0.5 * px;
                            const bool steep = px > 1 - 0.5 * py;
                            const bool past = edge == Edge::Diagonal  ? px + py > 1.5
                                              : edge == Edge::Shallow ? shallow
                                              : edge == Edge::Steep   ? steep
                                                                      : shallow || steep;
                            inside += past ? 1 : 0;
                        }
                    }
                    parts[static_cast<std::size_t>(row * factor + column)] = static_cast<double>(inside) / (kSamples * kSamples);
                }
            }
            return parts;
        }

        /// @brief Mixes a colour over another by weight, each weighed by its own alpha.
        /// @param base Underlying colour.
        /// @param top Colour laid over it.
        /// @param weight Share of the top colour (0..1).
        /// @return The blended colour (fully transparent when the result is almost invisible).
        core::Color blend(const core::Color& base, const core::Color& top, double weight) {
            const double alpha = base.a * (1 - weight) + top.a * weight;
            if (alpha < 0.5) { // below half an alpha step counts as transparent
                return {0, 0, 0, 0};
            }
            const auto mix = [&](std::uint8_t b, std::uint8_t t) {
                const double value = (b * base.a * (1 - weight) + t * top.a * weight) / alpha;
                return static_cast<std::uint8_t>(value + 0.5);
            };
            return {mix(base.r, top.r), mix(base.g, top.g), mix(base.b, top.b), static_cast<std::uint8_t>(alpha + 0.5)};
        }

        /// @brief Hyllian's xBR scaler for one factor, with its precomputed edge coverage.
        class Xbr {
            public:
                /// @brief Precomputes the coverage of every edge shape.
                /// @param factor Scale factor.
                explicit Xbr(
                    int factor
                )
                    : m_factor(factor) {
                    for (const Edge edge : {Edge::Diagonal, Edge::Shallow, Edge::Steep, Edge::Both}) {
                        m_coverage[static_cast<std::size_t>(edge)] = coverage(edge, factor);
                    }
                }

                /// @brief Scales one pixel to a factor x factor block.
                /// @param grid Source grid.
                /// @param x Source column.
                /// @param y Source row.
                /// @param out Destination picture.
                void scale(const Grid& grid, int x, int y, Image& out) const {
                    std::vector<core::Color> block(static_cast<std::size_t>(m_factor * m_factor), grid.color(x, y));
                    for (int turn = 0; turn < 4; ++turn) { // the four corners of the pixel
                        corner(grid, x, y, turn, block);
                    }
                    for (int row = 0; row < m_factor; ++row) {
                        for (int column = 0; column < m_factor; ++column) {
                            out.at(m_factor * x + column, m_factor * y + row) = block[static_cast<std::size_t>(row * m_factor + column)];
                        }
                    }
                }

            private:
                /// @brief Turns a step from the bottom right corner's frame to a corner's frame.
                /// @param dx Step in x.
                /// @param dy Step in y.
                /// @param turn Quarter clockwise turns (0: bottom right, 1: bottom left, 2: top left, 3: top right).
                /// @return The turned step.
                static core::Point turned(int dx, int dy, int turn) {
                    switch (turn) {
                        case 1: return {-dy, dx}; // one quarter turn (bottom left corner)
                        case 2: return {-dx, -dy}; // half turn (top left corner)
                        case 3: return {dy, -dx}; // three quarter turns (top right corner)
                        default: return {dx, dy}; // 0: no turn (bottom right corner)
                    }
                }

                /// @brief Blends the outside neighbour into the block where an edge crosses one corner of the pixel.
                /// @param grid Source grid.
                /// @param x Source column.
                /// @param y Source row.
                /// @param turn Which corner, as quarter turns from the bottom right.
                /// @param block Output block (factor x factor) changed in place.
                void corner(
                    const Grid& grid,
                    int x,
                    int y,
                    int turn,
                    std::vector<core::Color>& block
                ) const {
                    const auto colorAt = [&](int dx, int dy) -> const core::Color& {
                        const core::Point step = turned(dx, dy, turn);
                        return grid.color(x + step.x, y + step.y);
                    };
                    const auto yuvaAt = [&](int dx, int dy) -> const Yuva& {
                        const core::Point step = turned(dx, dy, turn);
                        return grid.yuva(x + step.x, y + step.y);
                    };
                    const core::Color& e = colorAt(0, 0);
                    const core::Color& f = colorAt(1, 0);
                    const core::Color& h = colorAt(0, 1);
                    if (same(e, f) || same(e, h)) {
                        return;
                    }
                    const Yuva& ye = yuvaAt(0, 0);
                    const Yuva& yf = yuvaAt(1, 0);
                    const Yuva& yh = yuvaAt(0, 1);
                    const Yuva& yi = yuvaAt(1, 1);
                    const Yuva& yb = yuvaAt(0, -1);
                    const Yuva& yc = yuvaAt(1, -1);
                    const Yuva& yd = yuvaAt(-1, 0);
                    const Yuva& yg = yuvaAt(-1, 1);
                    // xBR weighting: the centre pair (H,F or E,I) counts 4 times; (0, 2) and (2, 0) etc. are the outer neighbours
                    const int across = distance(ye, yc) + distance(ye, yg) + distance(yi, yuvaAt(0, 2)) + distance(yi, yuvaAt(2, 0)) + 4 * distance(yh, yf);
                    const int along = distance(yh, yd) + distance(yh, yuvaAt(1, 2)) + distance(yf, yuvaAt(2, 1)) + distance(yf, yb) + 4 * distance(ye, yi);
                    if (across >= along) {
                        return;
                    }
                    const core::Color& outside = distance(ye, yf) <= distance(ye, yh) ? f : h;
                    const core::Color& b = colorAt(0, -1);
                    const core::Color& c = colorAt(1, -1);
                    const core::Color& d = colorAt(-1, 0);
                    const core::Color& g = colorAt(-1, 1);
                    const core::Color& i = colorAt(1, 1);
                    Edge edge = Edge::Diagonal;
                    const bool shaped =
                        (!same(f, b) && !same(h, d)) || (same(e, i) && !same(f, colorAt(2, 1)) && !same(h, colorAt(1, 2))) || same(e, g) || same(e, c);
                    if (shaped) {
                        const int towardsG = distance(yf, yg);
                        const int towardsC = distance(yh, yc);
                        // 2: an edge is shallow/steep when one distance is at most half the other
                        const bool shallow = 2 * towardsG <= towardsC && !same(e, g) && !same(d, g);
                        const bool steep = towardsG >= 2 * towardsC && !same(e, c) && !same(b, c);
                        edge = shallow && steep ? Edge::Both : shallow ? Edge::Shallow : steep ? Edge::Steep : Edge::Diagonal;
                    }
                    const std::vector<double>& parts = m_coverage[static_cast<std::size_t>(edge)];
                    for (int row = 0; row < m_factor; ++row) {
                        for (int column = 0; column < m_factor; ++column) {
                            const double weight = parts[static_cast<std::size_t>(row * m_factor + column)];
                            if (weight <= 0) {
                                continue;
                            }
                            const int cx = 2 * column - (m_factor - 1);
                            const int cy = 2 * row - (m_factor - 1);
                            const core::Point place = turned(cx, cy, turn);
                            const int toColumn = (place.x + (m_factor - 1)) / 2;
                            const int toRow = (place.y + (m_factor - 1)) / 2;
                            core::Color& part = block[static_cast<std::size_t>(toRow * m_factor + toColumn)];
                            part = blend(part, outside, weight);
                        }
                    }
                }

                int m_factor;
                std::array<std::vector<double>, 4> m_coverage; // one entry per Edge shape
        };

    }

    const char* pixelArtFilterName(PixelArtFilter filter) {
        switch (filter) {
            case PixelArtFilter::Scale: return "scale"; // AdvMAME Scale2x / Scale3x
            case PixelArtFilter::Xbr: return "xbr"; // Hyllian's xBR
            default: return "off"; // no filter (nearest-neighbour blocks)
        }
    }

    std::optional<PixelArtFilter> pixelArtFilterNamed(const std::string& name) {
        if (name == "off") {
            return PixelArtFilter::None;
        }
        if (name == "scale") {
            return PixelArtFilter::Scale;
        }
        if (name == "xbr") {
            return PixelArtFilter::Xbr;
        }
        return std::nullopt;
    }

    Image scalePixelArt(const Image& image, PixelArtFilter filter, int factor) {
        factor = factor < 1 ? 1 : factor;
        Image out(image.width() * factor, image.height() * factor);
        if (image.isEmpty()) {
            return out;
        }
        const Grid grid(image);
        if (filter == PixelArtFilter::Scale && (factor == 2 || factor == 3)) { // Scale2x and Scale3x exist only for factors 2 and 3
            for (int y = 0; y < image.height(); ++y) {
                for (int x = 0; x < image.width(); ++x) {
                    factor == 2 ? scale2x(grid, x, y, out) : scale3x(grid, x, y, out);
                }
            }
            return out;
        }
        if (filter == PixelArtFilter::Xbr && factor > 1) {
            const Xbr xbr(factor);
            for (int y = 0; y < image.height(); ++y) {
                for (int x = 0; x < image.width(); ++x) {
                    xbr.scale(grid, x, y, out);
                }
            }
            return out;
        }
        for (int y = 0; y < out.height(); ++y) {
            for (int x = 0; x < out.width(); ++x) {
                out.at(x, y) = image.at(x / factor, y / factor);
            }
        }
        return out;
    }

}
