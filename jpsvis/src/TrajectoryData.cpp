#include "TrajectoryData.h"

#include <algorithm>

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

void TrajectoryData::append(std::unique_ptr<Frame> && frame)
{
    _frames.emplace_back(std::move(frame));
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
    if(position >= 0 && position < static_cast<int>(_frames.size())) {
        _frameCursor = position;
    } else {
        _frameCursor = 0;
    }
}

void TrajectoryData::moveFrameBy(int count)
{
    const auto newIndex = _frameCursor + count;
    const int low       = 0;
    const int high      = _frames.empty() ? 0 : static_cast<int>(_frames.size() - 1);
    _frameCursor        = std::clamp(newIndex, low, high);
}

Frame * TrajectoryData::currentFrame()
{
    return _frames[_frameCursor].get();
}
