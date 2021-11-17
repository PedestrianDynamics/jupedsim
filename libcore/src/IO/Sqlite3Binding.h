#pragma once

#include "general/Filesystem.h"
#include "sqlite3.h"

#include <iostream>
#include <optional>
#include <stdexcept>
#include <tuple>

class Sqlite3DB;

class Sqlite3Exception : public std::runtime_error
{
public:
    explicit Sqlite3Exception(const char * msg);
};

class Sqlite3PreparedStatement
{
    sqlite3_stmt * _stmt{nullptr};

public:
    Sqlite3PreparedStatement(const Sqlite3DB & db, const std::string & query);
    ~Sqlite3PreparedStatement();

    Sqlite3PreparedStatement(const Sqlite3PreparedStatement & other) = delete;
    Sqlite3PreparedStatement & operator=(const Sqlite3PreparedStatement & other) = delete;

    Sqlite3PreparedStatement(Sqlite3PreparedStatement && other) = delete;
    Sqlite3PreparedStatement & operator=(Sqlite3PreparedStatement && other) = delete;

    int Step();

    int Reset();

    template <typename... T>
    int Bind(const T &... args)
    {
        return Bind(std::forward_as_tuple(args...));
    }

    template <typename T>
    int Bind(const T & tuple)
    {
        int rc         = SQLITE_OK;
        auto cond_bind = [this, &rc](auto && arg, auto index) {
            if(rc != SQLITE_OK) {
                return;
            }
            rc = this->bind(arg, index);
        };
        std::apply(
            [cond_bind](auto &&... args) {
                int index = 1;
                ((cond_bind(args, index++)), ...);
            },
            tuple);
        return rc;
    }


private:
    int bind(int value, int index)
    {
        std::cout << "type integer " << value << " " << index << std::endl;
        return SQLITE_OK;
    }
    int bind(double value, int index)
    {
        std::cout << "type real " << value << " " << index << std::endl;
        return SQLITE_OK;
    }
    int bind(const std::string & value, int index)
    {
        std::cout << "type text " << value << " " << index << std::endl;
        return SQLITE_OK;
    }
};

class Sqlite3DB
{
    sqlite3 * _db{nullptr};

public:
    explicit Sqlite3DB(const std::optional<fs::path> & opt_path);
    ~Sqlite3DB();

    Sqlite3DB(const Sqlite3DB & other) = delete;
    Sqlite3DB & operator=(const Sqlite3DB & other) = delete;

    Sqlite3DB(Sqlite3DB && other) = delete;
    Sqlite3DB & operator=(Sqlite3DB && other) = delete;

    void Close() noexcept;
    explicit operator sqlite3 *() const noexcept;
};
