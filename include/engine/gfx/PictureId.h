#ifndef NORTH_ISLAND_GFX_PICTURE_ID_H
#define NORTH_ISLAND_GFX_PICTURE_ID_H

namespace gfx {

    /// @brief Names a picture of a PictureLibrary; the default value names no picture.
    struct PictureId {
        int value = -1;

        /// @brief Tells whether this names a picture.
        /// @return True when the value is not negative.
        bool isValid() const { return value >= 0; }

        /// @brief Compares two ids.
        /// @param other The other id.
        /// @return True when both name the same picture.
        bool operator==(PictureId other) const { return value == other.value; }

        /// @brief Compares two ids.
        /// @param other The other id.
        /// @return True when they name different pictures.
        bool operator!=(PictureId other) const { return value != other.value; }
    };

}

#endif
