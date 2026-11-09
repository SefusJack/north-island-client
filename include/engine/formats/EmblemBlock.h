#ifndef NORTH_ISLAND_FORMATS_EMBLEM_BLOCK_H
#define NORTH_ISLAND_FORMATS_EMBLEM_BLOCK_H

#include "core/Bytes.h"

namespace formats {

    /// @brief A guild badge as the game keeps it: 24 x 24 pixels of four bits, two to a byte (the first in the high half), in the order x * 24 + y; 288 bytes.
    /// @details Original: TEmblemBlock, a .ens file. The pixel value is an index of the emblem palette (emblemPaletteColor); 15 is empty.
    class EmblemBlock {
        public:
            static constexpr int kSide = 24; // badge width and height, px
            static constexpr std::size_t kBytes = 288; // 24 * 24 pixels / 2 pixels per byte
            static constexpr int kEmpty = 15; // palette index meaning "no pixel"

            /// @brief Takes the first 288 bytes of a file (what is missing stays zero).
            /// @param data File contents.
            /// @return The badge.
            /// @details Original: LoadEmblemFile.
            static EmblemBlock parse(const core::Bytes& data);

            /// @brief Reads one pixel.
            /// @param index Pixel number, x * 24 + y.
            /// @return Emblem palette index 0..15.
            /// @details Original: GetEmblemPixel.
            int pixel(int index) const;

            /// @brief Writes one pixel.
            /// @param index Pixel number, x * 24 + y.
            /// @param color Emblem palette index 0..15.
            /// @details Original: SetEmblemPixel.
            void setPixel(int index, int color);

            /// @brief Gives the raw 288 bytes.
            /// @return The packed pixels.
            const core::Bytes& bytes() const { return m_bytes; }

        private:
            core::Bytes m_bytes = core::Bytes(kBytes, 0);
    };

}

#endif
