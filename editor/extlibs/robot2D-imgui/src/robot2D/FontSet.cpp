#include "FontSet.hpp"
#include <cmath>
#include <fstream>
#include <iterator>
#include <limits>
#include <unordered_set>
#include <robot2D/Util/Logger.hpp>

namespace robot2D::priv {
    namespace {
        bool validFont(const std::vector<unsigned char>& bytes) {
            if(bytes.size() < 12) return false;
            const auto u16 = [&](std::size_t at) {
                return (static_cast<unsigned>(bytes[at]) << 8) | bytes[at + 1];
            };
            const auto u32 = [&](std::size_t at) {
                return (static_cast<std::uint32_t>(u16(at)) << 16) | u16(at + 2);
            };
            const auto signature = u32(0);
            if(signature != 0x00010000 && signature != 0x4F54544F && signature != 0x74727565) return false;
            const auto count = u16(4);
            if(count == 0 || 12 + 16 * count > bytes.size()) return false;
            for(unsigned i = 0; i < count; ++i) {
                const auto offset = u32(12 + i * 16 + 8);
                const auto length = u32(12 + i * 16 + 12);
                if(offset > bytes.size() || length > bytes.size() - offset) return false;
            }
            return true;
        }
    }

    bool FontSet::build(const GuiConfiguration& configuration, std::string& error) {
        error.clear();
        auto& io = ImGui::GetIO();
        if(!io.Fonts -> Fonts.empty()) {
            error = "font set requires a fresh atlas"; return false;
        }
        clear();
        m_sources = configuration.fonts;
        std::unordered_set<FontId> configuredIds;
        for(auto& source : m_sources) {
            const auto fail = [&](const std::string& reason) {
                error = "font id=" + std::to_string(source.id) + "; path='" + source.path + "'; " + reason;
                return false;
            };
            if(source.id == 0 || !configuredIds.insert(source.id).second) return fail("invalid or duplicate FontId");
            if(!std::isfinite(source.size) || source.size <= 0) return fail("size must be finite and positive");
            if(!source.path.empty() && !source.bytes.empty()) return fail("choose file or memory source");
            if(source.mergeInto != 0 && !get(source.mergeInto)) return fail("merge target must precede source");
            if(!source.glyphRanges.empty()) {
                const auto& ranges = source.glyphRanges;
                if(ranges.size() < 3 || ranges.size() % 2 != 1 ||
                    ranges.back() != 0) return fail("invalid glyph ranges");
                for(std::size_t i = 0; i + 1 < ranges.size(); i += 2)
                    if(ranges[i] == 0 || ranges[i] > ranges[i + 1]) return fail("invalid glyph range");
            }
            if(!source.path.empty()) {
                std::ifstream input(source.path, std::ios::binary);
                if(input) source.bytes.assign(std::istreambuf_iterator<char>(input), {});
            }
            if(source.bytes.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()) ||
                !validFont(source.bytes)) {
                if(source.required &&
                    !configuration.allowDefaultFallback) return fail("missing or invalid font source");
                RB_EDITOR_WARN("[GUI] Font source unavailable; id={}; path='{}'; fallback={}",
                    source.id, source.path, configuration.allowDefaultFallback);
                if(source.mergeInto) {
                    m_fonts[source.id] = get(source.mergeInto); continue;
                }
                if(configuration.allowDefaultFallback) {
                    ImFontConfig fallback;
                    fallback.SizePixels = source.size;
                    m_fonts[source.id] = io.Fonts -> AddFontDefault(&fallback);
                }
                continue;
            }
            ImFontConfig config;
            config.FontDataOwnedByAtlas = false;
            config.MergeMode = source.mergeInto != 0;
            config.DstFont = get(source.mergeInto);
            config.PixelSnapH = config.MergeMode;
            if(config.MergeMode) config.GlyphMinAdvanceX = source.size;
            auto* font = io.Fonts -> AddFontFromMemoryTTF(source.bytes.data(), static_cast<int>(source.bytes.size()),
                    source.size, &config, source.glyphRanges.empty() ? nullptr : source.glyphRanges.data());
            if(!font) return fail("ImGui rejected font");
            m_fonts[source.id] = font;
        }
        if(configuration.defaultFont) {
            io.FontDefault = get(configuration.defaultFont);
            if(!io.FontDefault) {
                error = "default FontId=" + std::to_string(configuration.defaultFont) + " is unavailable"; return false;
            }
        }
        else if(m_sources.empty() && configuration.allowDefaultFallback) io.FontDefault = io.Fonts -> AddFontDefault();
        else { error = "default FontId must be selected explicitly"; return false; }
        if(!io.Fonts -> Build()) {
            error = "font atlas build failed"; return false;
        }
        return true;
    }

    ImFont* FontSet::get(FontId id) const noexcept {
        const auto found = m_fonts.find(id);
        return found == m_fonts.end() ? nullptr : found -> second;
    }

    void FontSet::clear() {
        m_fonts.clear(); m_sources.clear();
    }
}
