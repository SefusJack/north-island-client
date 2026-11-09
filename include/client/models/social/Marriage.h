#ifndef NORTH_ISLAND_CLIENT_MODELS_SOCIAL_MARRIAGE_H
#define NORTH_ISLAND_CLIENT_MODELS_SOCIAL_MARRIAGE_H

#include "client/models/social/SpouseCard.h"
#include "client/models/social/TeamMate.h"
#include "core/Bytes.h"
#include "core/Point.h"
#include "formats/ItemTable.h"
#include "protocol/ItemInstance.h"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>

namespace client {

    /// @brief What the player knows of marriage: the spouse and card the Marry tab shows, the two players of the marriage or divorce application, and the wedding.
    /// @details Original: MK_Menu.pas TMarryApplication, TMarryDivorce, TMarryCard without their windows. Takes in the server's action 45 and sends the answers.
    class Marriage {
        public:
            /// @brief Callbacks into the rest of the client.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<std::optional<TeamMate>(std::int32_t)> player;
                std::function<std::int32_t()> ownId;
                std::function<void(const std::string&, int)> message;
                std::function<void(const std::string&)> chat;
                std::function<void(const std::string&, const std::optional<TeamMate>&, int, bool)> speak;
                std::function<void(std::int32_t, int)> turn;
                std::function<void(std::int32_t)> setGold;
                std::function<void(bool)> lockInput;
                std::function<void(const std::string&)> playMusic;
                std::function<void(bool)> weddingFlag;
                std::function<std::string(int)> sceneName;
                std::function<void()> openApplication;
                std::function<void()> closeApplication;
                std::function<void()> openDivorce;
                std::function<void()> closeDivorce;
                std::function<void()> eventMoveOn;
                std::function<void(const std::string&, std::function<void()>, std::function<void()>)> ask;
                std::function<void(int)> playAction;
                std::function<void(std::int32_t, core::Point)> setPosition;
                std::function<void(std::int32_t, bool)> hidePet;
                std::function<void(const protocol::ItemInstance&)> equip;
            };

            /// @brief Creates the marriage model.
            /// @param items Item table (rings, equipment slots).
            /// @param hooks Callbacks into the client.
            Marriage(
                const formats::ItemTable& items,
                Hooks hooks
            );

            /// @brief Gets the spouse's player id (original Player.FSpouseId).
            /// @return The id, 0 for none.
            std::int32_t spouseId() const { return m_spouseId; }

            /// @brief Gets the spouse card the Marry tab shows.
            /// @return The card.
            const SpouseCard& card() const { return m_card; }

            /// @brief Gets the own player of the application the forms show.
            /// @return The applicant.
            const std::optional<TeamMate>& applicant() const { return m_applicant; }

            /// @brief Gets the other player of the application the forms show.
            /// @return The partner.
            const std::optional<TeamMate>& partner() const { return m_partner; }

            /// @brief Gets the id of the other player of the application.
            /// @return The partner id.
            std::int32_t partnerId() const { return m_partnerId; }

            /// @brief Tells whether Accept has been pressed (the button shows the waiting picture).
            /// @return True after Accept.
            bool accepted() const { return m_accepted; }

            /// @brief Accepts the marriage application (original TMarryApplication.AcceptMarryApplication).
            void acceptMarriage();

            /// @brief Cancels the marriage application (original TMarryApplication.CancelMarryApplication).
            void cancelMarriage();

            /// @brief Accepts the divorce application (original TMarryDivorce.AcceptDivorceApplication).
            void acceptDivorce();

            /// @brief Cancels the divorce application (original TMarryDivorce.CancelDivorceApplication).
            void cancelDivorce();

            /// @brief Agrees to the spouse's request to get divorced (original TMarrySubForm.AcceptDivorceRequest).
            void agreeToDivorce();

            /// @brief Refuses the spouse's request to get divorced (original TMarrySubForm.RefuseDivorceRequest).
            void refuseDivorce();

            /// @brief Asks whether to use the divorce item (original Se_EquipUI.pas UseGridItem); on Confirm sends packet 23/113 with the bag place.
            /// @details Original: ConfirmForcedDivorce / CancelForcedDivorce.
            /// @param bagSlot Bag place of the divorce item.
            void askForcedDivorce(int bagSlot);

            /// @brief Asks the server for the spouse card (original ShowMarryTab, MarryButtonClick).
            void requestCard();

            /// @brief Handles a message of the server's action 45 (marriage).
            /// @param payload Message payload.
            void receive(const core::Bytes& payload);

        private:
            /// @brief Reads a marriage or divorce application: the form of two players (original RecvMarryApplication, RecvDivorceApplication).
            /// @param payload Message payload.
            /// @param divorce True for a divorce application.
            void receiveApplication(const core::Bytes& payload, bool divorce);

            /// @brief Reads a marriage result code (original RecvMarryResult).
            /// @param payload Message payload.
            void receiveResult(const core::Bytes& payload);

            /// @brief Reads a wedding step (original RecvWeddingStep).
            /// @param payload Message payload.
            void receiveWeddingStep(const core::Bytes& payload);

            /// @brief Reads a divorce result code (original RecvDivorceResult).
            /// @param payload Message payload.
            void receiveDivorceResult(const core::Bytes& payload);

            /// @brief Says a line about the player the id names (original RecvMarryAccepted, RecvMarryRefused, ShowMarryRefusal).
            /// @param payload Message payload.
            /// @param suffix Text put after the player's name.
            void receiveProposalAnswer(const core::Bytes& payload, const char* suffix);

            /// @brief Asks whether to agree to a divorce request (original RecvDivorceRequest).
            /// @param payload Message payload.
            void receiveDivorceRequest(const core::Bytes& payload);

            /// @brief Says the marriage cooldown notice (original RecvMarryCooldown).
            /// @param payload Message payload.
            void receiveCooldown(const core::Bytes& payload);

            /// @brief Tells whether the ceremony going on has one of the two players.
            /// @param first First player id.
            /// @param second Second player id.
            /// @return True when one of them is in the ceremony.
            bool ceremonyOf(std::int32_t first, std::int32_t second) const;

            /// @brief Reads the whole spouse card (original RecvSpouseInfo).
            /// @param payload Message payload.
            void receiveCard(const core::Bytes& payload);

            /// @brief Reads an item the spouse wore or took off (original RecvSpouseEquip).
            /// @param payload Message payload.
            void receiveEquipment(const core::Bytes& payload);

            /// @brief Reads the spouse's location (original RecvSpouseLocation).
            /// @param payload Message payload.
            void receiveLocation(const core::Bytes& payload);

            /// @brief Builds the card's figure from its items.
            void dress();

            /// @brief Shows a short message.
            /// @param text Message text.
            void say(const std::string& text) const { m_hooks.message(text, kMessageMs); }

            /// @brief Gets the name of a player of the scene other than the own one.
            /// @param id Player id.
            /// @return The name, empty when not known.
            std::string playerNameOf(std::int32_t id) const;

            static constexpr int kMessageMs = 2000; // ms a message line stays

            const formats::ItemTable& m_items;
            Hooks m_hooks;
            std::int32_t m_spouseId = 0;
            SpouseCard m_card;
            std::optional<TeamMate> m_applicant;
            std::optional<TeamMate> m_partner;
            std::int32_t m_partnerId = 0;
            int m_divorceItemSlot = 0;
            bool m_accepted = false;
            std::array<std::int32_t, 2> m_wedding{};
            std::optional<TeamMate> m_groom;
            std::optional<TeamMate> m_bride;
    };

}

#endif
