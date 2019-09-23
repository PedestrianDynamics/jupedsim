//
// Created by erik on 30.12.16.
//
#include "sign.h"

Sign::Sign(int id, int room_id, const Point &pos, const double &alpha, const double &alphaPointing, const double &width, const double &height) : _id(id), _roomId(room_id), _pos(pos), _alpha(alpha),_alphaPointing(alphaPointing),_width(width), _height(height) {


}

Sign::~Sign() {

}

const Point &Sign::GetPos() const {
    return _pos;
}

double Sign::GetAlpha() const {
    return _alpha;
}

double Sign::GetAlphaPointing() const
{
    return _alphaPointing;
}

void Sign::SetPos(const Point &pos) {
    _pos = pos;
}

void Sign::SetAlpha(const double& alpha) {
    _alpha = alpha;
}

void Sign::SetAlphaPointing(const double &alpha)
{
    _alphaPointing=alpha;
}

int Sign::GetId() const {
    return _id;
}

int Sign::GetRoomId() const
{
    return _roomId;
}

const double &Sign::GetWidth() const
{
    return _width;
}

const double &Sign::GetHeight() const
{
    return _height;
}

void Sign::SetId(int id) {

    _id=id;
}

void Sign::SetRoomId(int room_id)
{
    _roomId=room_id;
}

void Sign::SetWidth(const double &width)
{
    _width=width;
}

void Sign::SetHeight(const double &height)
{
    _height=height;
}
