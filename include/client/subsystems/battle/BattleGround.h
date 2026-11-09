#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_BATTLE_GROUND_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_BATTLE_GROUND_H

#include "client/subsystems/battle/BattleCamera.h"
#include "core/Color.h"
#include "formats/SceneTable.h"
#include "gfx/Canvas.h"
#include "gfx/PictureLibrary.h"

#include <optional>

namespace client {

    /// @brief The picture behind a battle: a backdrop with a far and a near layer, seen from the camera.
    /// @details Original: CH_TBattleGround, as SceneData.Dat gives them. The backdrop stays, the far layer moves at its own pace
    /// (BattleCamera::farX), the near one with the camera.
    class BattleGround {
        public:
            /// @brief Creates an empty ground.
            /// @param pictures Picture library to load from.
            explicit BattleGround(
                gfx::PictureLibrary& pictures
            )
                : m_pictures(pictures) {}

            /// @brief Loads the ground pictures in the map's tint.
            /// @details Original: LoadBattleGround; SceneData 59000 + ground (59081 when there is none).
            /// @param scenes Scene table.
            /// @param ground Ground number.
            /// @param tint Map tint.
            void load(const formats::SceneTable& scenes, int ground, core::Color tint);

            /// @brief Draws the ground; while the screen shakes the far layer moves by half the shake, the near one by all of it.
            /// @details Original: DrawBattleGround.
            /// @param canvas Target canvas.
            /// @param camera The camera.
            /// @param shake Screen shake offset.
            void draw(gfx::Canvas& canvas, const BattleCamera& camera, core::Point shake) const;

            /// @brief Where the near layer's top is drawn without a shake.
            /// @details Original: DrawBattleGround keeps it at or below layerY - 232.
            /// @param layerY The layer's y.
            /// @param cameraY The camera's y.
            /// @return Top y.
            static int nearTop(int layerY, int cameraY);

            /// @brief The near layer's size, the ground the camera keeps within.
            /// @return Size, 0 x 0 when none.
            core::Point size() const;

            /// @brief The near layer's bottom on the ground: its height plus its y.
            /// @details Original: FLayer1Height + FLayer1OffsetY (a missing picture has height 0), as ApplySkillZoom reads them.
            /// @return Bottom y.
            int nearBottom() const { return size().y + m_nearY; }

        private:
            gfx::PictureLibrary& m_pictures;
            std::optional<gfx::PictureId> m_backdrop;
            std::optional<gfx::PictureId> m_far;
            std::optional<gfx::PictureId> m_near;
            int m_farY = 0;
            int m_nearY = 0;
    };

}

#endif
