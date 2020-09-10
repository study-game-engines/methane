/******************************************************************************

Copyright 2019-2020 Evgeny Gorodetskiy

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

FILE: Methane/Graphics/DirectX12/FenceDX.cpp
DirectX 12 fence implementation.

******************************************************************************/

#include "FenceDX.h"
#include "CommandQueueDX.h"
#include "DeviceDX.h"

#include <Methane/Graphics/ContextBase.h>
#include <Methane/Instrumentation.h>
#include <Methane/Graphics/Windows/Primitives.h>

#include <nowide/convert.hpp>
#include <cassert>

namespace Methane::Graphics
{

Ptr<Fence> Fence::Create(CommandQueue& command_queue)
{
    META_FUNCTION_TASK();
    return std::make_shared<FenceDX>(static_cast<CommandQueueBase&>(command_queue));
}

FenceDX::FenceDX(CommandQueueBase& command_queue)
    : FenceBase(command_queue)
    , m_event(CreateEvent(nullptr, FALSE, FALSE, nullptr))
{
    META_FUNCTION_TASK();
    if (!m_event)
    {
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
    }

    const wrl::ComPtr<ID3D12Device>& cp_device = GetCommandQueueDX().GetContextDX().GetDeviceDX().GetNativeDevice();
    assert(!!cp_device);

    ThrowIfFailed(cp_device->CreateFence(GetValue(), D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_cp_fence)), cp_device.Get());
}

FenceDX::~FenceDX()
{
    META_FUNCTION_TASK();
    SafeCloseHandle(m_event);
}

void FenceDX::Signal()
{
    META_FUNCTION_TASK();
    FenceBase::Signal();

    assert(!!m_cp_fence);
    CommandQueueDX& command_queue = GetCommandQueueDX();
    ThrowIfFailed(command_queue.GetNativeCommandQueue().Signal(m_cp_fence.Get(), GetValue()),
                  command_queue.GetContextDX().GetDeviceDX().GetNativeDevice().Get());
}

void FenceDX::WaitOnCpu()
{
    META_FUNCTION_TASK();
    FenceBase::WaitOnCpu();

    assert(!!m_cp_fence);
    assert(!!m_event);
    if (m_cp_fence->GetCompletedValue() < GetValue())
    {
        META_LOG("SLEEP on fence \"" + GetName() + "\" with value " + std::to_string(GetValue()));

        ThrowIfFailed(m_cp_fence->SetEventOnCompletion(GetValue(), m_event),
                      GetCommandQueueDX().GetContextDX().GetDeviceDX().GetNativeDevice().Get());
        WaitForSingleObjectEx(m_event, INFINITE, FALSE);

        META_LOG("AWAKE on fence \"" + GetName() + "\" with value " + std::to_string(GetValue()));
    }
}

void FenceDX::WaitOnGpu(CommandQueue& wait_on_command_queue)
{
    META_FUNCTION_TASK();
    FenceBase::WaitOnGpu(wait_on_command_queue);

    assert(!!m_cp_fence);
    CommandQueueDX& dx_wait_on_command_queue = static_cast<CommandQueueDX&>(wait_on_command_queue);
    ID3D12CommandQueue& native_wait_on_command_queue = dx_wait_on_command_queue.GetNativeCommandQueue();
    ThrowIfFailed(native_wait_on_command_queue.Wait(m_cp_fence.Get(), GetValue()),
                  dx_wait_on_command_queue.GetContextDX().GetDeviceDX().GetNativeDevice().Get());
}

void FenceDX::SetName(const std::string& name) noexcept
{
    META_FUNCTION_TASK();
    if (ObjectBase::GetName() == name)
        return;

   ObjectBase::SetName(name);

    assert(!!m_cp_fence);
    m_cp_fence->SetName(nowide::widen(name).c_str());
}

CommandQueueDX& FenceDX::GetCommandQueueDX()
{
    META_FUNCTION_TASK();
    return static_cast<CommandQueueDX&>(GetCommandQueue());
}

} // namespace Methane::Graphics
