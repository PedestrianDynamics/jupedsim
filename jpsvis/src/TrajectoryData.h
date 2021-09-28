#pragma once
#include "Frame.h"

#include <memory>
#include <vector>

class TrajectoryData
{
public:
    TrajectoryData()  = default;
    ~TrajectoryData() = default;

    void resetFrameCursor();

    /// get the size
    unsigned int getSize();

    /// add a frame to the synchronized data
    void append(Frame * frame);

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

    void incrementFrame();

    void decrementFrame();

    Frame * currentFrame();

    void updatePolyDataForFrames();

private:
    std::vector<std::unique_ptr<Frame>> _frames{};
    int _frameCursor{0};
    double _fps{0};
};
