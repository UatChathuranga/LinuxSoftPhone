#pragma once

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
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
    void onRegistrationStateChanged(int state, const QString& message);

private:
    Agent* m_agent;
    QLineEdit* m_display;
    QStatusBar* m_statusBar;
    
    void createUI();
};
