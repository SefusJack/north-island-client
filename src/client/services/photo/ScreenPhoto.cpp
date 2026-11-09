#include "client/services/photo/ScreenPhoto.h"

#include "client/services/photo/PhotoText.h"

#include <algorithm>
#include <array>
#include <fstream>

namespace client {

    std::filesystem::path ScreenPhoto::chooseFile(const std::filesystem::path& photoFolder, std::time_t now) {
        std::error_code ignored;
        for (;; ++now) {
            std::tm local{};
#ifdef _WIN32
            localtime_s(&local, &now);
#else
            localtime_r(&now, &local);
#endif
            std::array<char, 16> name{}; // buffer size for mmddhhnnss
            std::strftime(name.data(), name.size(), "%m%d%H%M%S", &local);
            const std::filesystem::path file = photoFolder / (std::string(name.data()) + ".bmp");
            if (!std::filesystem::exists(file, ignored)) {
                return file;
            }
        }
    }

    std::string ScreenPhoto::savedMessage(const std::filesystem::path& file) {
        std::filesystem::path shown = file;
        return "Have saved in " + shown.make_preferred().string();
    }

    namespace {

        constexpr std::size_t kCheckSize = 18096; // check file size in bytes: 84 header + 1501 players * 12
        constexpr std::uint8_t kCheckKey = 39; // XOR key applied to every byte of the check file
        constexpr std::size_t kMaxPlayers = 1501; // most player records in the check file
        constexpr std::size_t kPlayersAt = 84; // byte offset of the first player record
        constexpr std::size_t kHeaderSize = 54; // BMP header size: 14 file header + 40 info header
        constexpr int kBitsPerPixel = 24; // BMP bits per pixel

        /// @brief Writes a 32-bit value little-endian into a byte buffer.
        /// @param bytes The buffer.
        /// @param at The offset of the first byte.
        /// @param value The value to write.
        void put32(std::vector<std::uint8_t>& bytes, std::size_t at, std::int32_t value) {
            for (std::size_t i = 0; i < 4; ++i) { // 4 bytes in a 32-bit value
                bytes[at + i] = static_cast<std::uint8_t>(static_cast<std::uint32_t>(value) >> (8 * i)); // 8 bits per byte
            }
        }

        /// @brief Writes a Pascal short string (length byte, then characters) into a byte buffer.
        /// @param bytes The buffer.
        /// @param at The offset of the length byte.
        /// @param text The text; cut to the capacity.
        /// @param capacity The most characters the field holds.
        void putShortString(std::vector<std::uint8_t>& bytes, std::size_t at, const std::string& text, std::size_t capacity) {
            const std::size_t length = std::min(text.size(), capacity);
            bytes[at] = static_cast<std::uint8_t>(length);
            std::copy_n(text.begin(), length, bytes.begin() + static_cast<std::ptrdiff_t>(at + 1));
        }

        /// @brief Tells whether a row number is in a list.
        /// @param y The row.
        /// @param rows The rows to look in.
        /// @return True when the row is listed.
        bool inRows(int y, std::initializer_list<int> rows) { return std::find(rows.begin(), rows.end(), y) != rows.end(); }

    }

    std::vector<std::uint8_t> ScreenPhoto::checkFile(
        const formats::Image& picture,
        const PhotoCheck& check,
        const std::string& address,
        const std::string& name
    ) {
        std::int32_t sumR = 0;
        std::int32_t sumG = 0;
        std::int32_t sumB = 0;
        std::int32_t greenSum = 0;
        std::int32_t blueSum = 0;
        int parity = 1;
        for (int y = 0; y < picture.height(); ++y) {
            for (int x = 0; x < picture.width(); ++x) {
                const core::Color& pixel = picture.at(x, y);
                sumR += pixel.r;
                sumG += pixel.g;
                sumB += pixel.b;
                int green = pixel.g;
                int blue = pixel.b;
                if (parity == 1) {
                    blue += blue % 2 == 0 ? 1 : 0;
                    green -= green % 2 == 1 ? 1 : 0;
                } else {
                    blue -= blue % 2 == 1 ? 1 : 0;
                    green += green % 2 == 0 ? 1 : 0;
                }
                if (inRows(y, {1, 13, 23, 33, 43, 53, 63, 73, 83, 93, 113, 163})) { // the 12 rows whose green bytes are summed
                    greenSum += green;
                } else if (inRows(y, {6, 16, 26, 36, 46, 56, 66, 76, 86, 96, 116, 166})) { // the 12 rows whose blue bytes are summed
                    blueSum += blue;
                }
                parity ^= 1;
            }
        }
        std::vector<std::uint8_t> bytes(kCheckSize, 0);
        // check file layout: 0 player id, 4 map id (u16), 8 and 12 position x and y, 16 address (short string of 20), 37 photo name (short string of 15), 56 60 64 sums of red green blue, 68 green rows sum, 72 and 76 blue rows sum, 80 player count, 84 on the players
        put32(bytes, 0, check.playerId);
        bytes[4] = static_cast<std::uint8_t>(check.mapId & 0xFF); // low byte of the map id
        bytes[5] = static_cast<std::uint8_t>((check.mapId >> 8) & 0xFF); // high byte of the map id
        put32(bytes, 8, check.position.x);
        put32(bytes, 12, check.position.y);
        putShortString(bytes, 16, address, 20); // offset 16, up to 20 characters
        putShortString(bytes, 37, name, 15); // offset 37, up to 15 characters
        put32(bytes, 56, sumR);
        put32(bytes, 60, sumG);
        put32(bytes, 64, sumB);
        put32(bytes, 68, greenSum);
        put32(bytes, 72, blueSum);
        put32(bytes, 76, blueSum);
        const std::size_t count = std::min(check.players.size(), kMaxPlayers);
        put32(bytes, 80, static_cast<std::int32_t>(count));
        for (std::size_t i = 0; i < count; ++i) {
            // each player record is 12 bytes: id, x, y
            put32(bytes, kPlayersAt + i * 12, check.players[i].id);
            put32(bytes, kPlayersAt + i * 12 + 4, check.players[i].position.x);
            put32(bytes, kPlayersAt + i * 12 + 8, check.players[i].position.y);
        }
        for (std::uint8_t& byte : bytes) {
            byte ^= kCheckKey;
        }
        return bytes;
    }

    bool ScreenPhoto::save(const std::filesystem::path& bmpFile, formats::Image screen, const PhotoCheck& check, std::time_t now) {
        const std::string address = PhotoText::localAddress();
        PhotoText::stamp(screen, PhotoText::dateLine(now), address);
        std::filesystem::path bftFile = bmpFile;
        bftFile.replace_extension(".bft");
        {
            const std::vector<std::uint8_t> bytes = checkFile(screen, check, address, bmpFile.stem().string());
            std::ofstream out(bftFile, std::ios::binary);
            out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
            if (!out) {
                return false;
            }
        }
        const int width = screen.width();
        const int height = screen.height();
        const std::size_t stride = (static_cast<std::size_t>(width) * kBitsPerPixel / 8 + 3) / 4 * 4; // row bytes: 8 bits per byte, padded to 4
        std::vector<std::uint8_t> file(kHeaderSize + stride * static_cast<std::size_t>(height), 0);
        // BMP header: 0 'BM', 2 file size, 10 pixel data offset, 14 info header size (40), 18 width, 22 height, 26 planes (1), 28 bits per pixel, 34 image size
        file[0] = 'B';
        file[1] = 'M';
        put32(file, 2, static_cast<std::int32_t>(file.size()));
        put32(file, 10, static_cast<std::int32_t>(kHeaderSize));
        put32(file, 14, 40); // info header size, bytes
        put32(file, 18, width);
        put32(file, 22, height);
        file[26] = 1; // number of colour planes
        file[28] = kBitsPerPixel;
        put32(file, 34, static_cast<std::int32_t>(stride * static_cast<std::size_t>(height)));
        for (int y = 0; y < height; ++y) {
            std::uint8_t* row = file.data() + kHeaderSize + stride * static_cast<std::size_t>(height - 1 - y);
            for (int x = 0; x < width; ++x) {
                const core::Color& pixel = screen.at(x, y);
                row[x * 3] = pixel.b; // 3 bytes per pixel: B, G, R
                row[x * 3 + 1] = pixel.g;
                row[x * 3 + 2] = pixel.r;
            }
        }
        std::ofstream out(bmpFile, std::ios::binary);
        out.write(reinterpret_cast<const char*>(file.data()), static_cast<std::streamsize>(file.size()));
        return static_cast<bool>(out);
    }

}
