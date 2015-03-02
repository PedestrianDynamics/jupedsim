#include "knot.h"

Knot::Knot()
{

}

Knot::Knot(const double &x, const double &y, const double &value)
{
    _x=x;
    _y=y;
    _value=value;

}

Knot::~Knot()
{

}

void Knot::SetValue(const double &value)
{
    _value=value;
}

const double &Knot::GetValue() const
{
    return _value;
}

