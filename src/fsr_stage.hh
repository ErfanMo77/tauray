#ifndef TAURAY_FSR_STAGE_HH
#define TAURAY_FSR_STAGE_HH
#include "context.hh"
#include "texture.hh"
#include "stage.hh"
#include "compute_pipeline.hh"
#include "timer.hh"
#include "gbuffer.hh"
#include "gpu_buffer.hh"
#include "scene.hh"
#include "sampler.hh"

namespace tr
{

class scene;
class fsr_stage: public stage
{
public:

    fsr_stage(
        device_data& dev,
        uint32_t frame_index,
        uint32_t pass_index,
        uvec3 expected_dispatch_size
    );

protected:
    void init_scene_resources() override;
    void record_command_buffer_pass(
        vk::CommandBuffer cb,
        uint32_t frame_index,
        uint32_t pass_index,
        uvec3 expected_dispatch_size
    ) override;

private:
    gfx_pipeline gfx;
    options opt;
};

}

#endif
