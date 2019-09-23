//
// Created by erik on 30.12.16.
//
#pragma once

#include "geometry/Point.h"

class Sign {
public:

    Sign(int id, int room_id, const Point &pos, const double& alpha, const double& alphaPointing, const double& width=0.3, const double &height=0.1);

    const Point &GetPos() const;
    double GetAlpha() const;
    double GetAlphaPointing() const;
    int GetId() const;
    int GetRoomId() const;
    const double& GetWidth() const;
    const double& GetHeight() const;

    void SetPos(const Point &pos);
    void SetAlpha(const double& alpha);
    void SetAlphaPointing(const double& alpha);
    void SetId(int id);
    void SetRoomId(int room_id);
    void SetWidth(const double& width);
    void SetHeight(const double& height);

    virtual ~Sign();

private:

    int _id;
    int _roomId;
    Point _pos;
    double _alpha; // in deg
    double _alphaPointing;
    double _width;
    double _height;


};
