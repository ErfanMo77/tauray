#include "fsr_stage.hh"
#include "misc.hh"

#include "fsr_linux/src/ffx-fsr2-api/vk/ffx_fsr2_vk.h"

using namespace tr;

namespace tr
{

fsr_stage::fsr_stage(
    device_data& dev,
    gbuffer_target& input_features,
    render_target& output_target,
    const options& opt
):  stage(dev),
    input_features(input_features),
    opt(opt),
    stage_timer(dev, "FSR stage")
{
    init_resources();
    out_color = output_target;
}

void fsr_stage::set_scene(scene* cur_scene)
{
    this->cur_scene = cur_scene;
}

void fsr_stage::update(uint32_t frame_index)
{
    clear_commands();

    // Updating the jitter buffer
    // bool existing = jitter_history.size() != 0;
    // jitter_history.resize(1);

    // vec4& v = jitter_history[0];
    // vec2 cur_jitter = cur_scene->get_camera(0)->get_jitter();
    // vec2 prev_jitter = v;
    // if(!existing)
    //     prev_jitter = cur_jitter;
    // v = vec4(cur_jitter, prev_jitter);

    // jitter_buffer.update(frame_index, jitter_history.data());

    // Recording the FSR command buffers
    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vk::CommandBuffer cb = begin_compute();
        stage_timer.begin(cb, i);

        auto camera = cur_scene->get_camera();
        float farPlane = camera->get_far();
        float nearPlane = camera->get_near();

        FfxFsr2DispatchDescription dispatchParameters = {};
        dispatchParameters.commandList = ffxGetCommandListVK(cb);

        //We need unresolved color texture, depth texture and motion vectors
        dispatchParameters.color = ffxGetTextureResourceVK(&fsr2_context,
        input_features.color[i].image,
        input_features.color[i].view,
        render_width,
        render_height,
        (VkFormat)input_features.color.get_format(),
         L"FSR2_InputColor");

        dispatchParameters.depth = ffxGetTextureResourceVK(&fsr2_context,
        input_features.depth[i].image,
        input_features.depth[i].view,
        render_width,
        render_height,
        (VkFormat)input_features.depth.get_format(),
         L"FSR2_InputDepth");

        dispatchParameters.motionVectors = ffxGetTextureResourceVK(&fsr2_context,
        input_features.screen_motion[i].image,
        input_features.screen_motion[i].view,
        render_width,
        render_height,
        (VkFormat)input_features.screen_motion.get_format(),
        L"FSR2_InputMotionVectors");
        dispatchParameters.exposure = ffxGetTextureResourceVK(
            &fsr2_context,
            nullptr,
            nullptr,
            1,
            1,
            VK_FORMAT_UNDEFINED,
            L"FSR2_InputExposure"
        );

        // We don't have reactive, transparency and compositions masks for now.
        dispatchParameters.reactive = ffxGetTextureResourceVK(
            &fsr2_context,
            nullptr,
            nullptr,
            1,
            1,
            VK_FORMAT_UNDEFINED,
            L"FSR2_EmptyInputReactiveMap"
        );
        dispatchParameters.transparencyAndComposition = ffxGetTextureResourceVK(
            &fsr2_context,
            nullptr,
            nullptr,
            1,
            1,
            VK_FORMAT_UNDEFINED,
            L"FSR2_EmptyTransparencyAndCompositionMap"
        );
        
        // Final output texture
        dispatchParameters.output = ffxGetTextureResourceVK(
            &fsr2_context,
            out_color[i].image,
            out_color[i].view,
            opt.display_width,
            opt.display_height,
            (VkFormat)out_color.get_format(),
            L"FSR2_OutputUpscaledColor",
            FFX_RESOURCE_STATE_UNORDERED_ACCESS
        );

        //Jitter offset
        vec2 jitter = camera->get_jitter();
        dispatchParameters.jitterOffset.x = jitter.x;
        dispatchParameters.jitterOffset.y = jitter.y;
        // render resolution
        dispatchParameters.motionVectorScale.x = (float)render_width;
        dispatchParameters.motionVectorScale.y = (float)render_height;
        //??
        dispatchParameters.reset = true;
        dispatchParameters.enableSharpening = true;
        //TODO: add options
        dispatchParameters.sharpness = 1.0f;
        dispatchParameters.frameTimeDelta = 0.017f; //?? delta_time;
        dispatchParameters.preExposure = 1.0f;
        dispatchParameters.renderSize.width = render_width;
        dispatchParameters.renderSize.height = render_height;
        dispatchParameters.cameraFar = farPlane;
        dispatchParameters.cameraNear = nearPlane;
        dispatchParameters.cameraFovAngleVertical = camera->get_vfov();

        FfxErrorCode errorCode = ffxFsr2ContextDispatch(&fsr2_context, &dispatchParameters);
        FFX_ASSERT(errorCode == FFX_OK);

        stage_timer.end(cb, i);
        end_compute(cb, i);
    }
}

void fsr_stage::init_resources()
{
    // Calculating render resolution
    render_width  = opt.display_width;
    render_height = opt.display_height;


    opt.display_width = render_width * opt.scale_factor;
    opt.display_height = render_height * opt.scale_factor;

    printf("Render resolution: %dx%d\n", render_width, render_height);
    printf("Display resolution: %dx%d\n", opt.display_width, opt.display_height);


    // Setup VK interface.
    const size_t scratchBufferSize = ffxFsr2GetScratchMemorySizeVK(dev->pdev);
    void* scratchBuffer = malloc(scratchBufferSize);
    FfxErrorCode errorCode = ffxFsr2GetInterfaceVK(&fsr2_context_desc.callbacks, scratchBuffer, scratchBufferSize, dev->pdev, vkGetDeviceProcAddr);
    FFX_ASSERT(errorCode == FFX_OK);

    // Context
    fsr2_context_desc.device = ffxGetDeviceVK(dev->dev);
    fsr2_context_desc.maxRenderSize.width = render_width;
    fsr2_context_desc.maxRenderSize.height = render_height;
    fsr2_context_desc.displaySize.width = opt.display_width;
    fsr2_context_desc.displaySize.height = opt.display_height;
    fsr2_context_desc.flags = FFX_FSR2_ENABLE_AUTO_EXPOSURE | FFX_FSR2_ENABLE_DEPTH_INVERTED | FFX_FSR2_ENABLE_HIGH_DYNAMIC_RANGE;

    ffxFsr2ContextCreate(&fsr2_context, &fsr2_context_desc);
}



}



