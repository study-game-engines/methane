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

FILE: Tests/Graphics/RHI/ProgramTest.cpp
Unit-tests of the RHI Program

******************************************************************************/

#include "Methane/Graphics/RHI/IContext.h"
#include "Methane/Graphics/RHI/IResource.h"
#include "Methane/Graphics/Types.h"
#include "Methane/Graphics/Volume.hpp"
#include "RhiTestHelpers.hpp"

#include <Methane/Data/AppShadersProvider.h>
#include <Methane/Graphics/RHI/ComputeContext.h>
#include <Methane/Graphics/RHI/Program.h>
#include <Methane/Graphics/RHI/ProgramBindings.h>
#include <Methane/Graphics/RHI/Buffer.h>
#include <Methane/Graphics/RHI/Texture.h>
#include <Methane/Graphics/RHI/Sampler.h>
#include <Methane/Graphics/Null/Program.h>

#include <memory>
#include <taskflow/taskflow.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace Methane;
using namespace Methane::Graphics;

static tf::Executor g_parallel_executor;

TEST_CASE("RHI Program Bindings Functions", "[rhi][program][bindings]")
{
    const Rhi::ComputeContext compute_context = Rhi::ComputeContext(GetTestDevice(), g_parallel_executor, {});
    const Rhi::Program compute_program = [&compute_context]()
        {
            const Rhi::ProgramArgumentAccessor texture_accessor{ Rhi::ShaderType::Compute, "InTexture", Rhi::ProgramArgumentAccessType::Constant };
            const Rhi::ProgramArgumentAccessor sampler_accessor{ Rhi::ShaderType::Compute, "InSampler", Rhi::ProgramArgumentAccessType::Constant };
            const Rhi::ProgramArgumentAccessor buffer_accessor { Rhi::ShaderType::Compute, "OutBuffer", Rhi::ProgramArgumentAccessType::Mutable };
            Rhi::Program compute_program = compute_context.CreateProgram(
                Rhi::ProgramSettingsImpl
                {
                    Rhi::ProgramSettingsImpl::ShaderSet
                    {
                        { Rhi::ShaderType::Compute, { Data::ShaderProvider::Get(), { "Compute", "Main" } } }
                    },
                    Rhi::ProgramInputBufferLayouts{ },
                    Rhi::ProgramArgumentAccessors
                    {
                        texture_accessor,
                        sampler_accessor,
                        buffer_accessor
                    }
                });
            dynamic_cast<Null::Program&>(compute_program.GetInterface()).InitArgumentBindings({
                { texture_accessor, { Rhi::ResourceType::Texture, 1U } },
                { sampler_accessor, { Rhi::ResourceType::Sampler, 1U } },
                { buffer_accessor,  { Rhi::ResourceType::Buffer,  1U } },
            });
            return compute_program;
        }();

    const Rhi::Texture texture = [&compute_context]()
    {
        Rhi::Texture texture = compute_context.CreateTexture(Rhi::TextureSettings::ForImage(Dimensions(640, 480), {}, PixelFormat::RGBA8, false));
        texture.SetName("T");
        return texture;
    }();

    const Rhi::Sampler sampler = [&compute_context]()
    {
        const Rhi::Sampler sampler = compute_context.CreateSampler({
            rhi::SamplerFilter  { rhi::SamplerFilter::MinMag::Linear },
            rhi::SamplerAddress { rhi::SamplerAddress::Mode::ClampToEdge }
        });
        sampler.SetName("S");
        return sampler;
    }();

    const Rhi::Buffer buffer1 = [&compute_context]()
    {
        const Rhi::Buffer buffer = compute_context.CreateBuffer(Rhi::BufferSettings::ForConstantBuffer(42000, false, true));
        buffer.SetName("B1");
        return buffer;
    }();

    const Rhi::Buffer buffer2 = [&compute_context]()
        {
            const Rhi::Buffer buffer = compute_context.CreateBuffer(Rhi::BufferSettings::ForConstantBuffer(64000, false, true));
            buffer.SetName("B2");
            return buffer;
        }();

    const Rhi::Program::ResourceViewsByArgument compute_resource_views{
        { { Rhi::ShaderType::All, "InTexture" }, { { texture.GetInterface() } } },
        { { Rhi::ShaderType::All, "InSampler" }, { { sampler.GetInterface() } } },
        { { Rhi::ShaderType::All, "OutBuffer" }, { { buffer1.GetInterface() } } },
    };

    SECTION("Create Compute Program Bindings")
    {
        Rhi::ProgramBindings program_bindings;
        REQUIRE_NOTHROW(program_bindings = compute_program.CreateBindings(compute_resource_views, 2U));
        REQUIRE(program_bindings.IsInitialized());
        CHECK(program_bindings.GetInterfacePtr());
        CHECK(program_bindings.GetArguments().size() == 3U);
        CHECK(program_bindings.GetFrameIndex() == 2U);
        CHECK(program_bindings.Get({ Rhi::ShaderType::All, "InTexture" }).GetResourceViews().at(0).GetResourcePtr().get() == texture.GetInterfacePtr().get());
        CHECK(program_bindings.Get({ Rhi::ShaderType::All, "InSampler" }).GetResourceViews().at(0).GetResourcePtr().get() == sampler.GetInterfacePtr().get());
        CHECK(program_bindings.Get({ Rhi::ShaderType::All, "OutBuffer" }).GetResourceViews().at(0).GetResourcePtr().get() == buffer1.GetInterfacePtr().get());
    }

    SECTION("Can not create Compute Program Bindings with Unbound Resources")
    {
        Rhi::ProgramBindings program_bindings;
        REQUIRE_THROWS_AS(program_bindings = compute_program.CreateBindings({
                { { Rhi::ShaderType::All, "InTexture" }, { { texture.GetInterface() } } },
                { { Rhi::ShaderType::All, "OutBuffer" }, { { buffer1.GetInterface() } } }
            }),
            Rhi::ProgramBindingsUnboundArgumentsException);
    }

    SECTION("Create Multiple Compute Program Bindings")
    {
        std::vector<Rhi::ProgramBindings> program_bindings;
        for(size_t i = 0; i < 10; ++i)
        {
            REQUIRE_NOTHROW(program_bindings.push_back(compute_program.CreateBindings(compute_resource_views)));
            REQUIRE(program_bindings.back().IsInitialized());
            CHECK(program_bindings.back().GetArguments().size() == 3U);
            CHECK(program_bindings.back().GetBindingsIndex() == i);
        }
        CHECK(compute_program.GetBindingsCount() == 10);
        program_bindings.clear();
        // FIXME: CHECK(compute_program.GetBindingsCount() == 0);
    }

    SECTION("Create A Copy of Program Bindings with Replacements")
    {
        Rhi::ProgramBindings orig_program_bindings = compute_program.CreateBindings(compute_resource_views, 2U);
        Rhi::ProgramBindings copy_program_bindings;
        REQUIRE_NOTHROW(copy_program_bindings = Rhi::ProgramBindings(orig_program_bindings, {
            { { Rhi::ShaderType::All, "OutBuffer" }, { { buffer2.GetInterface() } } },
        }, 3U));
        REQUIRE(copy_program_bindings.IsInitialized());
        CHECK(copy_program_bindings.GetInterfacePtr());
        CHECK(copy_program_bindings.GetArguments().size() == 3U);
        CHECK(copy_program_bindings.GetFrameIndex() == 3U);
        CHECK(copy_program_bindings.Get({ Rhi::ShaderType::All, "InTexture" }).GetResourceViews().at(0).GetResourcePtr().get() == texture.GetInterfacePtr().get());
        CHECK(copy_program_bindings.Get({ Rhi::ShaderType::All, "InSampler" }).GetResourceViews().at(0).GetResourcePtr().get() == sampler.GetInterfacePtr().get());
        CHECK(copy_program_bindings.Get({ Rhi::ShaderType::All, "OutBuffer" }).GetResourceViews().at(0).GetResourcePtr().get() == buffer2.GetInterfacePtr().get());
    }

    SECTION("Object Destroyed Callback")
    {
        auto program_bindings_ptr = std::make_unique<Rhi::ProgramBindings>(compute_program, compute_resource_views);
        ObjectCallbackTester object_callback_tester(*program_bindings_ptr);
        CHECK_FALSE(object_callback_tester.IsObjectDestroyed());
        program_bindings_ptr.reset();
        CHECK(object_callback_tester.IsObjectDestroyed());
    }

    const Rhi::ProgramBindings program_bindings = compute_program.CreateBindings(compute_resource_views);

    SECTION("Object Name Setup")
    {
        CHECK(compute_program.SetName("My Program"));
        CHECK(compute_program.GetName() == "My Program");
    }

    SECTION("Object Name Change Callback")
    {
        CHECK(compute_program.SetName("My Program"));
        ObjectCallbackTester object_callback_tester(compute_program);
        CHECK(compute_program.SetName("Our Program"));
        CHECK(object_callback_tester.IsObjectNameChanged());
        CHECK(object_callback_tester.GetCurObjectName() == "Our Program");
        CHECK(object_callback_tester.GetOldObjectName() == "My Program");
    }

    SECTION("Object Name Set Unchanged")
    {
        CHECK(compute_program.SetName("My Compute Context"));
        ObjectCallbackTester object_callback_tester(compute_program);
        CHECK_FALSE(compute_program.SetName("My Compute Context"));
        CHECK_FALSE(object_callback_tester.IsObjectNameChanged());
    }

    SECTION("Can Get Program Binding Arguments")
    {
        Rhi::ProgramArguments program_arguments;
        REQUIRE(program_bindings.GetArguments().size() == 3U);
        REQUIRE_NOTHROW(program_arguments = program_bindings.GetArguments());
        CHECK(program_arguments.count({ Rhi::ShaderType::All, "InTexture" }) == 1);
        CHECK(program_arguments.count({ Rhi::ShaderType::All, "InSampler" }) == 1);
        CHECK(program_arguments.count({ Rhi::ShaderType::All, "OutBuffer" }) == 1);
    }

    SECTION("Can Get Texture Argument Binding")
    {
        Rhi::IProgramArgumentBinding* texture_binding_ptr = nullptr;
        REQUIRE_NOTHROW(texture_binding_ptr = &program_bindings.Get({ Rhi::ShaderType::All, "InTexture" }));
        REQUIRE(texture_binding_ptr);
        CHECK(texture_binding_ptr->GetSettings().argument.GetName() == "InTexture");
        CHECK(texture_binding_ptr->GetSettings().resource_count == 1U);
        CHECK(texture_binding_ptr->GetSettings().resource_type == Rhi::ResourceType::Texture);
        CHECK(texture_binding_ptr->GetResourceViews().size() == 1U);
        CHECK(texture_binding_ptr->GetResourceViews().at(0).GetResourcePtr().get() == texture.GetInterfacePtr().get());
    }

    SECTION("Can Get Sampler Argument Binding")
    {
        Rhi::IProgramArgumentBinding* sampler_binding_ptr = nullptr;
        REQUIRE_NOTHROW(sampler_binding_ptr = &program_bindings.Get({ Rhi::ShaderType::All, "InSampler" }));
        REQUIRE(sampler_binding_ptr);
        CHECK(sampler_binding_ptr->GetSettings().argument.GetName() == "InSampler");
        CHECK(sampler_binding_ptr->GetSettings().resource_count == 1U);
        CHECK(sampler_binding_ptr->GetSettings().resource_type == Rhi::ResourceType::Sampler);
        CHECK(sampler_binding_ptr->GetResourceViews().size() == 1U);
        CHECK(sampler_binding_ptr->GetResourceViews().at(0).GetResourcePtr().get() == sampler.GetInterfacePtr().get());
    }

    SECTION("Can Get Buffer Argument Binding")
    {
        Rhi::IProgramArgumentBinding* buffer_binding_ptr = nullptr;
        REQUIRE_NOTHROW(buffer_binding_ptr = &program_bindings.Get({ Rhi::ShaderType::All, "OutBuffer" }));
        REQUIRE(buffer_binding_ptr);
        CHECK(buffer_binding_ptr->GetSettings().argument.GetName() == "OutBuffer");
        CHECK(buffer_binding_ptr->GetSettings().resource_count == 1U);
        CHECK(buffer_binding_ptr->GetSettings().resource_type == Rhi::ResourceType::Buffer);
        CHECK(buffer_binding_ptr->GetResourceViews().size() == 1U);
        CHECK(buffer_binding_ptr->GetResourceViews().at(0).GetResourcePtr().get() == buffer1.GetInterfacePtr().get());
    }

    SECTION("Can Change Buffer Argument Binding")
    {
        Rhi::IProgramArgumentBinding& buffer_binding = program_bindings.Get({ Rhi::ShaderType::All, "OutBuffer" });
        REQUIRE_NOTHROW(buffer_binding.SetResourceViews({ { buffer2.GetInterface(), 0U, 128U } }));
        const Rhi::ResourceView& buffer_view = buffer_binding.GetResourceViews().at(0);
        CHECK(buffer_view.GetResourcePtr().get() == buffer2.GetInterfacePtr().get());
        CHECK(buffer_view.GetOffset() == 0U);
        CHECK(buffer_view.GetSize() == 128U);
    }

    SECTION("Convert to String")
    {
        CHECK(static_cast<std::string>(program_bindings) == 
              "  - Compute shaders argument 'InSampler' (Constant) is bound to Sampler 'S' subresources from index(d:0, a:0, m:0) for count(d:0, a:0, m:0) with offset 0;\n" \
              "  - Compute shaders argument 'InTexture' (Constant) is bound to Texture 'T' subresources from index(d:0, a:0, m:0) for count(d:1, a:1, m:1) with offset 0;\n" \
              "  - Compute shaders argument 'OutBuffer' (Mutable) is bound to Buffer 'B1' subresources from index(d:0, a:0, m:0) for count(d:1, a:1, m:1) with offset 0.");
    }
}