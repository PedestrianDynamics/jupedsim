#include "IO/Sqlite3Binding.h"

#include <gtest/gtest.h>
#include <optional>

TEST(Sqlite3, CanConstructInMemory)
{
    Sqlite3DB db(std::nullopt);
}

TEST(Sqlite3, CanCreatePreparedStatement)
{
    Sqlite3DB db(std::nullopt);
    Sqlite3PreparedStatement stmt(db, "create table t(x integer, y text, z real)");
}

TEST(Sqlite3, CanCreateTable)
{
    Sqlite3DB db(std::nullopt);
    Sqlite3PreparedStatement stmt(db, "create table t(x integer, y text, z real)");
    ASSERT_EQ(stmt.Step(), SQLITE_DONE);
}

TEST(Sqlite3, CanExecuteStatementMultipleTimes)
{
    Sqlite3DB db(std::nullopt);
    Sqlite3PreparedStatement create_stmt(db, "create table t(x integer, y text, z real)");
    ASSERT_EQ(create_stmt.Step(), SQLITE_DONE);

    Sqlite3PreparedStatement count_star_stmt(db, "select count(*) from t");
    ASSERT_EQ(count_star_stmt.Step(), SQLITE_ROW);
    ASSERT_EQ(count_star_stmt.Step(), SQLITE_DONE);
    ASSERT_EQ(count_star_stmt.Reset(), SQLITE_OK);
    ASSERT_EQ(count_star_stmt.Step(), SQLITE_ROW);
    ASSERT_EQ(count_star_stmt.Step(), SQLITE_DONE);
}

TEST(Sqlite3, CanInsertDataFromTuple)
{
    Sqlite3DB db(std::nullopt);
    Sqlite3PreparedStatement stmt(db, "create table t(x integer, y integer, z integer)");
    ASSERT_EQ(stmt.Step(), SQLITE_DONE);
    Sqlite3PreparedStatement insert_stmt(db, "insert into t values(?,?,?)");
    std::tuple value{1, 2.0, std::string{"3"}};

    insert_stmt.Bind(value);
    insert_stmt.Step();
}

TEST(Sqlite3, CanInsertDataFromVarArgs)
{
    Sqlite3DB db(std::nullopt);
    Sqlite3PreparedStatement stmt(db, "create table t(x integer, y integer, z integer)");
    ASSERT_EQ(stmt.Step(), SQLITE_DONE);
    Sqlite3PreparedStatement insert_stmt(db, "insert into t values(?,?,?)");

    insert_stmt.Bind(1, 2.0, std::string{"3"});
    insert_stmt.Step();
}
