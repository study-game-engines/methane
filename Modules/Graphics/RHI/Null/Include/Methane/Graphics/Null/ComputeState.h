/******************************************************************************

Copyright 2023 Evgeny Gorodetskiy

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

FILE: Methane/Graphics/Null/ComputeState.h
Null implementation of the compute state interface.

******************************************************************************/

#pragma once

#include <Methane/Graphics/Base/ComputeState.h>

namespace Methane::Graphics::Null
{

class ComputeState final
    : public Base::ComputeState
{
public:
    using Base::ComputeState::ComputeState;

    // Base::ComputeState interface
    void Apply(Base::ComputeCommandList&) override { /* Intentionally unimplemented */ }
};

} // namespace Methane::Graphics::Null
