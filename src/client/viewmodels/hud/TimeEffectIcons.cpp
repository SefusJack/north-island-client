#include "client/viewmodels/hud/TimeEffectIcons.h"

#include "ui/DrawingArea.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kFirstIconX = 750; // x of the first (rightmost) icon, px
        constexpr int kIconPitch = 50; // distance between icons, px (they run leftwards)
        constexpr int kIconY = 500 - 10; // y of the icon row, px
        constexpr int kRemoteIcon = 49; // icon size, px (square)
        constexpr int kRemoteTag = 1; // icon tag 1: automatic remote control
        constexpr int kExpTag = 2; // icon tag 2: double exp
        constexpr int kLuckyTag = 3; // icon tag 3: Lucky Grass
        constexpr int kSpaTag = 4; // icon tag 4: Spa Buns
        constexpr int kSandglassTag = 5; // icon tag 5: Magic Sandglass
        constexpr int kWarTag = 6; // icon tag 6: leave-war button
        constexpr int kDreamTag = 9; // icon tag 9: Dreaming Pill
        constexpr int kTrainingTag = 12; // icon tag 12: Training Island item
        constexpr int kBodyTag = 14; // icon tag 14: Riceball (body shift)
        constexpr int kStrengthTag = 16; // icon tag 16: Strength Ability
        constexpr int kStopMark = 31; // size of the "stopped" mark picture, px (square)
        constexpr int kLuckyGrassItem = 34102; // item 34102: 4 Leaves Grass
        constexpr int kSpaBunsItem = 34117; // item 34117: Spa Buns
        constexpr int kSandglassItem = 39074; // item 39074: Magic Sandglass
        constexpr int kSandglassRise = 18; // the sandglass icon sits 18 px higher than the others, px
        constexpr int kDreamingPillItem = 34026; // item 34026: Protective EXP Pill (the "Dreaming Pill" of this code)
        constexpr int kPuppetTag = 13; // icon tag 13: puppet (Memory Card)
        constexpr int kEnterTag = 10; // icon tag 10: enter-instance button
        constexpr int kQuitTag = 11; // icon tag 11: quit-instance button
        constexpr std::size_t kPlaces = 17; // number of icon places (1..17; place 0 unused)

        /// @brief Pixel test of an icon: maps the point through the icon's borders onto the picture before asking for the pixel.
        /// @details Original: TSe_panel.ContainsScreenPoint; an icon is a panel with 6 pixel borders over a 49 x 49 source, and its pixel test (on from TSe_GrBasic.Create) is how the rows and columns 43 come out as the 6th.
        /// @param picture Icon picture.
        /// @param local Point inside the icon.
        /// @return Whether the mapped pixel is opaque.
        bool iconOpaque(const gfx::Picture& picture, core::Point local) {
            static constexpr int kSize = 49; // icon size, px
            static constexpr int kBorder = 6; // panel border, px
            const auto mapped = [](int at) {
                if (at <= kBorder) {
                    return at;
                }
                return at <= kSize - kBorder ? (at - kBorder) % (kSize - 2 * kBorder) + kBorder : at;
            };
            return picture.isOpaque(mapped(local.x), mapped(local.y));
        }

        /// @brief Loads the icons' layout file and returns the Enter-instance button.
        /// @param services Shared game services.
        /// @param screen Screen the form is loaded into.
        /// @return The Enter-instance button.
        ui::ImageButton& loadEnter(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("TimeEffectIcons.xml"), services.loadContext());
            return screen.get<ui::ImageButton>("TimeEffectEnterInstance");
        }

    }

    TimeEffectIcons::TimeEffectIcons(
        GameServices& services,
        ui::Screen& screen,
        const StatusEffects& effects,
        Hooks hooks
    )
        : m_services(services),
          m_hooks(std::move(hooks)),
          m_effects(effects),
          m_enter(loadEnter(services, screen)),
          m_quit(screen.get<ui::ImageButton>("TimeEffectQuitInstance")),
          m_war(screen.get<ui::ImageButton>("TimeEffectLeaveWar")),
          m_remote(screen.get<ui::DrawingArea>("TimeEffectRemote")),
          m_puppet(screen.get<ui::DrawingArea>("TimeEffectPuppet")),
          m_statusIcons{
              {{kExpTag, screen.get<ui::DrawingArea>("TimeEffectExp")},
               {kLuckyTag, screen.get<ui::DrawingArea>("TimeEffectLucky")},
               {kSpaTag, screen.get<ui::DrawingArea>("TimeEffectSpa")},
               {kSandglassTag, screen.get<ui::DrawingArea>("TimeEffectSandglass")},
               {kDreamTag, screen.get<ui::DrawingArea>("TimeEffectDream")},
               {kTrainingTag, screen.get<ui::DrawingArea>("TimeEffectTraining")},
               {kBodyTag, screen.get<ui::DrawingArea>("TimeEffectBody")},
               {kStrengthTag, screen.get<ui::DrawingArea>("TimeEffectStrength")}}
          } {
        screen.get<ui::DrawingArea>("TimeEffectLayout").setPainter([this](gfx::Canvas&, core::Point, std::uint64_t) { layout(); });
        const auto opaqueIn = [&services](std::function<std::optional<gfx::PictureId>()> which) {
            return [&services, which = std::move(which)](core::Point local) {
                const auto shown = which();
                return shown && iconOpaque(services.pictures.picture(*shown), local);
            };
        };
        m_remote.setHitTester(opaqueIn([&services] { return services.pictures.find("AutoplayTool_1"); }));
        m_puppet.setHitTester(opaqueIn([&services] { return services.pictures.find("3225"); }));
        m_remote.setHint("Automatic remote control");
        m_remote.setPainter([&services](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
            if (const auto icon = services.pictures.find("AutoplayTool_1")) {
                canvas.drawPicture(*icon, at, core::Rect{0, 0, kRemoteIcon, kRemoteIcon});
            }
        });
        m_puppet.setHint("Memory Card");
        m_puppet.setPainter([&services](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
            if (const auto icon = services.pictures.find("3225")) {
                canvas.drawPicture(*icon, at, core::Rect{0, 0, kRemoteIcon, kRemoteIcon});
            }
        });
        const auto picture = [&services, this](int tag) -> std::optional<gfx::PictureId> {
            const auto ofItem = [&services](int itemId) {
                const auto item = services.items.find(itemId);
                return item ? services.pictures.find(std::to_string(item->iconId)) : std::nullopt;
            };
            switch (tag) {
                case kExpTag: // exp effect: the icon of the exp item by its rate
                    return ofItem(m_effects.expIconItem());
                case kLuckyTag: // Lucky Grass: its item icon
                    return ofItem(kLuckyGrassItem);
                case kSpaTag: // Spa Buns: its item icon
                    return ofItem(kSpaBunsItem);
                case kSandglassTag: // Magic Sandglass: its item icon
                    return ofItem(kSandglassItem);
                case kDreamTag: // Dreaming Pill: its item icon
                    return ofItem(kDreamingPillItem);
                case kTrainingTag: // Training Island item: item sheet picture 3040
                    return services.pictures.find("3040");
                case kBodyTag: // Riceball (body shift): item sheet picture 3170
                    return services.pictures.find("3170");
                default: // Strength Ability: item sheet picture 3348
                    return services.pictures.find("3348");
            }
        };
        const auto stopped = [this](int tag) {
            // body shift state 2 is the shift being off
            return (tag == kExpTag && m_effects.doubleExpPaused()) || (tag == kTrainingTag && !m_hooks.inTrainingMap()) ||
                   (tag == kBodyTag && m_effects.bodyShiftState() == 2);
        };
        for (const StatusIcon& icon : m_statusIcons) {
            icon.area.setHitTester(opaqueIn([picture, tag = icon.tag] { return picture(tag); }));
            icon.area.setPainter([picture, stopped, &services, tag = icon.tag](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
                if (const auto shown = picture(tag)) {
                    canvas.drawPicture(*shown, at, core::Rect{0, 0, kRemoteIcon, kRemoteIcon});
                }
                if (stopped(tag)) {
                    if (const auto mark = services.pictures.find("StopExp")) {
                        canvas.drawPicture(*mark, at, core::Rect{0, 0, kStopMark, kStopMark});
                    }
                }
            });
        }
        screen.on("onTimeEffectExpClick", [this](const ui::EventArgs&) { m_hooks.askTimeLeft(kExpTag); });
        screen.on("onTimeEffectLuckyClick", [this](const ui::EventArgs&) { m_hooks.askTimeLeft(kLuckyTag); });
        screen.on("onTimeEffectSpaClick", [this](const ui::EventArgs&) { m_hooks.askTimeLeft(kSpaTag); });
        screen.on("onTimeEffectSandglassClick", [this](const ui::EventArgs&) { m_hooks.showSandglass(); });
        screen.on("onTimeEffectDreamClick", [this](const ui::EventArgs&) { m_hooks.askTimeLeft(kDreamTag); });
        screen.on("onTimeEffectTrainingClick", [this](const ui::EventArgs&) { m_hooks.askTimeLeft(kTrainingTag); });
        screen.on("onTimeEffectBodyClick", [this](const ui::EventArgs&) { m_hooks.askTimeLeft(kBodyTag); });
        screen.on("onTimeEffectStrengthClick", [this](const ui::EventArgs&) { m_hooks.askTimeLeft(kStrengthTag); });
        screen.on("onTimeEffectPuppetClick", [this](const ui::EventArgs&) { m_hooks.openPuppet(); });
        screen.on("onTimeEffectRemoteStop", [this](const ui::EventArgs&) { m_hooks.stopRemote(); });
        screen.on("onTimeEffectRemoteClick", [this](const ui::EventArgs&) { m_hooks.openRemote(); });
        screen.on("onTimeEffectEnterInstance", [this](const ui::EventArgs&) { m_hooks.enterInstance(); });
        screen.on("onTimeEffectQuitInstance", [this](const ui::EventArgs&) { m_hooks.quitInstance(); });
        screen.on("onTimeEffectLeaveWar", [this](const ui::EventArgs&) { m_hooks.leaveWar(); });
    }

    void TimeEffectIcons::add(int tag) {
        if (std::find(m_places.begin() + 1, m_places.end(), tag) != m_places.end()) {
            return;
        }
        for (std::size_t place = 1; place <= kPlaces; ++place) {
            if (m_places[place] == 0) {
                m_places[place] = tag;
                return;
            }
        }
    }

    void TimeEffectIcons::remove(int tag) {
        const auto found = std::find(m_places.begin() + 1, m_places.end(), tag);
        if (found != m_places.end()) {
            *found = 0;
        }
    }

    std::string TimeEffectIcons::hintOf(int tag) const {
        switch (tag) {
            case kExpTag: // exp effect: hint from the effects
                return m_effects.expHint();
            case kLuckyTag: // Lucky Grass
                return "Lucky Grass";
            case kSpaTag: // Spa Buns
                return "Using spa buns";
            case kSandglassTag: // Magic Sandglass
                return "Magic Sandglass";
            case kDreamTag: // Dreaming Pill
                return "Dreaming Pill";
            case kTrainingTag: { // Training Island item: named after the item
                // 34180 is the base item id of the Training Island items (34180 Stopwatch, 34181 Training Voucher, ...)
                const auto item = m_services.items.find(34180 + m_effects.trainingItem());
                return item ? item->name : std::string();
            }
            case kBodyTag: // Riceball (body shift)
                return "Transferred Riceball";
            default: // Strength Ability
                return "Strength Ability";
        }
    }

    void TimeEffectIcons::layout() {
        const auto set = [this](int tag, bool active) { active ? add(tag) : remove(tag); };
        set(kRemoteTag, m_hooks.remoteOn());
        set(kLuckyTag, m_effects.luckyGrassOn());
        set(kExpTag, m_effects.doubleExpOn());
        set(kSpaTag, m_effects.spaBunsOn());
        set(kSandglassTag, m_hooks.sandglassOn());
        set(kWarTag, m_hooks.warLeaveShown());
        set(kDreamTag, m_effects.dreamingPillOn());
        set(kQuitTag, m_hooks.instanceState() != 0); // instance state 0: not in an instance
        set(kEnterTag, m_hooks.instanceState() == 2 && !m_hooks.inInstanceMap()); // instance state 2: the instance has started
        set(kTrainingTag, m_effects.trainingItem() != 0);
        set(kPuppetTag, m_hooks.puppetMode() != 0);
        set(kBodyTag, m_effects.bodyShiftState() != 0);
        set(kStrengthTag, m_effects.strengthOn());
        const bool shown = !m_hooks.fighting();
        m_enter.setVisible(false);
        m_quit.setVisible(false);
        m_war.setVisible(false);
        m_remote.setVisible(false);
        m_puppet.setVisible(false);
        for (const StatusIcon& icon : m_statusIcons) {
            icon.area.setVisible(false);
        }
        int x = kFirstIconX;
        for (std::size_t place = kPlaces; place >= 1; --place) {
            const int tag = m_places[place];
            if (tag == 0) {
                continue;
            }
            if (tag == kRemoteTag) {
                m_remote.setBounds({x, kIconY, kRemoteIcon, kRemoteIcon});
                m_remote.setVisible(true);
            } else if (
                const auto statusIcon = std::find_if(m_statusIcons.begin(), m_statusIcons.end(), [tag](const StatusIcon& each) { return each.tag == tag; });
                statusIcon != m_statusIcons.end()
            ) {
                if (tag == kSandglassTag && !m_hooks.inHouseRoom()) {
                    continue;
                }
                statusIcon->area.setHint(hintOf(tag));
                statusIcon->area.setBounds({x, tag == kSandglassTag ? kIconY - kSandglassRise : kIconY, kRemoteIcon, kRemoteIcon});
                statusIcon->area.setVisible(true);
            } else if (tag == kPuppetTag) {
                m_puppet.setBounds({x, kIconY, kRemoteIcon, kRemoteIcon});
                m_puppet.setVisible(true);
            } else if (shown) {
                ui::ImageButton& icon = tag == kEnterTag ? m_enter : tag == kWarTag ? m_war : m_quit;
                icon.setPosition({x, kIconY});
                icon.setVisible(true);
            } else {
                continue;
            }
            x -= kIconPitch;
        }
    }

}
