/*
 Copyright (c) 2022 Alex Raag. All rights reserved.
 Use of this source code is governed by a BSD-style license that can be
 found in the LICENSE file.
*/

#include <editor/EventBinder.hpp>

namespace robot2D {

    template<>
    const Event::MouseButtonEvent& eventAs(const robot2D::Event& event) {
        return event.mouse;
    }

    template<>
    const Event::MouseWheelEvent& eventAs(const robot2D::Event& event) {
        return event.wheel;
    }

    template<>
    const Event::MouseMoveEvent& eventAs(const robot2D::Event& event) {
        return event.move;
    }

    template<>
    const Event::KeyboardEvent& eventAs(const robot2D::Event& event) {
        return event.key;
    }

    template<>
    const Event::SizeEvent& eventAs(const robot2D::Event& event) {
        return event.size;
    }
}

namespace editor  {

    EventBinder::IFunc::~IFunc() {}
} // namespace editor