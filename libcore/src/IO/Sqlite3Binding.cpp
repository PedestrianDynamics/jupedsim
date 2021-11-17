#include "Sqlite3Binding.h"

#include <exception>
#include <optional>
#include <stdexcept>

static void ThrowLastError(sqlite3 * db)
{
    const auto * error_msg = sqlite3_errmsg(db);
    throw Sqlite3Exception(error_msg);
}

Sqlite3PreparedStatement::Sqlite3PreparedStatement(const Sqlite3DB & db, const std::string & query)
{
    const auto rc =
        sqlite3_prepare_v2(static_cast<sqlite3 *>(db), query.data(), query.size(), &_stmt, nullptr);
    if(rc != SQLITE_OK) {
        ThrowLastError(static_cast<sqlite3 *>(db));
    }

    //sqlite3_
}

int Sqlite3PreparedStatement::Step()
{
    return sqlite3_step(_stmt);
}

int Sqlite3PreparedStatement::Reset()
{
    return sqlite3_reset(_stmt);
}

Sqlite3PreparedStatement::~Sqlite3PreparedStatement()
{
    sqlite3_finalize(_stmt);
}

Sqlite3Exception::Sqlite3Exception(const char * msg) : std::runtime_error(msg) {}

Sqlite3DB::Sqlite3DB(const std::optional<fs::path> & opt_path)
{
    const auto rc = sqlite3_open(opt_path.value_or(":memory:").c_str(), &_db);
    if(rc != SQLITE_OK) {
        Close();
        ThrowLastError(_db);
    }
}

Sqlite3DB::~Sqlite3DB()
{
    Close();
}

void Sqlite3DB::Close() noexcept
{
    if(_db == nullptr) {
        return;
    }
    const auto rc = sqlite3_close(_db);
    if(rc != SQLITE_OK) {
        std::abort();
    }
}

Sqlite3DB::operator sqlite3 *() const noexcept
{
    return _db;
}
