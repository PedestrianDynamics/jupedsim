#pragma once

#include "landmark.h"

using AILandmarks = std::vector<AILandmark>;

class AIConnection
{
public:
    AIConnection(int id, const std::string& caption, const std::string& type, int landmarkId1, int landmarkId2);
    ~AIConnection();

    //Getter
    int GetId() const;
    const std::string& GetCaption() const;
    const std::string& GetType() const;

    std::pair<int,int> GetLandmarkIds() const;

    //Setter
    void SetId(int id);
    void SetCaption(const std::string& caption);
    void SetType(const std::string& type);

private:
    int _id;
    std::string _caption;
    std::string _type;
    int _landmarkId1;
    int _landmarkId2;
};
