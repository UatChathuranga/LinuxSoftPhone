#include "Agent.h"
#include "AppConfig.h"
#include "CommandServer.h"
#include "MainWindow.h"
#include <QApplication>
#include <string>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setApplicationName("facetonesoftphone");
  app.setOrganizationName("Facetone");

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
