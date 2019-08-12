#pragma once

#include<memory>

class Landmark;
using ptrLandmark = std::shared_ptr<Landmark>;
class Connection;
using ptrConnection = std::shared_ptr<Connection>;

class Association
{
public:
    Association();
    Association(ptrLandmark landmark, ptrLandmark associated_landmark, bool connected=false);
    Association(ptrConnection connection);
    ~Association();
    ptrLandmark GetLandmarkAssociation(ptrLandmark landmark) const;
    ptrConnection GetConnectionAssoziation() const;
private:
    ptrLandmark _landmark;
    ptrLandmark _associatedLandmark;
    ptrConnection _connection;
};
