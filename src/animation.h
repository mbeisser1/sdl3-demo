#ifndef ANIMATION_H
#define ANIMATION_H

#include "timer.h"

class Animation
{
    public:
    Animation() = default;
    
    Animation(int frame_count, float step_length) : frame_count_{frame_count}, timer_(step_length){}

    float getLength() const {return timer_.getLength();}
    
    // current frame of animation we need to display for sprite
    int currentFrame() const
    {
        // current frame we should be displaying on screen, frame only goes up by whole integers
        // timer value / length value = (0 -> 1)
        return static_cast<int>((timer_.getTime() / timer_.getLength()) * frame_count_);
    }

    void step(float delta_time)
    {
        timer_.step(delta_time);
    }

    void reset()
    {
        timer_.reset();
    }

    private:
    Timer timer_{0};
    int frame_count_{};    
};

#endif