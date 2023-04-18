#include "fsr_stage.hh"
#include "misc.hh"

#include <FSR/vk/ffx_fsr2_vk.h>

using namespace tr;

namespace tr
{

fsr_stage::fsr_stage(
    device_data& dev,
    const options& opt
):  stage(dev),
    opt(opt),
    stage_timer(dev, "FSR stage")
{
    init_resources();
}

void fsr_stage::set_scene(scene* cur_scene)
{
    this->cur_scene = cur_scene;
    init_resources();
}

void fsr_stage::update(uint32_t frame_index)
{
    tr::camera& camera = cur_scene->get_camera();
    float farPlane  = camera.get_far();
    float nearPlane = camera.get_near();

    FfxFsr2DispatchDescription dispatchParameters = {};
    dispatchParameters.commandList = ffxGetCommandListVK(commandBuffer);

    //We need unresolved color texture, depth texture and motion vectors
    //dispatchParameters.color = ffxGetTextureResourceVK(&context, , cameraSetup.unresolvedColorResourceView, cameraSetup.unresolvedColorResource->GetWidth(), cameraSetup.unresolvedColorResource->GetHeight(), cameraSetup.unresolvedColorResource->GetFormat(), L"FSR2_InputColor");
    //dispatchParameters.depth = ffxGetTextureResourceVK(&context, , cameraSetup.depthbufferResourceView, cameraSetup.depthbufferResource->GetWidth(), cameraSetup.depthbufferResource->GetHeight(), cameraSetup.depthbufferResource->GetFormat(), L"FSR2_InputDepth");
    //dispatchParameters.motionVectors = ffxGetTextureResourceVK(&context, cameraSetup.motionvectorResource->Resource(), cameraSetup.motionvectorResourceView, cameraSetup.motionvectorResource->GetWidth(), cameraSetup.motionvectorResource->GetHeight(), cameraSetup.motionvectorResource->GetFormat(), L"FSR2_InputMotionVectors");
    dispatchParameters.exposure = ffxGetTextureResourceVK(&context, nullptr, nullptr, 1, 1, VK_FORMAT_UNDEFINED, L"FSR2_InputExposure");

    // We don't have reactive, transparency and compositions masks for now.
    dispatchParameters.reactive = ffxGetTextureResourceVK(&context, nullptr, nullptr, 1, 1, VK_FORMAT_UNDEFINED, L"FSR2_EmptyInputReactiveMap");
    dispatchParameters.transparencyAndComposition = ffxGetTextureResourceVK(&context, nullptr, nullptr, 1, 1, VK_FORMAT_UNDEFINED, L"FSR2_EmptyTransparencyAndCompositionMap");
    
    // Final output texture
    dispatchParameters.output = ffxGetTextureResourceVK(&context, cameraSetup.resolvedColorResource->Resource(), cameraSetup.resolvedColorResourceView, cameraSetup.resolvedColorResource->GetWidth(), cameraSetup.resolvedColorResource->GetHeight(), cameraSetup.resolvedColorResource->GetFormat(), L"FSR2_OutputUpscaledColor", FFX_RESOURCE_STATE_UNORDERED_ACCESS);
    //Jitter offset
    vec2& jitter = camera.get_jitter();
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
    dispatchParameters.frameTimeDelta = 0;//?? delta_time;
    dispatchParameters.preExposure = 1.0f;
    dispatchParameters.renderSize.width = render_width;
    dispatchParameters.renderSize.height = render_height;
    dispatchParameters.cameraFar = farPlane;
    dispatchParameters.cameraNear = nearPlane;
    dispatchParameters.cameraFovAngleVertical = camera.get_vfov();
    pState->bReset = false;

    FfxErrorCode errorCode = ffxFsr2ContextDispatch(&context, &dispatchParameters);
    FFX_ASSERT(errorCode == FFX_OK);
}

void fsr_stage::init_resources()
{
    // Calculating render resolution
    render_width  = opt.display_width/opt.scale_factor;
    render_height = opt.display_height/opt.scale_factor;

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



