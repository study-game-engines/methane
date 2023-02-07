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

FILE: Methane/Graphics/Metal/Resource.hh
Metal implementation of the resource interface.

******************************************************************************/
#pragma once

#include "Device.hh"

#include <Methane/Graphics/Base/Resource.h>
#include <Methane/Graphics/Base/RenderContext.h>
#include <Methane/Instrumentation.h>

#include <vector>

namespace Methane::Graphics::Metal
{

struct IContext;

template<typename ReourceBaseType, typename = std::enable_if_t<std::is_base_of_v<Base::Resource, ReourceBaseType>, void>>
class Resource : public ReourceBaseType
{
public:
    template<typename SettingsType>
    Resource(const Base::Context& context, const SettingsType& settings)
        : ReourceBaseType(context, settings)
    { }

    ~Resource() override
    {
        META_FUNCTION_TASK();
        // Resource released callback has to be emitted before native resource is released
        Data::Emitter<Rhi::IResourceCallback>::Emit(&Rhi::IResourceCallback::OnResourceReleased, std::ref(*this));
    }

    const Rhi::IResource::DescriptorByViewId& GetDescriptorByViewId() const noexcept final
    {
        static const Rhi::IResource::DescriptorByViewId s_dummy_descriptor_by_view_id;
        return s_dummy_descriptor_by_view_id;
    }

    void RestoreDescriptorViews(const Rhi::IResource::DescriptorByViewId&) final { /* intentionally uninitialized */ }

protected:
    const IContext& GetMetalContext() const noexcept
    {
        META_FUNCTION_TASK();
        return dynamic_cast<const IContext&>(Base::Resource::GetBaseContext());
    }

    id<MTLBuffer> GetUploadSubresourceBuffer(const Rhi::IResource::SubResource& sub_resource)
    {
        META_FUNCTION_TASK();
        const Data::Index sub_resource_raw_index = sub_resource.GetIndex().GetRawIndex(Base::Resource::GetSubresourceCount());
        m_upload_subresource_buffers.resize(sub_resource_raw_index + 1);

        id<MTLBuffer> mtl_upload_subresource_buffer = m_upload_subresource_buffers[sub_resource_raw_index];
        if (!mtl_upload_subresource_buffer || mtl_upload_subresource_buffer.length != sub_resource.GetDataSize())
        {
            const id<MTLDevice>& mtl_device = GetMetalContext().GetMetalDevice().GetNativeDevice();
            mtl_upload_subresource_buffer = [mtl_device newBufferWithBytes:sub_resource.GetDataPtr()
                                                                    length:sub_resource.GetDataSize()
                                                                   options:MTLResourceStorageModeShared];
            m_upload_subresource_buffers[sub_resource_raw_index] = mtl_upload_subresource_buffer;
        }
        else
        {
            Data::RawPtr p_resource_data = static_cast<Data::RawPtr>([mtl_upload_subresource_buffer contents]);
            META_CHECK_ARG_NOT_NULL(p_resource_data);
            std::copy(sub_resource.GetDataPtr(), sub_resource.GetDataEndPtr(), p_resource_data);
        }
        return mtl_upload_subresource_buffer;
    }

private:
    std::vector<id<MTLBuffer>> m_upload_subresource_buffers;
};

} // namespace Methane::Graphics::Metal