#ifndef NORTH_ISLAND_FORMATS_JPEG_DECODER_H
#define NORTH_ISLAND_FORMATS_JPEG_DECODER_H

#include "core/Bytes.h"
#include "formats/Image.h"

#include <cstddef>
#include <optional>
#include <vector>

namespace formats {

    /// @brief Decodes a baseline or progressive JPEG into opaque pixels.
    /// @param data Raw JPEG file bytes.
    /// @return The image, or nothing when decoding fails.
    std::optional<Image> decodeJpeg(const core::Bytes& data);

    /// @brief The row bands a baseline JPEG can be decoded in, one at a time, when its restart markers allow it.
    /// @details Works when every restart interval is a whole number of block rows and all channels share one sampling (the
    /// large map pictures: 4:4:4 with a restart marker after each block row). A band is a JPEG of its own: the original
    /// headers with the band's height, then that band's entropy data. The pixels match a whole-picture decode.
    class JpegBands {
        public:
            /// @brief Finds the bands of a JPEG.
            /// @param data The JPEG file bytes.
            /// @return The bands, or nothing when the file cannot be split.
            static std::optional<JpegBands> parse(const core::Bytes& data);

            int width() const { return m_width; }
            int height() const { return m_height; }

            /// @brief How many bands there are.
            int count() const { return static_cast<int>(m_starts.size()); }

            /// @brief The pixel rows of a band (the last band may be shorter).
            int bandRows() const { return m_bandRows; }

            /// @brief The first pixel row of a band; count() gives the picture's height.
            int top(int band) const { return band >= count() ? m_height : band * m_bandRows; }

            /// @brief Decodes some bands in a row (any thread).
            /// @param data The same bytes parse was given.
            /// @param first The first band.
            /// @param last One past the last band.
            /// @param out Gets width() pixels a row for the rows top(first) to top(last).
            /// @return False when the decode fails.
            bool decode(const core::Bytes& data, int first, int last, core::Color* out) const;

        private:
            int m_width = 0;
            int m_height = 0;
            int m_bandRows = 0;
            std::size_t m_frame = 0; // where the frame header marker is
            std::size_t m_scan = 0; // where the entropy data starts
            std::vector<std::size_t> m_starts; // where each band's data starts
            std::vector<std::size_t> m_ends; // and ends (at its restart marker, or the marker ending the scan)
    };

}

#endif
