#include "CLI.hpp"

#include <iostream>
CLI parseCommandLine(
    QCommandLineParser & parser,
    std::optional<std::filesystem::path> & path,
    QString * errorMessage)
{
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    parser.addPositionalArgument("Trajectory", "trajfile");
    const QCommandLineOption helpOption    = parser.addHelpOption();
    const QCommandLineOption versionOption = parser.addVersionOption();
    if(!parser.parse(QCoreApplication::arguments())) {
        *errorMessage = parser.errorText() + ". Try: 'jpsvis --help'";
        return CLI::CommandLineError;
    }
    if(parser.isSet(versionOption))
        return CLI::CommandLineVersionRequested;

    if(parser.isSet(helpOption))
        return CLI::CommandLineHelpRequested;

    const QStringList positionalArguments = parser.positionalArguments();
    if(positionalArguments.size() > 1) {
        *errorMessage = "Several trajectory files specified.  Try: 'jpsvis --help'";
        return CLI::CommandLineError;
    }
    if(!positionalArguments.isEmpty()) {
        path = positionalArguments[0].toStdString();
    }

    return CLI::CommandLineOk;
}

std::optional<std::filesystem::path> handleParserArguments()
{
    QString errorMessage;
    QCommandLineParser parser;
    std::optional<std::filesystem::path> path = {};
    switch(parseCommandLine(parser, path, &errorMessage)) {
        case CLI::CommandLineOk:
            break;
        case CLI::CommandLineError:
            Log::Error(errorMessage.toStdString().c_str());
            std::exit(0);
        case CLI::CommandLineVersionRequested:
            Log::Info(
                "%s: %s",
                qUtf8Printable(QCoreApplication::applicationName()),
                qUtf8Printable(QCoreApplication::applicationVersion()));
            Log::Info("Current date     : %s %s", __DATE__, __TIME__);
            Log::Info("Compiler         : %s (%s)", COMPILER.c_str(), COMPILER_VERSION.c_str());
            Log::Info("Commit hash      : %s", GIT_COMMIT_HASH.c_str());
            Log::Info("Commit date      : %s", GIT_COMMIT_DATE.c_str());
            Log::Info("Branch           : %s", GIT_BRANCH.c_str());
            std::exit(0);
        case CLI::CommandLineHelpRequested:
            parser.showHelp();
            Q_UNREACHABLE();
    }
    return path;
}
