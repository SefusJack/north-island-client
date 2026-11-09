#include "xml/Parser.h"

#include "xml/ParseError.h"

#include <cctype>
#include <cstring>

namespace xml {

    namespace {

        /// @brief Tells whether a character may start an XML name.
        /// @param c The character.
        /// @return True for a letter, '_' or ':'.
        bool isNameStart(char c) { return std::isalpha(static_cast<unsigned char>(c)) != 0 || c == '_' || c == ':'; }

        /// @brief Tells whether a character may continue an XML name.
        /// @param c The character.
        /// @return True for a name start, a digit, '-' or '.'.
        bool isNameChar(char c) { return isNameStart(c) || std::isdigit(static_cast<unsigned char>(c)) != 0 || c == '-' || c == '.'; }

        /// @brief Appends a code point to a string as UTF-8.
        /// @param out String to append to.
        /// @param code Unicode code point.
        void appendUtf8(std::string& out, unsigned long code) {
            // UTF-8 encoding: up to 0x7F one byte, 0x7FF two bytes, 0xFFFF three bytes, else four; lead bytes 0xC0/0xE0/0xF0, continuation 0x80 with 6 payload bits (0x3F)
            if (code < 0x80) {
                out += static_cast<char>(code);
            } else if (code < 0x800) {
                out += static_cast<char>(0xC0 | (code >> 6));
                out += static_cast<char>(0x80 | (code & 0x3F));
            } else if (code < 0x10000) {
                out += static_cast<char>(0xE0 | (code >> 12));
                out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
                out += static_cast<char>(0x80 | (code & 0x3F));
            } else {
                out += static_cast<char>(0xF0 | (code >> 18));
                out += static_cast<char>(0x80 | ((code >> 12) & 0x3F));
                out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
                out += static_cast<char>(0x80 | (code & 0x3F));
            }
        }

    }

    Node Parser::parse(const std::string& document) {
        Parser parser(document);
        return parser.parseDocument();
    }

    void Parser::advance(std::size_t count) {
        for (std::size_t i = 0; i < count && m_pos < m_text.size(); ++i) {
            if (m_text[m_pos] == '\n') {
                ++m_line;
            }
            ++m_pos;
        }
    }

    bool Parser::lookingAt(const char* prefix) const { return m_text.compare(m_pos, std::strlen(prefix), prefix) == 0; }

    void Parser::fail(const std::string& message) const { throw ParseError(message, m_line); }

    void Parser::expect(char c) {
        if (m_pos >= m_text.size() || m_text[m_pos] != c) {
            fail(std::string("expected '") + c + "'");
        }
        advance();
    }

    void Parser::skipWhitespace() {
        while (m_pos < m_text.size() && std::isspace(static_cast<unsigned char>(m_text[m_pos])) != 0) {
            advance();
        }
    }

    void Parser::skipMisc() {
        for (;;) {
            skipWhitespace();
            if (lookingAt("<!--")) {
                const std::size_t end = m_text.find("-->", m_pos);
                if (end == std::string::npos) {
                    fail("unterminated comment");
                }
                advance(end + 3 - m_pos); // 3: length of "-->"
            } else if (lookingAt("<?")) {
                const std::size_t end = m_text.find("?>", m_pos);
                if (end == std::string::npos) {
                    fail("unterminated processing instruction");
                }
                advance(end + 2 - m_pos); // 2: length of "?>"
            } else if (lookingAt("<!DOCTYPE")) {
                const std::size_t end = m_text.find('>', m_pos);
                if (end == std::string::npos) {
                    fail("unterminated DOCTYPE");
                }
                advance(end + 1 - m_pos);
            } else {
                return;
            }
        }
    }

    Node Parser::parseDocument() {
        if (lookingAt("\xEF\xBB\xBF")) {
            advance(3); // skip the 3 byte UTF-8 byte order mark
        }
        skipMisc();
        if (!lookingAt("<")) {
            fail("expected the root element");
        }
        Node root = parseElement();
        skipMisc();
        if (m_pos != m_text.size()) {
            fail("content after the root element");
        }
        return root;
    }

    std::string Parser::parseName() {
        if (m_pos >= m_text.size() || !isNameStart(m_text[m_pos])) {
            fail("expected a name");
        }
        const std::size_t start = m_pos;
        while (m_pos < m_text.size() && isNameChar(m_text[m_pos])) {
            advance();
        }
        return m_text.substr(start, m_pos - start);
    }

    void Parser::parseAttributes(Node& node) {
        for (;;) {
            skipWhitespace();
            if (m_pos >= m_text.size()) {
                fail("unterminated start tag");
            }
            const char c = m_text[m_pos];
            if (c == '>' || c == '/') {
                return;
            }
            std::string name = parseName();
            skipWhitespace();
            expect('=');
            skipWhitespace();
            if (m_pos >= m_text.size() || (m_text[m_pos] != '"' && m_text[m_pos] != '\'')) {
                fail("expected a quoted value for attribute " + name);
            }
            const char quote = m_text[m_pos];
            advance();
            const std::size_t end = m_text.find(quote, m_pos);
            if (end == std::string::npos) {
                fail("unterminated value of attribute " + name);
            }
            const std::string raw = m_text.substr(m_pos, end - m_pos);
            advance(end + 1 - m_pos);
            if (node.hasAttribute(name)) {
                fail("duplicate attribute " + name);
            }
            node.addAttribute(name, decodeEntities(raw));
        }
    }

    Node Parser::parseElement() {
        expect('<');
        Node node(parseName());
        node.setLine(m_line);
        parseAttributes(node);
        if (lookingAt("/>")) {
            advance(2);
            return node;
        }
        expect('>');
        for (;;) {
            if (m_pos >= m_text.size()) {
                fail("unterminated element " + node.name());
            }
            if (lookingAt("</")) {
                advance(2);
                const std::string closing = parseName();
                if (closing != node.name()) {
                    fail("</" + closing + "> closes <" + node.name() + ">");
                }
                skipWhitespace();
                expect('>');
                return node;
            }
            if (lookingAt("<!--") || lookingAt("<?")) {
                skipMisc();
                continue;
            }
            if (lookingAt("<![CDATA[")) {
                advance(9); // 9: length of "<![CDATA["
                const std::size_t end = m_text.find("]]>", m_pos);
                if (end == std::string::npos) {
                    fail("unterminated CDATA section");
                }
                node.appendText(m_text.substr(m_pos, end - m_pos));
                advance(end + 3 - m_pos); // 3: length of "]]>"
                continue;
            }
            if (lookingAt("<")) {
                node.addChild(parseElement());
                continue;
            }
            const std::size_t end = m_text.find('<', m_pos);
            const std::size_t stop = end == std::string::npos ? m_text.size() : end;
            node.appendText(decodeEntities(m_text.substr(m_pos, stop - m_pos)));
            advance(stop - m_pos);
        }
    }

    std::string Parser::decodeEntities(const std::string& raw) {
        std::string out;
        for (std::size_t i = 0; i < raw.size(); ++i) {
            if (raw[i] != '&') {
                out += raw[i];
                continue;
            }
            const std::size_t end = raw.find(';', i);
            if (end == std::string::npos) {
                fail("unterminated entity");
            }
            const std::string entity = raw.substr(i + 1, end - i - 1);
            if (entity == "lt") {
                out += '<';
            } else if (entity == "gt") {
                out += '>';
            } else if (entity == "amp") {
                out += '&';
            } else if (entity == "quot") {
                out += '"';
            } else if (entity == "apos") {
                out += '\'';
            } else if (entity.size() > 1 && entity[0] == '#') {
                const bool hex = entity[1] == 'x' || entity[1] == 'X';
                // skip "#x" (2) or "#" (1)
                const std::string digits = entity.substr(hex ? 2 : 1);
                if (digits.empty()) {
                    fail("bad character reference");
                }
                unsigned long code = 0;
                for (char c : digits) {
                    // 10: hex letters a-f are the digit values 10..15; -1: not a digit
                    const int value = std::isdigit(static_cast<unsigned char>(c)) != 0           ? c - '0'
                                      : hex && std::isxdigit(static_cast<unsigned char>(c)) != 0 ? std::tolower(static_cast<unsigned char>(c)) - 'a' + 10
                                                                                                 : -1;
                    if (value < 0) {
                        fail("bad character reference");
                    }
                    // number base: 16 for &#x..; 10 for &#..;
                    code = code * (hex ? 16 : 10) + static_cast<unsigned long>(value);
                }
                appendUtf8(out, code);
            } else {
                fail("unknown entity &" + entity + ";");
            }
            i = end;
        }
        return out;
    }

}
