/*********************************************************************
(c) Alex Raag 2024
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

#include <TinyXML/tinyxml.h>
#include <robot2D/Graphics/Color.hpp>
#include <robot2D/Util/Logger.hpp>
#include <editor/AnimationParser.hpp>

namespace editor {

    unsigned long colorToHex(int r, int g, int b, int a)
    {
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
        const char* p = path.c_str();
        TiXmlDocument document(p);
        if (!document.LoadFile())
            return false;

        TiXmlElement* head = document.FirstChildElement(m_keys[XmlKey::head].c_str());
        if(!head)
            return false;

        anim -> texturePath = head -> Attribute(m_keys[XmlKey::image].c_str());
        // std::string maskColor = head -> Attribute(m_keys[XmlKey::maskColor].c_str());

        TiXmlElement* animation = head -> FirstChildElement(m_keys[XmlKey::animation].c_str());
        if(!animation)
            return false;

        while(animation) {
            anim -> name = animation -> Attribute(m_keys[XmlKey::title].c_str());
            animation -> Attribute(m_keys[XmlKey::delay].c_str(), &anim -> framesPerSecond);
            RB_EDITOR_INFO("Start process animation, title: {0}, delay: {1}", anim -> name, anim -> framesPerSecond);

            TiXmlElement* cut = animation -> FirstChildElement(m_keys[XmlKey::cut].c_str());
            if(!cut)
                return false;

            while (cut) {
                robot2D::IntRect frame;
                cut -> Attribute(m_keys[XmlKey::x_coord].c_str(), &frame.lx);
                cut -> Attribute(m_keys[XmlKey::y_coord].c_str(), &frame.ly);
                cut -> Attribute(m_keys[XmlKey::width].c_str(), &frame.width);
                cut -> Attribute(m_keys[XmlKey::height].c_str(), &frame.height);
                anim -> addFrame(frame);
                cut = cut->NextSiblingElement();
            }
            animation = animation -> NextSiblingElement();
            RB_EDITOR_INFO("Animation reading finish, got : {0} frames", anim -> frames.size());
        }
        return true;
    }

    bool AnimationParser::saveToFile(const std::string& path, const Animation* anim) {
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

        if(!document.SaveFile())
            return false;

        return true;
    }


} // namespace editor