#include "PedestrianEvents.h"

std::vector<CreatePedestrianEvent> LoadCreatePedestrianEvents(const fs::path & path)
{
    sqlite3 * db;
    auto rc = sqlite3_open(path.c_str(), &db);
    if(rc != 0) {
        // TODO(kkratz)
        return {};
    }

    char * error_msg{nullptr};
    size_t size{0};
    sqlite3_exec(
        db,
        "select count * from pedestrian_events",
        [](void * size, int num_rows, char ** row, char ** header) {
            auto * as_size = static_cast<size_t *>(size);
            *as_size       = std::stoll(row[0]);
            return 0;
        },
        static_cast<void *>(&size),
        &error_msg);
    sqlite3_free(error_msg);
}


void AppendCreatePedestrianEvents(
    const std::vector<CreatePedestrianEvent> & events,
    const fs::path & path)
{
    sqlite3 * db;
    auto rc = sqlite3_open(path.c_str(), &db);
    if(rc != 0) {
        //TODO handle error
    }

    //    sqlite3_prepa
}
