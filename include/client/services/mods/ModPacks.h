#ifndef NORTH_ISLAND_CLIENT_SERVICES_MODS_MOD_PACKS_H
#define NORTH_ISLAND_CLIENT_SERVICES_MODS_MOD_PACKS_H

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace formats {
    class ItemTable;
    class NpcTable;
    class SkillTable;
    class TalkTable;
}

namespace gfx {
    class PictureLibrary;
}

namespace client {

    class RoleSpriteLibrary;

    /// @brief One pack of mods\ (its mod.json): what it is and where.
    struct ModPack {
        std::string id;
        std::string name;
        std::string version;
        std::string author;
        std::string description;
        std::vector<std::string> needs;
        std::vector<std::string> loadAfter;
        std::filesystem::path folder;
    };

    /// @brief The qol client's asset packs: folders of mods\ next to the program that replace the game's sounds, pictures and text without touching the install.
    /// @details mods\load-order.json lists them, later ones winning ({"mods": [{"id": ..., "enabled": true}]}); a pack not listed there is added to it turned off. What each pack changed and hid is in the report (the log, --mods-report). A pack has:
    ///   sounds\ the install's sound\ folder (wav0154.ogg for sound\wav0154.wav: .wav, .ogg, .flac or .mp3), mp3s\ its Mp3s\;
    ///   pictures\ pictures by the game's names (Btn_Next.png; PNG, BMP or JPG of the original's size); pictures\@2x\ the same at twice the size each way (HD: drawn at the original's size, detail shown at a pixel scale above 1; used over a same-size one of the name);
    ///   sprites\@2x\<bank>\<sprite>\<frame>.png HD sprite frames: 8-bit paletted PNGs of the frame's palette indices at twice its size each way (the frame: its place in the sprite's frame table);
    ///   fonts\@2x\<font file>.png an HD font (tatpc1.twn.png): the font's atlas (formats::FontAtlasLayout), at twice the size each way;
    ///   text\ Item.json, Skill.json ({"id": {"name", "description"}}), Npc.json ({"id": {"name"}}), Talk.json ({"id": "line"}).
    class ModPacks {
        public:
            /// @brief Reads the packs of the mods folder and its load order, writing unlisted packs into load-order.json turned off.
            /// @param modsRoot The mods\ folder.
            /// @return The loaded packs; none when the folder is not there (or --no-mods).
            static ModPacks load(const std::filesystem::path& modsRoot);

            /// @brief The enabled packs in load order.
            /// @return The packs.
            const std::vector<ModPack>& packs() const { return m_packs; }

            /// @brief The report lines of what was loaded, changed, hidden or refused.
            /// @return The lines.
            const std::vector<std::string>& report() const { return m_report; }

            /// @brief Tells whether no pack is enabled.
            /// @return True when there are no packs.
            bool empty() const { return m_packs.empty(); }

            /// @brief Finds the pack file for a sound of the install ("Sound/wav0154.wav", any case and slashes).
            /// @param relative Sound path relative to the install.
            /// @return The file, nothing when no pack has it.
            std::optional<std::filesystem::path> sound(const std::string& relative) const;

            /// @brief Adds the packs' pictures to the library, last pack winning (HD ones after the same-size ones).
            /// @param pictures Library that receives the replacements.
            void addPictures(gfx::PictureLibrary& pictures);

            /// @brief Adds the packs' HD sprite frames to the sprite library, last pack winning.
            /// @param sprites Library that receives the frames.
            void addSprites(RoleSpriteLibrary& sprites) const;

            /// @brief Finds a pack's HD atlas of a font file of the install ("tatpc1.twn"), last pack winning.
            /// @param fontFile Font file name.
            /// @return The atlas file, nothing when no pack has it.
            std::optional<std::filesystem::path> hdFont(const std::string& fontFile) const;

            /// @brief Applies the packs' text replacements to the game's tables.
            /// @param items Item table.
            /// @param skills Skill table.
            /// @param npcs Npc table.
            /// @param talks Talk table.
            /// @param simplified True when the game's text is GBK (else Big5).
            void applyText(
                formats::ItemTable& items,
                formats::SkillTable& skills,
                formats::NpcTable& npcs,
                formats::TalkTable& talks,
                bool simplified
            );

            /// @brief Gives the key a sound path is known by ("sound/wav0154").
            /// @param relative Sound path.
            /// @return The key, empty for a path outside sound\ and Mp3s\ (tests).
            static std::string soundKey(const std::string& relative);

        private:
            /// @brief A pack file and the id of the pack it came from.
            struct Entry {
                std::filesystem::path file;
                std::string pack;
            };

            /// @brief Indexes the sounds, pictures, HD pictures, fonts and sprite frames of a pack and reports the counts.
            /// @param pack The pack.
            void index(const ModPack& pack);

            /// @brief Stores an entry under a key, reporting when it hides another pack's entry.
            /// @param entries Map to store into.
            /// @param kind Kind name for the report.
            /// @param key Entry key.
            /// @param entry The entry.
            void put(std::map<std::string, Entry>& entries, const std::string& kind, const std::string& key, Entry entry);

            std::vector<ModPack> m_packs;
            std::vector<std::string> m_report;
            std::map<std::string, Entry> m_sounds;
            std::map<std::string, Entry> m_pictures;
            std::map<std::string, Entry> m_hdPictures;
            std::map<std::string, Entry> m_hdSprites;
            std::map<std::string, Entry> m_hdFonts;
    };

    /// @brief Converts the UTF-8 of a pack to the game's text (Big5, or GBK when simplified), so the game's fonts draw it.
    /// @param utf8 The pack's text.
    /// @param simplified True for GBK, false for Big5.
    /// @param lost Incremented when a character is not in the code page (it becomes '?').
    /// @return The text in the game's code page.
    std::string gameTextFromUtf8(const std::string& utf8, bool simplified, int& lost);

}

#endif
