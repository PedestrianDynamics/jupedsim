#pragma once

#include<memory>

class AILandmark;
class AIConnection;


class AIAssociation
{
public:
    AIAssociation();
    AIAssociation(const AILandmark* landmark, const AILandmark* associated_landmark, bool connected=false);
    AIAssociation(const AIConnection* connection);
    ~AIAssociation();
    const AILandmark* GetLandmarkAssociation(const AILandmark* landmark) const;
    const AIConnection* GetConnectionAssoziation() const;
    bool operator==(const AIAssociation& asso2) const;
private:
    const AILandmark* _landmark;
    const AILandmark* _associatedLandmark;
    const AIConnection* _connection;
};
