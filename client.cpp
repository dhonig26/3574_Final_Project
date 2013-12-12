#include "client.h"

Client::Client(QWidget *parent)
#ifdef Q_WS_MAEMO_5
    : QWidget(parent)
#else
    : QDialog(parent)
#endif
{
    nameLabel = new QLabel(tr("&Player name:"));
    nameLineEdit = new QLineEdit("Enter your name");
    nameLabel->setBuddy(nameLineEdit);
    nameButton = new QPushButton(tr("Enter"));
    nameButton->setDefault(true);
    nameButton->setEnabled(false);
    buttonBoxNameEnter = new QDialogButtonBox;
    buttonBoxNameEnter->addButton(nameButton, QDialogButtonBox::ActionRole);

    questionLabel = new QLabel(tr("To begin enter your name above and press enter. The game might already be in play so wait patiently until the next round. Have Fun!"));
    questionLabel->setWordWrap(true);

    scoreLabel           = new QLabel(tr("Score: "));
    playerScoreLabel     = new QLabel(tr(""));
    highScoreLabel       = new QLabel(tr("High Score: "));
    highScoreNameLabel   = new QLabel(tr(""));
    playerHighScoreLabel = new QLabel(tr(""));
    timeLabel            = new QLabel(tr("Time Left: "));
    timeLeftLabel        = new QLabel(tr(""));

    bufferLabel = new QLabel(tr(""));

    answerALabel = new QLabel(tr(""));
    answerALabel->setWordWrap(true);
    answerBLabel = new QLabel(tr(""));
    answerBLabel->setWordWrap(true);
    answerCLabel = new QLabel(tr(""));
    answerCLabel->setWordWrap(true);
    answerDLabel = new QLabel(tr(""));
    answerDLabel->setWordWrap(true);

    answerAButton = new QPushButton(tr("A"));
    answerAButton->setEnabled(false);
    answerBButton = new QPushButton(tr("B"));
    answerBButton->setEnabled(false);
    answerCButton = new QPushButton(tr("C"));
    answerCButton->setEnabled(false);
    answerDButton = new QPushButton(tr("D"));
    answerDButton->setEnabled(false);
    quitButton    = new QPushButton(tr("Quit"));

    socket = new QTcpSocket(this);

    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(nameLineEdit, SIGNAL(textChanged(QString)), this, SLOT(enableGetNameButton()));
    connect(nameButton,   SIGNAL(clicked()),            this, SLOT(getName()));

    connect(answerAButton, SIGNAL(clicked()), this, SLOT(sendResponseAnswerA()));
    connect(answerBButton, SIGNAL(clicked()), this, SLOT(sendResponseAnswerB()));
    connect(answerCButton, SIGNAL(clicked()), this, SLOT(sendResponseAnswerC()));
    connect(answerDButton, SIGNAL(clicked()), this, SLOT(sendResponseAnswerD()));

    connect(socket, SIGNAL(readyRead()), this, SLOT(readResponse()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(disconnected()), this, SLOT(close()));

    mainLayout = new QGridLayout;
    mainLayout->addWidget(nameLabel,    0, 0, Qt::AlignLeft);
    mainLayout->addWidget(nameLineEdit, 0, 1, Qt::AlignLeft);
    mainLayout->addWidget(nameButton,   0, 2, 1, 1, Qt::AlignLeft);
    mainLayout->addWidget(scoreLabel,           1, 0, 1, 1, Qt::AlignLeft);
    mainLayout->addWidget(playerScoreLabel,     1, 1, 1, 2, Qt::AlignLeft);
    mainLayout->addWidget(highScoreLabel,       2, 0, 1, 1, Qt::AlignLeft);
    mainLayout->addWidget(highScoreNameLabel,   2, 2, 1, 1, Qt::AlignLeft);
    mainLayout->addWidget(playerHighScoreLabel, 2, 3, 1, 1, Qt::AlignLeft);
    mainLayout->addWidget(timeLabel,     3,  0, 1, 1, Qt::AlignLeft);
    mainLayout->addWidget(timeLeftLabel, 3,  1, 1, 2, Qt::AlignLeft);
    mainLayout->addWidget(questionLabel, 4,  0, 3, 3, Qt::AlignLeft);
    mainLayout->addWidget(bufferLabel,   7,  0, 1, 3, Qt::AlignLeft);
    mainLayout->addWidget(answerALabel,  8,  1, Qt::AlignLeft);
    mainLayout->addWidget(answerBLabel,  9,  1, Qt::AlignLeft);
    mainLayout->addWidget(answerCLabel,  10, 1, Qt::AlignLeft);
    mainLayout->addWidget(answerDLabel,  11, 1, Qt::AlignLeft);
    mainLayout->addWidget(answerAButton, 8,  0, 1, 1, Qt::AlignLeft);
    mainLayout->addWidget(answerBButton, 9,  0, 1, 1, Qt::AlignLeft);
    mainLayout->addWidget(answerCButton, 10, 0, 1, 1, Qt::AlignLeft);
    mainLayout->addWidget(answerDButton, 11, 0, 1, 1, Qt::AlignLeft);
    mainLayout->addWidget(quitButton,    12, 0, 1, 1, Qt::AlignLeft);

    setLayout(mainLayout);

    setWindowTitle(tr("Trivia Game Client"));
    nameLineEdit->setFocus();
}

void Client::enableGetNameButton()
{
    nameButton->setEnabled(!nameLineEdit->text().isEmpty());
}

void Client::getName()
{
    socket->connectToHost(QHostAddress("127.0.0.1"), 3574);

    QString name;
    name = nameLineEdit->text();

    blockSize = 0;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    out << (quint16)0;
    out << name;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));

    socket->write(block);

    nameLineEdit->hide();
    nameButton->hide();

    playerNameLabel = new QLabel(name);
    playerNameLabel->setWordWrap(true);

    mainLayout->addWidget(playerNameLabel, 0, 1, 1, 2, Qt::AlignLeft);

    questionLabel->setText("Play will begin shortly");
}

void Client::readResponse()
{
    qDebug() << "readResponse";

    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_4_0);

    if (blockSize == 0)
    {
        if (socket->bytesAvailable() < (int)sizeof(quint16))
            return;
        in >> blockSize;
    }

    if (in.atEnd())
        return;

    QString response;
    in >> response;

    if(!timerSet)
    {
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(updateTime()));
        timer->start(1000);
        timerSet = true;
    }

    if(response.startsWith("Question:"))
    {
        questionLabel->setText(response.replace("Question:", "", Qt::CaseSensitive));
        timeLeft = 30;
    }
    else if(response.startsWith("AnswerA:"))
        answerALabel->setText(response.replace("AnswerA:", "", Qt::CaseSensitive));
    else if(response.startsWith("AnswerB:"))
        answerBLabel->setText(response.replace("AnswerB:", "", Qt::CaseSensitive));
    else if(response.startsWith("AnswerC:"))
        answerCLabel->setText(response.replace("AnswerC:", "", Qt::CaseSensitive));
    else if(response.startsWith("AnswerD:"))
        answerDLabel->setText(response.replace("AnswerD:", "", Qt::CaseSensitive));
    else if(response.startsWith("PlayerScore:"))
        playerScoreLabel->setText(response.replace("PlayerScore:", "", Qt::CaseSensitive));
    else if(response.startsWith("HighScorePlayer:"))
        highScoreNameLabel->setText(response.replace("HighScorePlayer:", "", Qt::CaseSensitive));
    else if(response.startsWith("HighScore:"))
        playerHighScoreLabel->setText(response.replace("HighScore:", "", Qt::CaseSensitive));

    answerAButton->setEnabled(true);
    answerBButton->setEnabled(true);
    answerCButton->setEnabled(true);
    answerDButton->setEnabled(true);

    blockSize = 0;
}

void Client::sendResponseAnswerA()
{
    blockSize = 0;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    out << (quint16)0;
    out << "A";
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));

    socket->write(block);

    answerAButton->setEnabled(false);
    answerBButton->setEnabled(false);
    answerCButton->setEnabled(false);
    answerDButton->setEnabled(false);
}

void Client::sendResponseAnswerB()
{
    blockSize = 0;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    out << (quint16)0;
    out << "B";
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));

    socket->write(block);

    answerAButton->setEnabled(false);
    answerBButton->setEnabled(false);
    answerCButton->setEnabled(false);
    answerDButton->setEnabled(false);
}

void Client::sendResponseAnswerC()
{
    blockSize = 0;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    out << (quint16)0;
    out << "C";
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));

    socket->write(block);

    answerAButton->setEnabled(false);
    answerBButton->setEnabled(false);
    answerCButton->setEnabled(false);
    answerDButton->setEnabled(false);
}

void Client::sendResponseAnswerD()
{
    blockSize = 0;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    out << (quint16)0;
    out << "D";
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));

    socket->write(block);

    answerAButton->setEnabled(false);
    answerBButton->setEnabled(false);
    answerCButton->setEnabled(false);
    answerDButton->setEnabled(false);
}

void Client::updateTime()
{
    timeLeft = timeLeft - 1;
    timeLeftLabel->setText(QString::number(timeLeft));
    blockSize = 0;
}

void Client::displayError(QTcpSocket::SocketError socketError)
{
    switch (socketError) {
    case QLocalSocket::ServerNotFoundError:
        QMessageBox::information(this, tr("Gate Client"),
                                 tr("The host was not found. Please check the "
                                    "host name and port settings."));
        break;
    case QLocalSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("Gate Client"),
                                 tr("The connection was refused by the peer. "
                                    "Make sure the gate server is running, "
                                    "and check that the host name and port "
                                    "settings are correct."));
        break;
    case QLocalSocket::PeerClosedError:
        break;
    default:
        QMessageBox::information(this, tr("Gate Client"),
                                 tr("The following error occurred: %1.")
                                 .arg(socket->errorString()));
    }

    answerAButton->setEnabled(true);
}

