#ifndef CLIENT_H
#define CLIENT_H

#include <QWidget>
#include <QDialog>
#include <QTcpSocket>
#include <QtGui>
#include <QtNetwork>
#include <QDebug>

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QLocalSocket;
QT_END_NAMESPACE

#ifdef Q_WS_MAEMO_5
class Client : public QWidget
#else
class Client : public QDialog
#endif
{
    Q_OBJECT

public:
    Client(QWidget *parent = 0);

private slots:
    void enableGetNameButton();
    void getName();

    void readResponse();
    void updateTime();

    void sendResponseAnswerA();
    void sendResponseAnswerB();
    void sendResponseAnswerC();
    void sendResponseAnswerD();

    void displayError(QAbstractSocket::SocketError socketError);

private:
    QGridLayout *mainLayout;
    QLabel *nameLabel;
    QLabel *playerNameLabel;
    QLineEdit *nameLineEdit;
    QPushButton *nameButton;
    QDialogButtonBox *buttonBoxNameEnter;
    QLabel *questionLabel;
    QLabel *answerALabel;
    QLabel *answerBLabel;
    QLabel *answerCLabel;
    QLabel *answerDLabel;
    QPushButton *answerAButton;
    QPushButton *answerBButton;
    QPushButton *answerCButton;
    QPushButton *answerDButton;
    QPushButton *quitButton;
    QLabel *scoreLabel;
    QLabel *playerScoreLabel;
    QLabel *highScoreLabel;
    QLabel *highScoreNameLabel;
    QLabel *playerHighScoreLabel;
    QLabel *timeLabel;
    QLabel *timeLeftLabel;
    QLabel *bufferLabel;
    QTimer *timer;
    int  timeLeft;
    bool timerSet;
    QTcpSocket *socket;
    quint16 blockSize;
};

#endif
