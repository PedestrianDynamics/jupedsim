#include "youareherepointer.h"

YouAreHerePointer::YouAreHerePointer()
{

}

YouAreHerePointer::~YouAreHerePointer()
{

}

Point YouAreHerePointer::GetPos() const
{
    return _pos;
}

void YouAreHerePointer::SetPos(const Point &point)
{
    _pos=point;
}

