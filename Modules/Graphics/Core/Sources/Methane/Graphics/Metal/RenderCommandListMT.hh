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

FILE: Methane/Graphics/Metal/RenderCommandListMT.hh
Metal implementation of the render command list interface.

******************************************************************************/

#pragma once

#include <Methane/Graphics/RenderCommandListBase.h>

#include <functional>

#import <Metal/Metal.h>

namespace Methane::Graphics
{

class CommandQueueMT;
class BufferMT;
class RenderPassMT;

class RenderCommandListMT final : public RenderCommandListBase
{
public:
    RenderCommandListMT(CommandQueueBase& command_queue, RenderPassBase& render_pass);
    RenderCommandListMT(ParallelRenderCommandListBase& parallel_render_command_list);

    // CommandList interface
    void PushDebugGroup(const std::string& name) override;
    void PopDebugGroup() override;
    void Commit(bool present_drawable) override;

    // CommandListBase interface
    void SetResourceBarriers(const ResourceBase::Barriers&) override { }
    void Execute(uint32_t frame_index) override;

    // RenderCommandList interface
    void Reset(const RenderState::Ptr& sp_render_state, const std::string& debug_group = "") override;
    void SetVertexBuffers(const Buffer::Refs& vertex_buffers) override;
    void DrawIndexed(Primitive primitive, Buffer& index_buffer,
                     uint32_t index_count, uint32_t start_index, uint32_t start_vertex,
                     uint32_t instance_count, uint32_t start_instance) override;
    void Draw(Primitive primitive, uint32_t vertex_count, uint32_t start_vertex,
              uint32_t instance_count, uint32_t start_instance) override;

    // Object interface
    void SetName(const std::string& label) override;

    bool IsRenderEncoding() const { return m_mtl_render_encoder != nil; }
    void StartRenderEncoding();
    void EndRenderEncoding();
    
    bool IsBlitEncoding() const   { return m_mtl_blit_encoder != nil; }
    void StartBlitEncoding();
    void EndBlitEncoding();

    id<MTLCommandBuffer>&        GetNativeCommandBuffer() noexcept { return m_mtl_cmd_buffer; }
    id<MTLRenderCommandEncoder>& GetNativeRenderEncoder() noexcept { return m_mtl_render_encoder; }
    id<MTLBlitCommandEncoder>&   GetNativeBlitEncoder() noexcept   { return m_mtl_blit_encoder; }

protected:
    void InitializeCommandBuffer();
    
    CommandQueueMT& GetCommandQueueMT() noexcept;
    RenderPassMT&   GetPassMT();

    id<MTLCommandBuffer>        m_mtl_cmd_buffer = nil;
    id<MTLRenderCommandEncoder> m_mtl_render_encoder = nil;
    id<MTLBlitCommandEncoder>   m_mtl_blit_encoder = nil;
};

} // namespace Methane::Graphics