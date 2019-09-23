#include "associations.h"

#include "connection.h"

Association::Association()
{
    _landmark=nullptr;
    _associatedLandmark=nullptr;

}

Association::Association(ptrLandmark landmark, ptrLandmark associated_landmark, bool /*connected*/)
{
    _landmark=landmark;
    _associatedLandmark=associated_landmark;

//    if (connected)
//        _connection = std::make_shared<Connection>(_landmark, _associatedLandmark);

//    else
         _connection=nullptr;


}

Association::Association(ptrConnection connection)
{
    _connection=connection;
    _landmark=nullptr;
    _associatedLandmark=nullptr;
}

Association::~Association()
{

}

ptrLandmark Association::GetLandmarkAssociation(ptrLandmark landmark) const
{
    if (landmark==nullptr)
        return nullptr;
    if (_landmark==landmark)
    {
        return _associatedLandmark;
    }
    else
        return nullptr;

}

ptrConnection Association::GetConnectionAssoziation() const
{
    return _connection;
}
