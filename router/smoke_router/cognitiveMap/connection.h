#pragma once

#include "landmark.h"

using ptrLandmark = std::shared_ptr<Landmark>;
using Landmarks = std::vector<ptrLandmark>;

class Connection
{
public:
    Connection(const int& id, const std::string& caption, const std::string& type, const ptrLandmark& landmark1,const ptrLandmark& landmark2);
    ~Connection();

    //Getter
    const int& GetId() const;
    const std::string& GetCaption() const;
    const std::string& GetType() const;

    std::pair<ptrLandmark,ptrLandmark> GetLandmarks() const;

    //Setter
    void SetId(const int& id);
    void SetCaption(const std::string& caption);
    void SetType(const std::string& type);

private:
    int _id;
    std::string _caption;
    std::string _type;
    ptrLandmark _landmark1;
    ptrLandmark _landmark2;
};
