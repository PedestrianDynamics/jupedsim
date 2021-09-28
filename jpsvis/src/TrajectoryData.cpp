#include "TrajectoryData.h"

void TrajectoryData::resetFrameCursor()
{
    _frameCursor = 0;
}

unsigned int TrajectoryData::getSize()
{
    if(_frames.empty())
        return 0;
    else
        return _frames.size();
}

void TrajectoryData::append(Frame * frame)
{
    _frames.push_back(std::unique_ptr<Frame>{frame});
}

void TrajectoryData::clearFrames()
{
    _frameCursor = 0;
    _frames.clear();
}

int TrajectoryData::getFrameCount()
{
    return _frames.size();
}

double TrajectoryData::getFps() const
{
    return _fps;
}

void TrajectoryData::setFps(double val)
{
    _fps = val;
}

int TrajectoryData::currentIndex()
{
    return _frameCursor;
}

void TrajectoryData::moveToFrame(int position)
{
    if(position >= 0 && position < _frames.size()) {
        _frameCursor = position;
    } else {
        _frameCursor = 0;
    }
}

void TrajectoryData::incrementFrame()
{
    const auto nextIndex = _frameCursor + 1;
    if(nextIndex < _frames.size()) {
        _frameCursor = nextIndex;
    }
}

void TrajectoryData::decrementFrame()
{
    const auto nextIndex = _frameCursor - 1;
    if(nextIndex >= 0) {
        _frameCursor = nextIndex;
    }
}

Frame * TrajectoryData::currentFrame()
{
    return _frames[_frameCursor].get();
}

void TrajectoryData::updatePolyDataForFrames()
{
    for(auto & frame : _frames) {
        frame->ComputePolyData();
    }
}
