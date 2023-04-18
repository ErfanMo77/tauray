#ifndef TAURAY_FSR_STAGE_HH
#define TAURAY_FSR_STAGE_HH

#include "context.hh"
#include "stage.hh"
#include "timer.hh"
#include "scene.hh"

#include <FSR/ffx_fsr2.h>

namespace tr
{

class fsr_stage: public stage
{
public:
    struct options
    {
        size_t display_width = 1920;
        size_t display_height = 1080;
        float scale_factor = 1.0f;
    };

    fsr_stage(
        device_data& dev,
        const options& opt
    );
    fsr_stage(const fsr_stage& other) = delete;
    fsr_stage(fsr_stage&& other) = delete;

    void set_scene(scene* cur_scene);
    virtual void update(uint32_t frame_index) override;

    void init_resources();

private:
    FfxFsr2Context fsr2_context;
    FfxFsr2ContextDescription fsr2_context_desc;
    std::vector<vec4> jitter_history;

    uint32_t render_width, render_height;
    options opt;
    scene* cur_scene;
    timer stage_timer;
};

}

#endif
