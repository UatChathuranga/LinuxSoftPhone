#include "MainWindow.h"
#include "SettingsWindow.h"
#include <QApplication>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>

MainWindow::MainWindow(Agent *agent, QWidget *parent)
    : QMainWindow(parent), m_agent(agent) {
  setWindowTitle("Facetone Softphone");
  // setFixedSize(280, 600);
  setMinimumSize(300, 600);
  resize(300, 600);
  setMaximumSize(400, 800);
  createUI();

  connect(m_agent, &Agent::registrationStateChangedSignal, this,
          &MainWindow::onRegistrationStateChanged);
  connect(m_agent, &Agent::callStateChangedSignal, this,
          &MainWindow::onCallStateChanged);
  connect(m_agent, &Agent::incomingCallReceivedSignal, this,
          &MainWindow::onIncomingCallReceived);
}

MainWindow::~MainWindow() {}

void MainWindow::createUI() {
  QWidget *centralWidget = new QWidget(this);
  centralWidget->setStyleSheet("background-color: #121212; color: #e0e0e0;");
  QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
  mainLayout->setContentsMargins(15, 15, 15, 15);
  mainLayout->setSpacing(10);

  // 1. Display Area
  QFrame *displayFrame = new QFrame(this);
  displayFrame->setObjectName("displayFrame");
  displayFrame->setStyleSheet(
      "#displayFrame { background-color: #1a1a1a; border: 2px solid #333333; "
      "border-radius: 12px; padding: 10px; }");
  QVBoxLayout *displayLayout = new QVBoxLayout(displayFrame);

  m_display = new QLineEdit(this);
  m_display->setAlignment(Qt::AlignCenter);
  m_display->setReadOnly(true);
  m_display->setFrame(false);
  m_display->setStyleSheet(
      "background: transparent; color: white; font-size: 22pt; font-weight: "
      "bold; font-family: 'Monospace';");
  displayLayout->addWidget(m_display);

  m_statusLabel = new QLabel("...", this);
  m_statusLabel->setAlignment(Qt::AlignCenter);
  m_statusLabel->setStyleSheet(
      "color: white; font-size: 14pt; background-color: transparent;");
  displayLayout->addWidget(m_statusLabel);

  mainLayout->addWidget(displayFrame);

  // 2. Action Buttons (Now at top of dial buttons)
  QHBoxLayout *actionLayout = new QHBoxLayout();
  actionLayout->setSpacing(10);

  QPushButton *callBtn = new QPushButton("📞", this);
  callBtn->setFixedSize(65, 65);
  callBtn->setStyleSheet("background-color: #2e7d32; border-radius: 10px; "
                         "font-size: 20pt; color: white;");
  connect(callBtn, &QPushButton::clicked, this, &MainWindow::onCallClicked);

  QPushButton *declineBtn = new QPushButton("📵", this);
  declineBtn->setFixedSize(65, 65);
  declineBtn->setStyleSheet("background-color: #c62828; border-radius: 10px; "
                            "font-size: 20pt; color: white;");
  connect(declineBtn, &QPushButton::clicked, this,
          &MainWindow::onDeclineClicked);

  QPushButton *clearBtn = new QPushButton("⌫", this);
  clearBtn->setFixedSize(65, 65);
  clearBtn->setStyleSheet("background-color: #424242; border-radius: 10px; "
                          "font-size: 18pt; color: white;");
  connect(clearBtn, &QPushButton::clicked, [this]() { m_display->clear(); });

  actionLayout->addStretch();
  actionLayout->addWidget(callBtn);
  actionLayout->addWidget(declineBtn);
  actionLayout->addWidget(clearBtn);
  actionLayout->addStretch();
  mainLayout->addLayout(actionLayout);

  // 3. Dialpad
  QGridLayout *dialpadLayout = new QGridLayout();
  dialpadLayout->setSpacing(8);
  const char *buttons[] = {"1", "2", "3", "4", "5", "6",
                           "7", "8", "9", "*", "0", "#"};

  for (int i = 0; i < 12; ++i) {
    QPushButton *btn = new QPushButton(buttons[i], this);
    btn->setFixedSize(65, 65);
    btn->setStyleSheet(
        "QPushButton { background-color: #222222; border: 1px solid #444444; "
        "border-radius: 32px; font-size: 18pt; color: white; }"
        "QPushButton:hover { background-color: #333333; }"
        "QPushButton:pressed { background-color: #444444; }");
    dialpadLayout->addWidget(btn, i / 3, i % 3);
    connect(btn, &QPushButton::clicked, this, &MainWindow::onNumberClicked);
  }
  mainLayout->addLayout(dialpadLayout);

  // 4. Bottom Bar
  QHBoxLayout *bottomLayout = new QHBoxLayout();

  QPushButton *settingsBtn = new QPushButton("⚙️", this);
  settingsBtn->setFixedSize(30, 30);
  settingsBtn->setStyleSheet("border-radius: 20px; "
                             "font-size: 14pt; color: white;");
  connect(settingsBtn, &QPushButton::clicked, this,
          &MainWindow::onSettingsClicked);
  bottomLayout->addWidget(settingsBtn);

  bottomLayout->addStretch();
  QLabel *versionLabel = new QLabel("v1.0.1", this);
  versionLabel->setStyleSheet("color: #555555; font-size: 9pt;");
  bottomLayout->addWidget(versionLabel);
  bottomLayout->addStretch();

  // Transparent spacer to balance settings button
  QWidget *spacer = new QWidget(this);
  spacer->setFixedSize(40, 40);
  bottomLayout->addWidget(spacer);

  mainLayout->addLayout(bottomLayout);

  setCentralWidget(centralWidget);
  m_statusBar = new QStatusBar(this);
  setStatusBar(m_statusBar);
}

void MainWindow::onNumberClicked() {
  QPushButton *btn = qobject_cast<QPushButton *>(sender());
  if (btn) {
    m_display->setText(m_display->text() + btn->text());
  }
}

void MainWindow::onCallClicked() {
  std::string dest = m_display->text().toStdString();
  if (!dest.empty()) {
    m_agent->initiateCall(dest);
    m_statusLabel->setText("Calling " + QString::fromStdString(dest) + "...");
  }
}

void MainWindow::onDeclineClicked() {
  m_agent->endCall();
  m_statusLabel->setText("Call ended");
  m_statusBar->showMessage("Call terminated.");
}

void MainWindow::onCallStateChanged(int state, const QString &remoteAddress) {
  QString stateStr;
  QString displayNum = getDisplayNumber(remoteAddress);

  switch (state) {
  case LinphoneCallIdle:
    stateStr = "Idle";
    break;
  case LinphoneCallIncomingReceived:
    stateStr = "Incoming Call";
    m_display->setText(displayNum);
    break;
  case LinphoneCallOutgoingInit:
    stateStr = "Calling...";
    m_display->setText(displayNum);
    break;
  case LinphoneCallOutgoingProgress:
    stateStr = "Ringing...";
    break;
  case LinphoneCallOutgoingRinging:
    stateStr = "Remote Ringing...";
    break;
  case LinphoneCallOutgoingEarlyMedia:
    stateStr = "Early Media";
    break;
  case LinphoneCallConnected:
    stateStr = "Connected";
    break;
  case LinphoneCallStreamsRunning:
    stateStr = "Active";
    break;
  case LinphoneCallPausing:
    stateStr = "Pausing";
    break;
  case LinphoneCallPaused:
    stateStr = "On Hold";
    break;
  case LinphoneCallResuming:
    stateStr = "Resuming";
    break;
  case LinphoneCallRefered:
    stateStr = "Referred";
    break;
  case LinphoneCallError:
    stateStr = "Error";
    break;
  case LinphoneCallEnd:
    stateStr = "Call Ended";
    // Clear display after a short delay
    QTimer::singleShot(2000, this, [this]() {
      if (m_statusLabel->text() == "Call Ended" ||
          m_statusLabel->text() == "Released") {
        m_display->clear();
        m_statusLabel->setText("Ready");
      }
    });
    break;
  case LinphoneCallReleased:
    stateStr = "Released";
    break;
  default:
    stateStr = "Unknown (" + QString::number(state) + ")";
    break;
  }
  m_statusLabel->setText(stateStr);
  m_statusBar->showMessage("Call State: " + stateStr + " (" + remoteAddress +
                           ")");
}

void MainWindow::onIncomingCallReceived(const QString &remoteAddr,
                                        const QString &toAddr,
                                        const QString &callId) {
  m_display->setText(getDisplayNumber(remoteAddr));
  m_statusLabel->setText("Incoming Call");
}

QString MainWindow::getDisplayNumber(const QString &uri) {
  if (uri.isEmpty())
    return "";
  QString name = uri;
  if (name.startsWith("sip:"))
    name = name.mid(4);
  int atPos = name.indexOf('@');
  if (atPos != -1)
    name = name.left(atPos);
  return name;
}

void MainWindow::onRegistrationStateChanged(int state, const QString &message) {
  QString stateStr;
  switch (state) {
  case 0:
    stateStr = "None";
    break;
  case 1:
    stateStr = "Progress";
    break;
  case 2:
    stateStr = "Ok";
    break;
  case 3:
    stateStr = "Cleared";
    break;
  case 4:
    stateStr = "Failed";
    break;
  default:
    stateStr = "Unknown";
    break;
  }
  m_statusLabel->setText("SIP: " + stateStr);
  m_statusBar->showMessage("Registration: " + stateStr + " " + message);
}

void MainWindow::onSettingsClicked() {
  SettingsWindow settings(m_agent, this);
  settings.exec();
}
