#include "client/services/photo/PhotoText.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <winsock2.h>
#include <windows.h>
#endif

namespace client {

    namespace {

        constexpr int kAddressLineY = 15; // y of the address text line below the date line, px

    }

    std::string PhotoText::dateLine(std::time_t now) {
        std::tm local{};
#ifdef _WIN32
        localtime_s(&local, &now);
#else
        localtime_r(&now, &local);
#endif
        std::array<char, 64> head{}; // buffer size for the weekday and month text
        std::strftime(head.data(), head.size(), "%A, %B ", &local);
        std::array<char, 64> tail{}; // buffer size for the year and clock text
        std::strftime(tail.data(), tail.size(), ", %Y, %I:%M:%S %p", &local);
        return std::string(head.data()) + std::to_string(local.tm_mday) + tail.data();
    }

#ifdef _WIN32

    std::string PhotoText::localAddress() {
        WSADATA data{};
        if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
            return {};
        }
        std::string result;
        std::array<char, 64> name{}; // buffer size for the host name
        if (gethostname(name.data(), static_cast<int>(name.size())) == 0) {
            if (const hostent* host = gethostbyname(name.data()); host != nullptr && host->h_addrtype == AF_INET) {
                for (char** list = host->h_addr_list; *list != nullptr; ++list) {
                    result = inet_ntoa(*reinterpret_cast<in_addr*>(*list));
                }
            }
        }
        WSACleanup();
        return result;
    }

    void PhotoText::stamp(formats::Image& picture, const std::string& date, const std::string& address) {
        const int width = picture.width();
        const int height = picture.height();
        BITMAPINFO info{};
        info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        info.bmiHeader.biWidth = width;
        info.bmiHeader.biHeight = -height;
        info.bmiHeader.biPlanes = 1;
        info.bmiHeader.biBitCount = 24; // bits per pixel (B, G, R)
        info.bmiHeader.biCompression = BI_RGB;
        void* bits = nullptr;
        HDC context = CreateCompatibleDC(nullptr);
        HBITMAP bitmap = CreateDIBSection(context, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
        if (context == nullptr || bitmap == nullptr || bits == nullptr) {
            if (bitmap != nullptr) {
                DeleteObject(bitmap);
            }
            if (context != nullptr) {
                DeleteDC(context);
            }
            return;
        }
        const std::size_t stride = (static_cast<std::size_t>(width) * 3 + 3) / 4 * 4; // row bytes: 3 per pixel, padded to 4
        auto* rows = static_cast<std::uint8_t*>(bits);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                const core::Color& pixel = picture.at(x, y);
                std::uint8_t* at = rows + static_cast<std::size_t>(y) * stride + static_cast<std::size_t>(x) * 3; // 3 bytes per pixel
                at[0] = pixel.b;
                at[1] = pixel.g;
                at[2] = pixel.r;
            }
        }
        HGDIOBJ oldBitmap = SelectObject(context, bitmap);
        HFONT font = CreateFontA(
            -11,
            0,
            0,
            0,
            FW_NORMAL,
            FALSE,
            FALSE,
            FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, // font height -11 = 8 points at 96 dpi
            DEFAULT_PITCH | FF_DONTCARE,
            "MS Sans Serif"
        );
        HGDIOBJ oldFont = SelectObject(context, font);
        SetBkMode(context, OPAQUE);
        SetBkColor(context, RGB(255, 255, 255)); // white background
        SetTextColor(context, RGB(0, 0, 0));
        TextOutA(context, 0, 0, date.c_str(), static_cast<int>(date.size()));
        TextOutA(context, 0, kAddressLineY, address.c_str(), static_cast<int>(address.size()));
        GdiFlush();
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                const std::uint8_t* at = rows + static_cast<std::size_t>(y) * stride + static_cast<std::size_t>(x) * 3; // 3 bytes per pixel
                picture.at(x, y) = {at[2], at[1], at[0], 255}; // alpha 255: opaque
            }
        }
        SelectObject(context, oldFont);
        DeleteObject(font);
        SelectObject(context, oldBitmap);
        DeleteObject(bitmap);
        DeleteDC(context);
    }

#else

    std::string PhotoText::localAddress() { return {}; }

    void PhotoText::stamp(formats::Image&, const std::string&, const std::string&) {}

#endif

}
