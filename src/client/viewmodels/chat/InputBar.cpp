#include "client/viewmodels/chat/InputBar.h"

#include "client/data/EmoteCodes.h"
#include "client/rules/GmIds.h"
#include "ui/DrawingArea.h"
#include "ui/ImageView.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kCycledChannels = 4; // clicking the channel button cycles channels 1..4 (5 is only picked from the list)
        constexpr int kChannels = 5; // chat channels 1..5 (3 whisper, 4 team, 5 guild)
        constexpr std::size_t kHistory = 10; // sent lines remembered for the up/down recall
        constexpr std::uint64_t kMailFrameMs = 400; // new-mail icon blink interval, ms
        constexpr std::size_t kWhisperNames = 10; // whisper target names kept in the drop-down
        constexpr int kCharWidth = 8; // width of one typed character, px
        constexpr int kIconSize = 16; // emote icon size, px
        constexpr std::uint64_t kIconFrameMs = 210; // emote icon animation frame time, ms
        constexpr std::size_t kLineLongest = 60; // longest chat line, characters

        /// @brief Control name of an emote button.
        /// @param number Emote number.
        /// @return "InputEmote" followed by the number.
        std::string emoteButton(int number) { return "InputEmote" + std::to_string(number); }

        /// @brief Control name of a channel pick button.
        /// @param row Row 1 .. 4.
        /// @return "InputChannelPick" followed by the row.
        std::string pickName(int row) { return "InputChannelPick" + std::to_string(row); }

    }

    InputBar::InputBar(
        GameServices& services,
        ui::Screen& screen,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_hooks(std::move(hooks)),
          m_panel(screen.load(services.game.layout("InputBar.xml"), services.loadContext())),
          m_channelMark(screen.get<ui::ImageView>("InputChannelMark")),
          m_mail(screen.get<ui::ImageView>("InputMail")),
          m_channelButton(screen.get<ui::ImageButton>("InputChannel")),
          m_quitViewing(screen.get<ui::ImageButton>("InputQuitViewing")),
          m_channelFrame(screen.get<ui::Element>("InputChannelFrame")),
          m_text(screen.get<ui::TextBox>("InputText")),
          m_whisperName(screen.get<ui::TextBox>("InputWhisperName")),
          m_whisperNames(screen.get<ui::ListBox>("InputWhisperNames")),
          m_mailFrameSpot(screen.get<ui::Spot>("InputMailFrame")),
          m_buttonSpot(screen.get<ui::Spot>("InputChannelButtonArea")),
          m_frameSpot(screen.get<ui::Spot>("InputChannelFrameArea")),
          m_nameFieldSpot(screen.get<ui::Spot>("InputWhisperFieldArea")),
          m_chosenNameSpot(screen.get<ui::Spot>("InputWhisperChosenField")),
          m_namesPanelSpot(screen.get<ui::Spot>("InputWhisperPanel")),
          m_namesInPanelSpot(screen.get<ui::Spot>("InputWhisperNamesInPanel")),
          m_lineEmotesSpot(screen.get<ui::Spot>("InputLineEmotes")) {
        screen.on("onInputChannel", [this](const ui::EventArgs&) { cycleChannel(); });
        screen.on("onInputQuitViewing", [this](const ui::EventArgs&) { m_hooks.quitViewing(); });
        screen.on("onInputChannelPick", [this](const ui::EventArgs& args) { pickChannel(args.source.back() - '0'); });
        screen.on("onInputSend", [this](const ui::EventArgs&) { sendLine(); });
        screen.on("onInputTextClick", [this](const ui::EventArgs&) { textClicked(); });
        screen.on("onInputWhisperEnter", [this](const ui::EventArgs&) { resolveWhisperTarget(); });
        screen.on("onInputWhisperClick", [this](const ui::EventArgs&) { toggleWhisperNames(); });
        screen.on("onInputWhisperPick", [this](const ui::EventArgs& args) { pickWhisperName(args.index); });
        screen.on("onInputEmotes", [this](const ui::EventArgs&) { showEmotes(!m_screen.get<ui::ImageView>("InputEmotePanel").isVisible()); });
        screen.on("onInputEmoteClose", [this](const ui::EventArgs&) { showEmotes(false); });
        screen.on("onInputEmotePick", [this](const ui::EventArgs& args) { pickEmote(std::stoi(args.source.substr(std::string("InputEmote").size()))); });
        for (int number = 1; number <= kEmoteCount; ++number) {
            screen.get<ui::ImageButton>(emoteButton(number)).setHint(emoteCode(number) + emoteName(number));
        }
        m_text.setShownText([](const std::string& visible) {
            std::string blanked = visible;
            for (std::size_t at = 0; at < blanked.size();) {
                const int emote = emoteAt(visible, at);
                const std::size_t length = emote != 0 ? emoteCode(emote).size() : 1;
                if (emote != 0) {
                    blanked.replace(at, length, length, ' ');
                }
                at += length;
            }
            return blanked;
        });
        ui::DrawingArea& textEmotes = screen.get<ui::DrawingArea>("InputTextEmotes");
        textEmotes.setPainter([this, &textEmotes](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
            paintTextEmotes(canvas, at - textEmotes.bounds().position(), nowMs);
        });
        screen.on("onInputPrevious", [this](const ui::EventArgs&) { recall(-1); });
        screen.on("onInputNext", [this](const ui::EventArgs&) { recall(1); });
        screen.on("onInputMail", [this](const ui::EventArgs&) { m_hooks.openMailBox(); });
        setChannel(2); // the bar starts on channel 2, local
    }

    void InputBar::setChannel(int channel) {
        m_whisperName.setReadOnly(true);
        m_whisperName.setVisible(false);
        // channel ids: 3 whisper, 4 team, 5 guild; a refused team/guild falls back to 2 (local)
        if (channel != 3) {
            m_whisperName.clear();
            m_whisperTarget = 0;
        }
        if (channel == 3) {
            m_whisperName.setReadOnly(false);
            m_whisperName.setVisible(true);
            m_screen.focus("InputWhisperName");
        }
        if ((channel == 4 || channel == 5) && !m_hooks.channelAllowed(channel)) {
            setChannel(2);
            return;
        }
        m_channel = channel;
        const std::string number = std::to_string(channel);
        m_channelMark.setPicture(m_services.pictures.find("btn_channel_" + number).value_or(gfx::PictureId{}));
        m_channelButton.setPicture(m_services.pictures.find("btn_channel_" + number + "_1").value_or(gfx::PictureId{}));
    }

    void InputBar::cycleChannel() {
        if (m_channelFrame.isVisible() || m_buttonSpot.rect().contains(panelMouse()) || m_channel < 1 || m_channel > kCycledChannels) {
            return;
        }
        setChannel(m_channel < kCycledChannels ? m_channel + 1 : 1);
    }

    void InputBar::pickChannel(int row) {
        if (row >= 1 && row <= kPickRows) {
            setChannel(m_pickChannels[static_cast<std::size_t>(row)]);
            m_channelFrame.setVisible(false);
            for (int pick = 1; pick <= kPickRows; ++pick) {
                m_screen.get<ui::ImageButton>(pickName(pick)).setVisible(false);
            }
        }
    }

    void InputBar::update(std::uint64_t nowMs, bool newMail, bool watching) {
        m_quitViewing.setVisible(watching);
        if (!newMail) {
            m_mailFrame = 0;
        } else if (nowMs > m_mailFrameMs + kMailFrameMs) {
            m_mailFrame = 1 - m_mailFrame;
            m_mailFrameMs = nowMs;
        }
        m_mail.setSourceRect(core::Rect{0, m_mailFrame * m_mailFrameSpot.size().height, m_mailFrameSpot.size().width, m_mailFrameSpot.size().height});

        const core::Point mouse = panelMouse();
        const bool open = m_buttonSpot.rect().contains(mouse) || (m_channelFrame.isVisible() && m_frameSpot.rect().contains(mouse));
        m_channelFrame.setVisible(open);
        int row = 0;
        for (int channel = 1; channel <= kChannels; ++channel) {
            if (channel != m_channel && row < kPickRows) {
                ++row;
                m_pickChannels[static_cast<std::size_t>(row)] = channel;
                m_screen.get<ui::ImageButton>(pickName(row))
                    .setPicture(m_services.pictures.find("btn_channel_" + std::to_string(channel) + "_1").value_or(gfx::PictureId{}));
            }
        }
        for (int pick = 1; pick <= kPickRows; ++pick) {
            m_screen.get<ui::ImageButton>(pickName(pick)).setVisible(open);
        }
        if (m_whisperNames.itemCount() > 0 && m_channel == 3) { // channel 3 is whisper
            const core::Rect names = m_whisperNames.bounds();
            const core::Rect panel{
                m_namesPanelSpot.rect().x,
                names.y - m_namesInPanelSpot.at().y,
                m_namesPanelSpot.rect().width,
                names.height + 12
            }; // panel border below the names
            m_whisperNames.setVisible(m_nameFieldSpot.rect().contains(mouse) || (m_whisperNames.isVisible() && panel.contains(mouse)));
        }
    }

    void InputBar::resolveWhisperTarget() {
        const std::string name = m_whisperName.text();
        if (name.empty()) {
            m_hooks.notice("Target unavailable to whisper", 0);
            m_whisperTarget = 0;
            return;
        }
        const auto found = m_hooks.playerNamed(name);
        if (!found) {
            m_hooks.notice("\"Target unavailable . Unable to whisper.\"", 0);
            m_whisperTarget = 0;
            m_whisperName.clear();
            return;
        }
        setWhisperTarget(*found);
    }

    void InputBar::setWhisperTarget(std::int32_t playerId, bool fromHead) {
        if (!m_hooks.targetChosen(playerId)) {
            return;
        }
        if (playerId == m_whisperTarget) {
            return;
        }
        m_whisperTarget = playerId;
        if (m_whisperTarget == 0) {
            return;
        }
        if (m_whisperTarget == m_hooks.ownId()) {
            m_whisperTarget = 0;
            return;
        }
        if (isGmId(m_whisperTarget)) {
            m_whisperName.clear();
            m_whisperTarget = 0;
            return;
        }
        const std::string name = m_hooks.playerName(m_whisperTarget);
        if (name.empty()) {
            return;
        }
        if (!fromHead) {
            m_hooks.notice("target to whisper<<" + name + ">>", m_whisperTarget);
        }
        setChannel(3); // channel 3 is whisper
        addWhisperName(name, true);
        m_whisperName.setText(name);
        m_whisperName.setReadOnly(false);
        m_whisperName.setBounds(m_chosenNameSpot.rect());
    }

    void InputBar::addWhisperName(const std::string& name, bool toEnd) {
        const int at = m_whisperNames.indexOf(name);
        if (at == -1) {
            if (toEnd || m_whisperNames.itemCount() == 0) {
                m_whisperNames.addItem(name);
            } else {
                m_whisperNames.insertItem(m_whisperNames.itemCount() - 1, name);
            }
            if (m_whisperNames.itemCount() > static_cast<int>(kWhisperNames)) {
                m_whisperNames.removeItem(0);
            }
            placeWhisperNames();
        } else if (toEnd) {
            m_whisperNames.removeItem(at);
            m_whisperNames.addItem(name);
        }
    }

    void InputBar::showEmotes(bool shown) {
        m_screen.get<ui::ImageView>("InputEmotePanel").setVisible(shown);
        for (int number = 1; number <= kEmoteCount; ++number) {
            m_screen.get<ui::ImageButton>(emoteButton(number)).setVisible(shown);
        }
        m_screen.get<ui::ImageButton>("InputEmoteClose").setVisible(shown);
    }

    void InputBar::pickEmote(int number) {
        if (m_text.text().size() > kLineLongest - 2) { // an emote code is two characters
            return;
        }
        if (m_emotesToMail) {
            m_hooks.emoteToMail(emoteCode(number));
        } else {
            m_text.setText(m_text.text() + emoteCode(number));
        }
    }

    void InputBar::paintTextEmotes(gfx::Canvas& canvas, core::Point origin, std::uint64_t nowMs) {
        const std::string& text = m_text.text();
        const std::size_t first = m_text.firstShown();
        const std::size_t last = first + static_cast<std::size_t>(m_text.visibleChars());
        const core::Rect source{0, static_cast<int>(nowMs / kIconFrameMs % 2) * kIconSize, kIconSize, kIconSize}; // emote icons have two frames
        for (std::size_t at = 0; at < text.size() && at <= last;) {
            const int emote = emoteAt(text, at);
            if (emote == 0 || at < first) {
                ++at;
                continue;
            }
            if (const auto icon = m_services.pictures.find("icon_expre_" + std::to_string(emote))) {
                canvas.drawPicture(*icon, origin + m_lineEmotesSpot.at() + core::Point{static_cast<int>(at - first) * kCharWidth, 0}, source);
            }
            at += emoteCode(emote).size();
        }
    }

    void InputBar::removeWhisperName(const std::string& name) {
        const int at = m_whisperNames.indexOf(name);
        if (at != -1) {
            m_whisperNames.removeItem(at);
            placeWhisperNames();
        }
    }

    void InputBar::placeWhisperNames() {
        const int height = m_whisperNames.itemCount() * m_namesPanelSpot.rect().height;
        m_whisperNames.setBounds(
            {m_namesPanelSpot.rect().x + m_namesInPanelSpot.at().x,
             m_namesPanelSpot.rect().y - (height + 12) + m_namesInPanelSpot.at().y,
             m_namesPanelSpot.rect().width,
             height}
        ); // panel border below the names
    }

    void InputBar::pickWhisperName(int index) {
        if (index >= 0 && index < m_whisperNames.itemCount() && m_whisperName.text() != m_whisperNames.itemText(index)) {
            m_whisperName.setText(m_whisperNames.itemText(index));
            resolveWhisperTarget();
        }
        m_whisperNames.setVisible(false);
    }

    void InputBar::toggleWhisperNames() {
        if (m_whisperNames.itemCount() > 0) {
            m_whisperNames.setVisible(!m_whisperNames.isVisible());
        }
        if (!m_whisperNames.isVisible() && !m_whisperName.text().empty()) {
            m_screen.focus("InputText");
        }
    }

    void InputBar::textClicked() {
        if (m_channel == 3 && !m_whisperName.text().empty()) { // channel 3 is whisper
            resolveWhisperTarget();
            m_screen.focus("InputText");
        }
    }

    void InputBar::sendLine() {
        const std::string text = m_text.text();
        if (text.empty()) {
            return;
        }
        const Sent sent = m_hooks.send(m_channel, text);
        if (sent == Sent::Kept) {
            return;
        }
        if (sent == Sent::Sent) {
            if (m_history.size() >= kHistory) {
                m_history.erase(m_history.begin());
                m_historyCount = static_cast<int>(kHistory);
            } else {
                ++m_historyCount;
            }
            m_historyAt = m_historyCount;
            m_history.push_back(text);
        }
        m_text.clear();
    }

    void InputBar::recall(int step) {
        if (m_historyCount == 0) {
            return;
        }
        if (step < 0) {
            m_text.setText(m_history[static_cast<std::size_t>(m_historyAt - 1)]);
            if (--m_historyAt < 1) {
                m_historyAt = m_historyCount;
            }
            return;
        }
        if (++m_historyAt > m_historyCount) {
            m_historyAt = 1;
        }
        m_text.setText(m_history[static_cast<std::size_t>(m_historyAt - 1)]);
    }

}
