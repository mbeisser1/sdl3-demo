#ifndef TIMER_H
#define TIMER_H

class Timer
{
public:
    Timer() = default;
    Timer(float step_length) : step_length_{step_length}, elapsed_time_{0.0F}, timeout_{false}
    {

    }

    void step(float delta_time)
    {
        elapsed_time_ += delta_time;
        
        // We may have gone over our time, and we can't just ignore the total elapsed
        // time as that needs to go into our next calculation. We need the game clock
        // to be as close as we can to real time
        if(elapsed_time_ >= step_length_)
        {
            elapsed_time_ -= step_length_;
            timeout_ = true;
        }
    }

    bool isTimeout() const {return timeout_;}
    float getTime() const {return elapsed_time_;}
    float getLength() const {return step_length_;}
    void reset()
    {
        elapsed_time_ = 0;
        timeout_ = false;
    }

private:
        float step_length_{};
        float elapsed_time_{};
        bool timeout_{};
};

#endif