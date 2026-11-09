#ifndef NORTH_ISLAND_CLIENT_SERVICES_PARTY_ACCOUNT_BUNDLE_H
#define NORTH_ISLAND_CLIENT_SERVICES_PARTY_ACCOUNT_BUNDLE_H

#include "audio/AudioDevice.h"
#include "audio/MusicPlayer.h"
#include "audio/SoundEffects.h"
#include "client/scenes/Scene.h"
#include "client/scenes/SceneKind.h"
#include "client/services/AccountSession.h"
#include "client/services/GameServices.h"
#include "client/rendering/world/ViewZoom.h"
#include "core/Log.h"
#include "net/GameConnection.h"

#include <memory>

namespace client {

    /// @brief References to the game-wide data every account's services share (folder, pictures, tables, character renderer).
    struct GameTables {
        const GameDirectory& game;
        const formats::StartupInfo& startup;
        gfx::PictureLibrary& pictures;
        const formats::FormulaData& formula;
        const formats::ItemTable& items;
        formats::CompoundTable& compounds;
        const formats::NpcTable& npcs;
        const formats::SkillTable& skills;
        const formats::MotionTable& motions;
        const formats::TalkTable& talks;
        const formats::MarkTable& marks;
        CharacterRenderer& characters;

        /// @brief Collects the shared tables out of a set of game services.
        /// @param services The services whose shared tables are referenced.
        /// @return A GameTables that refers to the same tables as the services.
        static GameTables of(const GameServices& services) {
            return {
                services.game,
                services.startup,
                services.pictures,
                services.formula,
                services.items,
                services.compounds,
                services.npcs,
                services.skills,
                services.motions,
                services.talks,
                services.marks,
                services.characters
            };
        }
    };

    /// @brief One account's own objects (sound, connection, session, zoom, services and scene), swapped whole when a party member takes the lead.
    struct AccountBundle {
        /// @brief Builds the account's objects; the message box only logs and screenshots go nowhere until the owner sets them.
        /// @param tables The shared game tables.
        /// @param device The sound device (the open one for the played account, a silent one for a party member).
        AccountBundle(
            const GameTables& tables,
            const audio::AudioDevice& device
        )
            : sounds(device),
              music(device),
              services{
                  tables.game,
                  tables.startup,
                  tables.pictures,
                  sounds,
                  music,
                  connection,
                  tables.formula,
                  tables.items,
                  tables.compounds,
                  tables.npcs,
                  tables.skills,
                  tables.motions,
                  tables.talks,
                  tables.marks,
                  tables.characters,
                  session,
                  zoom,
                  [](const std::string& caption, const std::string& text) { core::logInfo("message box: ", caption, " / ", text); },
                  [](const std::filesystem::path&, const PhotoCheck&) {}
              } {}

        /// @brief Drops the scene and closes the connection.
        ~AccountBundle() {
            scene.reset();
            connection.close();
        }

        AccountBundle(const AccountBundle&) = delete;
        AccountBundle& operator=(const AccountBundle&) = delete;

        audio::SoundEffects sounds;
        audio::MusicPlayer music;
        net::GameConnection connection;
        AccountSession session;
        ViewZoom zoom;
        GameServices services;
        std::unique_ptr<Scene> scene;
        SceneKind kind = SceneKind::Loading;
    };

}

#endif
