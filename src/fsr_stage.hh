#ifndef TAURAY_FSR_STAGE_HH
#define TAURAY_FSR_STAGE_HH

#include "context.hh"
#include "stage.hh"
#include "timer.hh"

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


private:
    void init_resources();
    virtual void update(uint32_t frame_index) override;
    
    FfxFsr2Context fsr2_context;
    FfxFsr2ContextDescription fsr2_context_desc;
    std::vector<vec4> jitter_history;
    options opt;
    timer stage_timer;
};

}

#endif
