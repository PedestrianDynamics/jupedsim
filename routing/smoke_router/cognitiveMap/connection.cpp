#include "connection.h"

Connection::Connection(const int &id, const std::string &caption, const std::string &type, const ptrLandmark &landmark1, const ptrLandmark &landmark2)
{
    _landmark1=landmark1;
    _landmark2=landmark2;
    _id=id;
    _caption=caption;
    _type=type;
}

Connection::~Connection()
{

}

std::pair<ptrLandmark, ptrLandmark> Connection::GetLandmarks() const
{
    return std::make_pair(_landmark1, _landmark2);

}

const int &Connection::GetId() const
{
    return _id;
}

const std::string &Connection::GetCaption() const
{
    return _caption;
}

const std::string &Connection::GetType() const
{
    return _type;
}

void Connection::SetId(const int &id)
{
    _id=id;
}

void Connection::SetCaption(const std::string &caption)
{
    _caption=caption;
}

void Connection::SetType(const std::string &type)
{
    _type=type;
}
