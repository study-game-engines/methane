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

FILE: Methane/Graphics/ParallelRenderCommandListBase.cpp
Base implementation of the parallel render command list interface.

******************************************************************************/

#include "ParallelRenderCommandListBase.h"
#include "RenderCommandListBase.h"
#include "RenderPassBase.h"
#include "RenderStateBase.h"
#include "BufferBase.h"
#include "ProgramBase.h"

#include <Methane/Data/Instrumentation.h>
#include <Methane/Data/Parallel.hpp>

#include <cassert>

namespace Methane::Graphics
{

inline std::string GetThreadCommandListName(const std::string& name, uint32_t index)
{
    return name + " [Thread " + std::to_string(index) + "]";
}

ParallelRenderCommandListBase::ParallelRenderCommandListBase(CommandQueueBase& command_queue, RenderPassBase& pass)
    : CommandListBase(command_queue, Type::ParallelRenderCommandList)
    , m_sp_pass(pass.GetPtr())
{
    ITT_FUNCTION_TASK();
}

void ParallelRenderCommandListBase::Reset(const RenderState::Ptr& sp_render_state, const std::string& debug_group)
{
    ITT_FUNCTION_TASK();

    uint32_t render_command_list_index = 0;
    for(const RenderCommandList::Ptr& sp_render_command_list : m_parallel_comand_lists)
    {
        assert(sp_render_command_list);
        const std::string render_debug_group = GetThreadCommandListName(debug_group, render_command_list_index++);
        sp_render_command_list->Reset(sp_render_state, render_debug_group);
    }
}

void ParallelRenderCommandListBase::Commit(bool present_drawable)
{
    ITT_FUNCTION_TASK();

    for(const RenderCommandList::Ptr& sp_render_command_list : m_parallel_comand_lists)
    {
        assert(!!sp_render_command_list);
        sp_render_command_list->Commit(false);
    }

    CommandListBase::Commit(present_drawable);
}

void ParallelRenderCommandListBase::SetParallelCommandListsCount(uint32_t count)
{
    ITT_FUNCTION_TASK();

    uint32_t initial_count = static_cast<uint32_t>(m_parallel_comand_lists.size());
    if (count < initial_count)
    {
        m_parallel_comand_lists.resize(count);
        return;
    }

    const std::string& name = GetName();
    m_parallel_comand_lists.reserve(count);
    for(uint32_t cmd_list_index = initial_count; cmd_list_index < count; ++cmd_list_index)
    {
        m_parallel_comand_lists.emplace_back(RenderCommandList::Create(*this));
        if (!name.empty())
        {
            m_parallel_comand_lists.back()->SetName(GetThreadCommandListName(name, cmd_list_index));
        }
    }
}

void ParallelRenderCommandListBase::Execute(uint32_t frame_index)
{
    ITT_FUNCTION_TASK();

    for(const RenderCommandList::Ptr& sp_render_command_list : m_parallel_comand_lists)
    {
        assert(!!sp_render_command_list);
        RenderCommandListBase& metal_render_command_list = static_cast<RenderCommandListBase&>(*sp_render_command_list);
        metal_render_command_list.Execute(frame_index);
    }

    CommandListBase::Execute(frame_index);
}

void ParallelRenderCommandListBase::Complete(uint32_t frame_index)
{
    ITT_FUNCTION_TASK();

    for(const RenderCommandList::Ptr& sp_render_command_list : m_parallel_comand_lists)
    {
        assert(!!sp_render_command_list);
        RenderCommandListBase& metal_render_command_list = static_cast<RenderCommandListBase&>(*sp_render_command_list);
        metal_render_command_list.Complete(frame_index);
    }

    CommandListBase::Complete(frame_index);
}

void ParallelRenderCommandListBase::SetName(const std::string& name)
{
    ITT_FUNCTION_TASK();

    CommandListBase::SetName(name);

    if (name.empty())
        return;

    uint32_t render_cmd_list_index = 0;
    for(const RenderCommandList::Ptr& sp_render_cmd_list : m_parallel_comand_lists)
    {
        assert(!!sp_render_cmd_list);
        sp_render_cmd_list->SetName(GetThreadCommandListName(name, render_cmd_list_index));
        render_cmd_list_index++;
    }
}

RenderPassBase& ParallelRenderCommandListBase::GetPass()
{
    ITT_FUNCTION_TASK();
    assert(!!m_sp_pass);
    return static_cast<RenderPassBase&>(*m_sp_pass);
}

} // namespace Methane::Graphics
