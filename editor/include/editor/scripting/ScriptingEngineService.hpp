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

#pragma once
#include <string>
#include <unordered_map>
#include <memory>

#include "IScriptingEngineService.hpp"
#include "MonoClassWrapper.hpp"
#include "ScriptInstance.hpp"

namespace editor {

    class ScriptingEngineService: virtual public IScriptingEngineService {
    public:
        using Ptr = ScriptingEngineService*;
    public:
        virtual ~ScriptingEngineService() = 0;
        virtual void runtimeInit(const std::string& runtimeDllPath) = 0;

        virtual ScriptInstance::Ptr getEntityScriptInstance(UUID entityID);
        virtual MonoClassWrapper::Ptr getEntityClass(const std::string& name);

        virtual bool hasEntityClass(const std::string& name);
        virtual const std::unordered_map<std::string, MonoClassWrapper::Ptr>& getClasses();
    };

}