#pragma once
#include "BuildInfo.h"
#include "Log.h"

#include <QCommandLineParser>
#include <QStringList>
#include <filesystem>
#include <optional>

enum class CLI {
    CommandLineOk,
    CommandLineError,
    CommandLineVersionRequested,
    CommandLineHelpRequested
};

CLI parseCommandLine(
    QCommandLineParser & parser,
    std::optional<std::filesystem::path> & path,
    QString * errorMessage);

std::optional<std::filesystem::path> handleParserArguments();
