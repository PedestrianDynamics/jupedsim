#include "knot.h"

knot::knot()
{

}

knot::knot(const double &x, const double &y, const double &value)
{
    _x=x;
    _y=y;
    _value=value;

}

knot::~knot()
{

}

void knot::setValue(const double &value)
{
    _value=value;
}

const double &knot::getValue() const
{
    return _value;
}

