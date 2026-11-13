#include "client/scenes/LoadingScene.h"

#include "core/Log.h"
#include "core/Speedups.h"
#include "gfx/RenderDevice.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <string>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace client {

    namespace {

        // Speedups::rememberRendererChecks: which renderers start on this machine is remembered in the user folder.
        // The answers are kept for the same graphics card for a week, then asked again (a driver update can change them).
        struct RendererAnswers {
            std::filesystem::path file;
            std::string machine;
            std::map<std::string, bool> known;
            bool changed = false;
        };

        /// @brief Names the graphics card, so answers from another machine or card are not used.
        std::string graphicsCard() {
#ifdef _WIN32
            DISPLAY_DEVICEA device{};
            device.cb = sizeof(device);
            for (DWORD i = 0; EnumDisplayDevicesA(nullptr, i, &device, 0); ++i) {
                if ((device.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) != 0) {
                    return std::string(device.DeviceString) + " " + device.DeviceID;
                }
            }
#endif
            return "unknown";
        }

        /// @brief Reads the remembered answers, when they are for this card and less than a week old.
        std::shared_ptr<RendererAnswers> readAnswers(const std::filesystem::path& file) {
            auto answers = std::make_shared<RendererAnswers>();
            answers->file = file;
            answers->machine = graphicsCard();
            std::error_code error;
            const auto written = std::filesystem::last_write_time(file, error);
            if (error || std::filesystem::file_time_type::clock::now() - written > std::chrono::hours(24 * 7)) {
                return answers;
            }
            std::ifstream in(file);
            std::string line;
            if (!std::getline(in, line) || line != "card=" + answers->machine) {
                return answers;
            }
            while (std::getline(in, line)) {
                const std::size_t equals = line.find('=');
                if (equals != std::string::npos && equals + 1 < line.size()) {
                    answers->known[line.substr(0, equals)] = line[equals + 1] == '1';
                }
            }
            return answers;
        }

        void writeAnswers(const RendererAnswers& answers) {
            std::ofstream out(answers.file, std::ios::trunc);
            out << "card=" << answers.machine << '\n';
            for (const auto& [driver, starts] : answers.known) {
                out << driver << '=' << (starts ? '1' : '0') << '\n';
            }
        }

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
        // the renderers in the build that cannot start on this machine leave the Video tab's list, one tried a step (first, while the bar (30 steps long) still moves)
        const auto answers = core::speedups().rememberRendererChecks ? readAnswers(m_services.game.user("renderer-check.txt")) : std::shared_ptr<RendererAnswers>();
        std::vector<std::string> cannotStart;
        for (const std::string& driver : m_services.renderDrivers) {
            if (driver == m_services.rendererInUse || driver == "software") {
                continue; // running already, or always there
            }
            if (const auto known = answers ? answers->known.find(driver) : std::map<std::string, bool>::iterator{};
                answers && known != answers->known.end()) {
                if (!known->second) {
                    cannotStart.push_back(driver);
                }
                continue; // remembered from an earlier launch
            }
            m_steps.push_back([this, driver, answers] {
                const bool starts = gfx::RenderDevice::driverStarts(driver);
                if (answers) {
                    answers->known[driver] = starts;
                    answers->changed = true;
                }
                if (!starts) {
                    auto& drivers = m_services.renderDrivers;
                    drivers.erase(std::remove(drivers.begin(), drivers.end(), driver), drivers.end());
                }
            });
        }
        for (const std::string& driver : cannotStart) {
            auto& drivers = m_services.renderDrivers;
            drivers.erase(std::remove(drivers.begin(), drivers.end(), driver), drivers.end());
        }
        if (answers) {
            core::logInfo("renderer checks remembered: ", answers->known.size(), ", to try now: ", m_steps.size());
        }
        m_driverSteps = m_steps.size();
        if (answers) {
            m_steps.push_back([answers] {
                if (answers->changed) {
                    writeAnswers(*answers);
                }
            });
        }
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
        // every step at once: together they take about 12 ms, while one step a 30 ms beat held the bar up for 2.5 s
        // (most of the 84 steps are archive names an install does not have)
        // The renderer tries stay one an update, as before: run before the first frame is shown, they left Direct3D 11's
        // present about twice as slow on a test PC (0.65 against 0.35 ms a frame) for the whole session.
        if (m_nextStep < m_driverSteps) {
            m_steps[m_nextStep++]();
            m_progress.setValue(m_progress.value() + 1);
        } else if (m_nextStep < m_steps.size()) {
            while (m_nextStep < m_steps.size()) {
                m_steps[m_nextStep++]();
                m_progress.setValue(m_progress.value() + 1);
            }
            core::logInfo("loading done: ", m_services.pictures.pictureCount(), " pictures in use");
            std::string drivers;
            for (const std::string& driver : m_services.renderDrivers) {
                drivers += (drivers.empty() ? "" : ", ") + driver;
            }
            core::logInfo("renderers available: ", drivers);
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
