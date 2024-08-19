#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>

#include <QtWidgets>
#include <QMessageBox>
#include <QTime>
#include <QNetworkInterface>
#include <QImage>

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
    void on_pushButton_Connect_clicked() ;
    void on_pushButton_Send_clicked() ;

    void keyPressEvent(QKeyEvent * event);

    void readData();
    void socket_disconnect();

    void on_colorButton_clicked();

    void on_inputEdit_textChanged(const QString &arg1);

    void on_sendImage_clicked();

private:
    Ui::Widget *ui;
    QTcpSocket *m_socket;

    QString userName = "Client #"+ QString::number(rand()%1002+1);
    QColor colorSendMessage = Qt::red;

    bool send = false;
    bool isServerConnected = true;

    QList <QString> connectedUsers; //лист клиентов
};

#endif // WIDGET_H
