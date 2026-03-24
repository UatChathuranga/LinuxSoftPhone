#include "MainWindow.h"
#include <QApplication>
#include <QFont>

MainWindow::MainWindow(Agent* agent, QWidget* parent)
    : QMainWindow(parent), m_agent(agent) {
    setWindowTitle("Linux Softphone");
    setMinimumSize(300, 450);
    createUI();

    connect(m_agent, &Agent::registrationStateChangedSignal, this, &MainWindow::onRegistrationStateChanged);
}

MainWindow::~MainWindow() {}

void MainWindow::createUI() {
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    m_display = new QLineEdit(this);
    m_display->setAlignment(Qt::AlignCenter);
    m_display->setReadOnly(true);
    QFont font = m_display->font();
    font.setPointSize(24);
    m_display->setFont(font);
    mainLayout->addWidget(m_display);

    QGridLayout* dialpadLayout = new QGridLayout();
    const char* buttons[] = {
        "1", "2", "3",
        "4", "5", "6",
        "7", "8", "9",
        "*", "0", "#"
    };

    for (int i = 0; i < 12; ++i) {
        QPushButton* btn = new QPushButton(buttons[i], this);
        btn->setFixedSize(60, 60);
        btn->setFont(font);
        dialpadLayout->addWidget(btn, i / 3, i % 3);
        connect(btn, &QPushButton::clicked, this, &MainWindow::onNumberClicked);
    }
    mainLayout->addLayout(dialpadLayout);

    QPushButton* callBtn = new QPushButton("CALL", this);
    callBtn->setMinimumHeight(60);
    callBtn->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold; font-size: 18px;");
    mainLayout->addWidget(callBtn);
    connect(callBtn, &QPushButton::clicked, this, &MainWindow::onCallClicked);

    QPushButton* clearBtn = new QPushButton("Clear", this);
    mainLayout->addWidget(clearBtn);
    connect(clearBtn, &QPushButton::clicked, [this](){ m_display->clear(); });

    setCentralWidget(centralWidget);
    m_statusBar = new QStatusBar(this);
    setStatusBar(m_statusBar);
    m_statusBar->showMessage("Initializing...");
}

void MainWindow::onNumberClicked() {
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (btn) {
        m_display->setText(m_display->text() + btn->text());
    }
}

void MainWindow::onCallClicked() {
    std::string dest = m_display->text().toStdString();
    if (!dest.empty()) {
        m_agent->initiateCall(dest);
        m_statusBar->showMessage("Calling " + QString::fromStdString(dest) + "...");
    }
}

void MainWindow::onRegistrationStateChanged(int state, const QString& message) {
    QString stateStr;
    switch(state) {
        case 0: stateStr = "None"; break;
        case 1: stateStr = "Progress"; break;
        case 2: stateStr = "Ok"; break;
        case 3: stateStr = "Cleared"; break;
        case 4: stateStr = "Failed"; break;
        default: stateStr = "Unknown"; break;
    }
    m_statusBar->showMessage("Registration: " + stateStr + " " + message);
}
