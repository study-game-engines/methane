/******************************************************************************

Copyright 2019-2020 Evgeny Gorodetskiy

Licensed under the Apache License, Version 2.0 (the "License"),
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*******************************************************************************

FILE: Methane/Graphics/DirectX12/CommandList.mm
Metal command lists sequence implementation

******************************************************************************/

#include <Methane/Graphics/Metal/CommandList.hh>

#include <Methane/Instrumentation.h>
#include <Methane/Platform/Apple/Types.hh>

namespace Methane::Graphics::Rhi
{

Ptr<ICommandListDebugGroup> ICommandListDebugGroup::Create(const std::string& name)
{
    META_FUNCTION_TASK();
    return std::make_shared<Metal::CommandListDebugGroup>(name);
}

Ptr<ICommandListSet> ICommandListSet::Create(const Refs<ICommandList>& command_list_refs, Opt<Data::Index> frame_index_opt)
{
    META_FUNCTION_TASK();
    return std::make_shared<Metal::CommandListSet>(command_list_refs, frame_index_opt);
}

} // namespace Methane::Graphics::Rhi

namespace Methane::Graphics::Metal
{

CommandListDebugGroup::CommandListDebugGroup(const std::string& name)
    : Base::CommandListDebugGroup(std::move(name))
    , m_ns_name(MacOS::ConvertToNsType<std::string, NSString*>(Base::Object::GetName()))
{
    META_FUNCTION_TASK();
}

CommandListSet::CommandListSet(const Refs<Rhi::ICommandList>& command_list_refs, Opt<Data::Index> frame_index_opt)
    : Base::CommandListSet(command_list_refs, frame_index_opt)
{
    META_FUNCTION_TASK();
}

} // namespace Methane::Graphics::Metal
