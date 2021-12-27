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

FILE: Methane/Graphics/SamplerBase.cpp
Base implementation of the sampler interface.

******************************************************************************/

#include "SamplerBase.h"

#include <Methane/Instrumentation.h>

namespace Methane::Graphics
{

Sampler::Settings::Settings(const Filter& filter, const Address& address,
                            const LevelOfDetail& lod, uint32_t max_anisotropy,
                            BorderColor border_color, Compare compare_function)
    : filter(filter)
    , address(address)
    , lod(lod)
    , max_anisotropy(max_anisotropy)
    , border_color(border_color)
    , compare_function(compare_function)
{
}

Sampler::LevelOfDetail::LevelOfDetail(float bias, float min, float max)
    : min(min)
    , max(max)
    , bias(bias)
{
}

SamplerBase::SamplerBase(const ContextBase& context, const Settings& settings)
    : ResourceBase(Type::Sampler, Usage::ShaderRead, context)
    , m_settings(settings)
{
    META_FUNCTION_TASK();
}

void SamplerBase::SetData(const SubResources&, CommandQueue*)
{
    META_FUNCTION_NOT_IMPLEMENTED_DESCR("Samplers do not support setting the data.");
}

} // namespace Methane::Graphics