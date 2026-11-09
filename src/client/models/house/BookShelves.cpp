#include "client/models/house/BookShelves.h"

namespace client {

    void BookShelves::set(bool desk, int place, int itemId) {
        if (place >= 1 && place <= kPlaces) {
            (desk ? m_desk : m_case)[static_cast<std::size_t>(place - 1)] = itemId;
        }
    }

    int BookShelves::item(bool desk, int place) const {
        return place >= 1 && place <= kPlaces ? (desk ? m_desk : m_case)[static_cast<std::size_t>(place - 1)] : 0;
    }

    int BookShelves::firstEmpty(bool desk) const {
        for (int place = 1; place <= kPlaces; ++place) {
            if (item(desk, place) == 0) {
                return place;
            }
        }
        return 0;
    }

    int BookShelves::placeOf(bool desk, int itemId) const {
        for (int place = 1; place <= kPlaces; ++place) {
            if (item(desk, place) == itemId) {
                return place;
            }
        }
        return 0;
    }

}
