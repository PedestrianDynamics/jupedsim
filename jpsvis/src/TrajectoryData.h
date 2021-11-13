#pragma once
#include "Frame.h"

#include <memory>
#include <vector>

class TrajectoryData
{
    std::vector<std::unique_ptr<Frame>> _frames{};
    int _frameCursor{0};
    double _fps{0};

public:
    TrajectoryData()  = default;
    ~TrajectoryData() = default;

    void resetFrameCursor();

    /// get the size
    unsigned int getSize();

    /// add a frame to the synchronized data
    void append(std::unique_ptr<Frame> && frame);

    /// clears all frames
    void clearFrames();

    /// returns the total number of frames
    int getFrameCount();

    /// Access the FPS this data was recorded with.
    /// @return fps the data was recored at
    double getFps() const;

    /// Set the FPS this data was recorded with.
    /// @param val, new fps value
    void setFps(double val);

    /// return the position of the actual frame
    int currentIndex();

    /// set to cursor position
    void moveToFrame(int position);

    /// Increment or decrement frame counter by 'count' frmaes. Movement is clamped to the number
    /// actual of actual available frames.
    /// @param count frames to move.
    void moveFrameBy(int count);

    Frame * currentFrame();
};
