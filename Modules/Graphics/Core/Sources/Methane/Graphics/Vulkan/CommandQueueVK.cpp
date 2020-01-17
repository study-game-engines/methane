/******************************************************************************

Copyright 2019 Evgeny Gorodetskiy

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*******************************************************************************

FILE: Methane/Graphics/Vulkan/CommandQueueVK.mm
Vulkan implementation of the command queue interface.

******************************************************************************/

#include "CommandQueueVK.h"
#include "ContextVK.h"

#include <Methane/Data/Instrumentation.h>

namespace Methane::Graphics
{

CommandQueue::Ptr CommandQueue::Create(Context& context)
{
    ITT_FUNCTION_TASK();
    return std::make_shared<CommandQueueVK>(static_cast<ContextBase&>(context));
}

CommandQueueVK::CommandQueueVK(ContextBase& context)
    : CommandQueueBase(context, true)
{
    ITT_FUNCTION_TASK();
}

CommandQueueVK::~CommandQueueVK()
{
    ITT_FUNCTION_TASK();
    assert(!IsExecuting());
}

void CommandQueueVK::SetName(const std::string& name)
{
    ITT_FUNCTION_TASK();

    CommandQueueBase::SetName(name);
}

ContextVK& CommandQueueVK::GetContextVK() noexcept
{
    ITT_FUNCTION_TASK();
    return static_cast<class ContextVK&>(m_context);
}

} // namespace Methane::Graphics