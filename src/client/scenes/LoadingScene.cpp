#include "client/scenes/LoadingScene.h"

#include "core/Log.h"

namespace client {

    namespace {

        /// @brief A group of picture archives sharing an extension; large ones are skipped in "ShortMemory" mode.
        struct ArchiveSet {
            bool largeMemoryOnly;
            const char* extension;
            std::vector<const char*> files;
        };

        /// @brief Lists the picture archives in the order the original opens them (a later archive wins for a name both hold).
        /// @return The archive sets.
        const std::vector<ArchiveSet>& pictureArchives() {
            static const std::vector<ArchiveSet> sets = {
                {false, ".BMg", {"images", "images_1", "images_2", "images_3", "images_01", "images_a01", "images_a02", "images_b01", "images_c01"}},
                {true,
                 ".BMg",
                 {"images1",
                  "images1_1",
                  "images1_2",
                  "images1_3",
                  "images1_01",
                  "images1_02",
                  "images1_test",
                  "images4",
                  "images4_a01",
                  "images4_b01",
                  "images4_b02",
                  "images4_c01",
                  "images4_d01"}},
                {false, ".BMg", {"images2", "images2_1", "images2_2", "images2_01"}},
                {true, ".BMg", {"images3", "images3_1", "images3_01", "images3_a01", "images3_b01", "images3_c01"}},
                {false, ".BMg", {"item",      "item_1",    "item_01",   "item_a01",  "item_a02",  "item_b01",  "item_c01",  "item_d01",
                                 "item1",     "item1_1",   "item1_2",   "item1_01",  "item1_a01", "item1_a02", "item1_b01", "item1_b02",
                                 "item1_c01", "item1_c02", "item1_d01", "item1_d02", "item1_d03", "item2",     "item2_1",   "item2_01",
                                 "item2_a01", "item2_b01", "item2_c01", "item3",     "item3_a01", "item3_b01", "item3_c01"}},
                {false, ".JMg", {"map", "map_01", "map_02", "map_a01", "map_b01", "map_c01", "map_d01", "jpeg", "jpeg_01", "jpeg_a01", "jpeg_b01"}},
                {true, ".JMg", {"jRole", "jRole_1", "jRole_2", "jRole_01", "jRole_a01", "jRole_b01", "jRole_b02", "jRole_c01"}},
            };
            return sets;
        }

    }

    LoadingScene::LoadingScene(
        GameServices& services
    )
        : m_services(services),
          m_screen(services.pictures),
          m_layout(m_screen.load(services.game.layout("LoadingScene.xml"), services.loadContext())),
          m_progress(m_screen.get<ui::ProgressBar>("Progress")) {
        m_steps.push_back([this] { m_services.pictures.addDirectory(m_services.game.data("menu/skins/white")); });
        m_steps.push_back([this] { m_services.pictures.addDirectory(m_services.game.data("menu/skins/default")); });
        const bool shortMemory = m_services.startup.memory == formats::StartupInfo::MemoryMode::Short;
        for (const ArchiveSet& set : pictureArchives()) {
            if (set.largeMemoryOnly && shortMemory) {
                continue;
            }
            for (const char* name : set.files) {
                m_steps.push_back([this, name, extension = set.extension] {
                    const auto file = m_services.game.data(std::string("pic/") + name + extension);
                    if (std::filesystem::exists(file)) {
                        m_services.pictures.addArchive(file);
                    }
                });
            }
        }
        m_services.music.playLooped(m_services.game.data("Sound/BGM0013.wav"));
    }

    void LoadingScene::update(std::uint64_t nowMs) {
        if (m_nextStep < m_steps.size()) {
            m_steps[m_nextStep++]();
            m_progress.setValue(m_progress.value() + 1);
            if (m_nextStep == m_steps.size()) {
                core::logInfo("loading done: ", m_services.pictures.pictureCount(), " pictures in use");
            }
        }
        m_screen.update(nowMs);
    }

    void LoadingScene::draw(gfx::Canvas& canvas, std::uint64_t nowMs) { m_screen.draw(canvas, nowMs); }

    std::optional<SceneKind> LoadingScene::nextScene() const {
        if (m_nextStep < m_steps.size()) {
            return std::nullopt;
        }
        return SceneKind::Login;
    }

}
