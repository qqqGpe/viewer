#include <QApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QDebug>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Set application metadata
    app.setApplicationName("Log Viewer");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("LogViewer");

    // Create main window
    Viewer::MainWindow window;
    window.show();

    // Parse command line arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("Log Viewer - A tool for visualizing log data as curves");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "Log file to open (CSV or binary)", "[file]");
    parser.process(app);

    // Open file if provided
    QStringList args = parser.positionalArguments();
    if (!args.isEmpty()) {
        QString filePath = args.first();
        window.loadFile(filePath);
    }

    return app.exec();
}
