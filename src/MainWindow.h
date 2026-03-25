#pragma once

#include <QMainWindow>
#include <QString>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QStatusBar>
#include "Agent.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(Agent* agent, QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onNumberClicked();
    void onCallClicked();
    void onDeclineClicked();
    void onRegistrationStateChanged(int state, const QString& message);
    void onCallStateChanged(int state, const QString& remoteAddress);
    void onIncomingCallReceived(const QString& remoteAddr, const QString& toAddr, const QString& callId);
    void onSettingsClicked();

private:
    Agent* m_agent;
    QLineEdit* m_display;
    QLabel* m_statusLabel;
    QStatusBar* m_statusBar;
    
    void createUI();
    QString getDisplayNumber(const QString& uri);
};
