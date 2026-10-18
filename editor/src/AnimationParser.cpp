/*********************************************************************
   (c) Alex Raag 2026
   https://github.com/Enziferum
   robot2D - Zlib license.
   This software is provided 'as-is', without any express or
   implied warranty. In no event will the authors be held
   liable for any damages arising from the use of this software.
   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute
   it freely, subject to the following restrictions:
   1. The origin of this software must not be misrepresented;
   you must not claim that you wrote the original software.
   If you use this software in a product, an acknowledgment
   in the product documentation would be appreciated but
   is not required.
   2. Altered source versions must be plainly marked as such,
   and must not be misrepresented as being the original software.
   3. This notice may not be removed or altered from any
   source distribution.
*********************************************************************/

#include <memory>
#include <limits>

#include <TinyXML/tinyxml.h>
#include <robot2D/Graphics/Color.hpp>
#include <robot2D/Util/Logger.hpp>
#include <editor/AnimationParser.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
namespace editor {

    unsigned long colorToHex(int r, int g, int b, int a) {
        return ((r & 0xff) << 24) + ((g & 0xff) << 16) + ((b & 0xff) << 8)
               + (a & 0xff);
    }

    robot2D::Color hexToColor(unsigned int hexValue) {
        robot2D::Color rgbColor;
        rgbColor.red = static_cast<float>((hexValue >> 24) & 0xFF);
        rgbColor.green = static_cast<float>((hexValue >> 16) & 0xFF);
        rgbColor.blue = static_cast<float>((hexValue >> 16) & 0xFF);
        rgbColor.alpha = static_cast<float>((hexValue) & 0xFF);
        return rgbColor;
    }

    AnimationParser::AnimationParser() {
        m_keys = {
            { XmlKey::image, "image"},
            { XmlKey::maskColor, "maskColor"},
            { XmlKey::title, "title"},
            { XmlKey::delay, "delay"},
            { XmlKey::head, "sprites"},
            { XmlKey::cut, "cut"},
            { XmlKey::animation, "animation"},
            { XmlKey::x_coord, "x"},
            { XmlKey::y_coord, "y"},
            { XmlKey::width, "w"},
            { XmlKey::height, "h"},
        };
    }

    bool AnimationParser::loadFromFile(const std::string& path, Animation* anim) {
        const auto fail = [&path](const char* reason) {
            RB_EDITOR_WARN("AnimationParser::loadFromFile: path={0}, reason={1}", path, reason);
            return false;
        };
        if(!anim)
            return fail("null destination");
        std::ifstream input(std::filesystem::u8path(path), std::ios::binary);
        std::ostringstream contents;
        contents << input.rdbuf();
        TiXmlDocument document;
        document.Parse(contents.str().c_str());
        if(!input || document.Error())
            return fail("invalid or unreadable XML");
        const auto* head = document.FirstChildElement("sprites");
        if(!head || !head -> Attribute("image") || !*head -> Attribute("image"))
            return fail("missing image");
        const auto* element = head -> FirstChildElement("animation");
        if(!element || element -> NextSiblingElement("animation"))
            return fail("expected one clip per animation file");
        if(!element -> Attribute("title") || !*element -> Attribute("title"))
            return fail("missing clip name");
        Animation loaded;
        loaded.texturePath = head -> Attribute("image");
        loaded.name = element -> Attribute("title");
        loaded.filePath = path;
        if(element -> QueryIntAttribute("delay", &loaded.framesPerSecond) != TIXML_SUCCESS ||
            loaded.framesPerSecond <= 0)
            return fail("FPS must be a positive integer");
        if(element -> Attribute("loop") && element -> QueryBoolAttribute("loop", &loaded.isLooped) != TIXML_SUCCESS)
            return fail("invalid loop flag");
        if(element -> Attribute("flipped") && element -> QueryBoolAttribute("flipped",
            &loaded.isFlipped) != TIXML_SUCCESS)
            return fail("invalid flipped flag");
        for(const auto* cut = element -> FirstChildElement("cut"); cut; cut = cut -> NextSiblingElement("cut")) {
            robot2D::IntRect frame;
            if(cut -> QueryIntAttribute("x", &frame.lx) != TIXML_SUCCESS ||
                cut -> QueryIntAttribute("y", &frame.ly) != TIXML_SUCCESS ||
                cut -> QueryIntAttribute("w", &frame.width) != TIXML_SUCCESS ||
                cut -> QueryIntAttribute("h", &frame.height) != TIXML_SUCCESS ||
                frame.lx < 0 || frame.ly < 0 || frame.width <= 0 || frame.height <= 0 ||
                frame.lx > std::numeric_limits<int>::max() - frame.width ||
                frame.ly > std::numeric_limits<int>::max() - frame.height)
                return fail("invalid frame rectangle");
            loaded.addFrame(frame);
        }
        if(loaded.frames.empty())
            return fail("clip has no frames");
        *anim = std::move(loaded);
        return true;
    }

    bool AnimationParser::saveToFile(const std::string& path, const Animation* anim) {
        if(!anim || anim -> name.empty() || anim -> texturePath.empty() ||
            anim -> framesPerSecond <= 0 || anim -> frames.empty()) {
            RB_EDITOR_WARN("AnimationParser::saveToFile: path={0}, reason=invalid clip metadata", path);
            return false;
        }
        for(const auto& frame: anim -> frames) {
            const auto& rect = frame.frame;
            if(rect.lx < 0 || rect.ly < 0 || rect.width <= 0 || rect.height <= 0 ||
                rect.lx > std::numeric_limits<int>::max() - rect.width ||
                rect.ly > std::numeric_limits<int>::max() - rect.height)
                return false;
        }
        TiXmlDocument document{path.c_str()};

        auto declaration = std::make_unique<TiXmlDeclaration>("1.0", "", "robot2D ");
        if(!declaration)
            return false;
        document.InsertEndChild(*declaration.get());

        auto head = std::make_unique<TiXmlElement>(m_keys[XmlKey::head].c_str());
        if(!head)
            return false;
        head -> SetAttribute(m_keys[XmlKey::image].c_str(), anim -> texturePath.c_str());

        //        auto hexColor = colorToHex(maskColor.red, maskColor.green, maskColor.blue, maskColor.alpha);
        //        std::string hexColorStr = std::to_string(hexColor);
        //        head -> SetAttribute(m_keys[XmlKey::maskColor].c_str(), hexColorStr.c_str());

        auto animation = std::make_unique<TiXmlElement>(m_keys[XmlKey::animation].c_str());
        if(!animation)
            return false;
        animation -> SetAttribute(m_keys[XmlKey::title].c_str(), anim -> name.c_str());
        animation -> SetAttribute(m_keys[XmlKey::delay].c_str(), anim -> framesPerSecond);
        animation -> SetAttribute("loop", anim -> isLooped ? "true" : "false");
        animation -> SetAttribute("flipped", anim -> isFlipped ? "true" : "false");
        for(auto& frame: anim -> frames) {
            auto cutElement = std::make_unique<TiXmlElement>(m_keys[XmlKey::cut].c_str());
            if(!cutElement)
                return false;
            cutElement -> SetAttribute(m_keys[XmlKey::x_coord].c_str(), frame.frame.lx);
            cutElement -> SetAttribute(m_keys[XmlKey::y_coord].c_str(), (frame.frame.ly));
            cutElement -> SetAttribute(m_keys[XmlKey::width].c_str(), frame.frame.width);
            cutElement -> SetAttribute(m_keys[XmlKey::height].c_str(), frame.frame.height);
            animation -> InsertEndChild(*cutElement.get());
        }
        head -> InsertEndChild(*animation.get());
        document.InsertEndChild(*head.get());

        TiXmlPrinter printer;
        if(!document.Accept(&printer)) return false;
        std::ofstream output(std::filesystem::u8path(path), std::ios::binary | std::ios::trunc);
        output << printer.CStr();
        output.close();
        return static_cast<bool>(output);
    }


} // namespace editor