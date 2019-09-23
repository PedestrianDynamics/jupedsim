#include "associations.h"

#include "connection.h"

AIAssociation::AIAssociation()
{
    _landmark=nullptr;
    _associatedLandmark=nullptr;

}

AIAssociation::AIAssociation(const AILandmark *landmark, const AILandmark *associated_landmark, bool /*connected*/)
{
    _landmark=landmark;
    _associatedLandmark=associated_landmark;

//    if (connected)
//        _connection = std::make_shared<Connection>(_landmark, _associatedLandmark);

//    else
         _connection=nullptr;


}

AIAssociation::AIAssociation(const AIConnection *connection)
{
    _connection=connection;
    _landmark=nullptr;
    _associatedLandmark=nullptr;
}

AIAssociation::~AIAssociation()
{

}

const AILandmark *AIAssociation::GetLandmarkAssociation(const AILandmark *landmark) const
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

const AIConnection *AIAssociation::GetConnectionAssoziation() const
{
    return _connection;
}

bool AIAssociation::operator==(const AIAssociation &asso2) const
{
    return this==&asso2;
}
