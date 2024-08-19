#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

#include <QtWidgets>
#include <QMessageBox>
#include <QTime>
#include <QTimer>

#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QTextBrowser>
#include <QPixmap>

#include <DataToSend.h>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void on_pushButton_Listen_clicked();

    void on_pushButton_Send_clicked();

    void keyPressEvent(QKeyEvent * event);

    void server_New_Connect();
    void readData();
    void socket_Disconnect();
    void onTimerTimeout();

    void on_colorButton_clicked();

    void on_inputEdit_textChanged(const QString &arg1);

    void on_listClients_itemDoubleClicked(QListWidgetItem *item);

    void on_banList_itemDoubleClicked(QListWidgetItem *item);

    void on_sendImage_clicked();

private:
    Ui::Widget *ui;

    QTcpServer *server;

    QList<QTcpSocket *> sockets; //лист с подключенными клиентами

    QString userName = "Server"; //дефолтное имя сервера
    QColor colorSendMessage = Qt::blue; //дефолтный цвет сообщений сервера

    QTimer *timer = new QTimer(this);

    bool send = false;

    QVector <QString> banClients; //лист наказанных
    QVector <QString> connectedUsers; //лист клиентов
    QVector <QString> connectedUsersNames; //лист клиентов
    QList <int> countMessage; //счетчик сообщений от каждого клиента
};

#endif // WIDGET_H
