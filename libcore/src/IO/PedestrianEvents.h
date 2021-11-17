#include "events/Event.h"
#include "general/Filesystem.h"
#include "sqlite3.h"

#include <vector>

std::vector<CreatePedestrianEvent> LoadCreatePedestrianEvents(const fs::path & path);

void AppendCreatePedestrianEvents(
    const std::vector<CreatePedestrianEvent> & events,
    const fs::path & path);
