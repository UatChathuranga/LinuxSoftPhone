#include "Agent.h"
#include "AppConfig.h"
#include "CommandServer.h"
#include "MainWindow.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <iostream>
#include <QApplication>
#include <string>

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QByteArray localMsg = msg.toLocal8Bit();
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    QString logLine;
    
    switch (type) {
    case QtDebugMsg:
        logLine = QString("[%1] Debug: %2").arg(timestamp, localMsg.constData());
        break;
    case QtInfoMsg:
        logLine = QString("[%1] Info: %2").arg(timestamp, localMsg.constData());
        break;
    case QtWarningMsg:
        logLine = QString("[%1] Warning: %2").arg(timestamp, localMsg.constData());
        break;
    case QtCriticalMsg:
        logLine = QString("[%1] Critical: %2").arg(timestamp, localMsg.constData());
        break;
    case QtFatalMsg:
        logLine = QString("[%1] Fatal: %2").arg(timestamp, localMsg.constData());
        break;
    }

    // Try writing to the system log directory first
    QString logDirPath = "/var/log/facetonesoftphone";
    QString logFilePath = logDirPath + "/facetonesoftphone.log";

    QFile outFile(logFilePath);
    if (outFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream ts(&outFile);
        ts << logLine << Qt::endl;
        outFile.close();
    } else {
        // Fallback to stdout if the log path is not writable (e.g. running without sudo/install)
        std::cout << logLine.toStdString() << std::endl;
    }
}


int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setApplicationName("facetonesoftphone");
  app.setOrganizationName("Facetone");

  // Install custom message handler for logging
  qInstallMessageHandler(customMessageHandler);

  // Initialize Config
  AppConfig &config = AppConfig::instance();

  std::string username = "testuser";
  std::string password = "testingpassword";
  std::string domain = "sip.example.com";

  Agent agent(username, password, domain);

  CommandServer server(config.webSocketPort, &agent);

  MainWindow win(&agent);
  win.show();

  return app.exec();
}
