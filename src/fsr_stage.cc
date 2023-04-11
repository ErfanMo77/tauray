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


void fsr_stage::init_resources()
{
    // Setup VK interface.
    const size_t scratchBufferSize = ffxFsr2GetScratchMemorySizeVK(dev->pdev);
    void* scratchBuffer = malloc(scratchBufferSize);
    FfxErrorCode errorCode = ffxFsr2GetInterfaceVK(&fsr2_context_desc.callbacks, scratchBuffer, scratchBufferSize, dev->pdev, vkGetDeviceProcAddr);
    FFX_ASSERT(errorCode == FFX_OK);

    // Context
    fsr2_context_desc.device = ffxGetDeviceVK(dev->dev);
    fsr2_context_desc.maxRenderSize.width = opt.display_width/opt.scale_factor;
    fsr2_context_desc.maxRenderSize.height = opt.display_height/opt.scale_factor;
    fsr2_context_desc.displaySize.width = opt.display_width;
    fsr2_context_desc.displaySize.height = opt.display_height;
    fsr2_context_desc.flags = FFX_FSR2_ENABLE_AUTO_EXPOSURE | FFX_FSR2_ENABLE_DEPTH_INVERTED | FFX_FSR2_ENABLE_HIGH_DYNAMIC_RANGE;

    ffxFsr2ContextCreate(&fsr2_context, &fsr2_context_desc);
}

void fsr_stage::update(uint32_t frame_index)
{
   
}



}



