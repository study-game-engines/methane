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

FILE: Methane/Graphics/Metal/ParallelRenderCommandList.mm
Metal implementation of the parallel render command list interface.

******************************************************************************/

#include <Methane/Graphics/Metal/ParallelRenderCommandList.hh>
#include <Methane/Graphics/Metal/RenderPass.hh>
#include <Methane/Graphics/Metal/RenderState.hh>
#include <Methane/Graphics/Metal/RenderContext.hh>

#include <Methane/Instrumentation.h>

namespace Methane::Graphics::Rhi
{

Ptr<IParallelRenderCommandList> IParallelRenderCommandList::Create(ICommandQueue& command_queue, IRenderPass& render_pass)
{
    META_FUNCTION_TASK();
    return std::make_shared<Metal::ParallelRenderCommandList>(static_cast<Base::CommandQueue&>(command_queue), static_cast<Base::RenderPass&>(render_pass));
}

} // namespace Methane::Graphics::Rhi

namespace Methane::Graphics::Metal
{

ParallelRenderCommandList::ParallelRenderCommandList(Base::CommandQueue& command_queue, Base::RenderPass& render_pass)
    : CommandList<id<MTLParallelRenderCommandEncoder>, Base::ParallelRenderCommandList>(true, command_queue, render_pass)
{
    META_FUNCTION_TASK();
}

void ParallelRenderCommandList::Reset(IDebugGroup* p_debug_group)
{
    META_FUNCTION_TASK();
    ResetCommandEncoder();
    Base::ParallelRenderCommandList::Reset(p_debug_group);
}

void ParallelRenderCommandList::ResetWithState(Rhi::IRenderState& render_state, IDebugGroup* p_debug_group)
{
    META_FUNCTION_TASK();
    if (ResetCommandEncoder())
    {
        static_cast<RenderState&>(render_state).InitializeNativeStates();
    }
    Base::ParallelRenderCommandList::ResetWithState(render_state, p_debug_group);
}

bool ParallelRenderCommandList::ResetCommandEncoder()
{
    META_FUNCTION_TASK();
    if (IsCommandEncoderInitialized())
        return false;

    // NOTE: If command buffer was not created for current frame yet,
    // then render pass descriptor should be reset with new frame drawable
    MTLRenderPassDescriptor* mtl_render_pass = GetMetalRenderPass().GetNativeDescriptor(!IsCommandBufferInitialized());
    META_CHECK_ARG_NOT_NULL(mtl_render_pass);

    const id<MTLCommandBuffer>& mtl_cmd_buffer = InitializeCommandBuffer();
    InitializeCommandEncoder([mtl_cmd_buffer parallelRenderCommandEncoderWithDescriptor: mtl_render_pass]);
    return true;
}

RenderPass& ParallelRenderCommandList::GetMetalRenderPass()
{
    META_FUNCTION_TASK();
    return static_cast<class RenderPass&>(GetPass());
}

} // namespace Methane::Graphics::Metal
