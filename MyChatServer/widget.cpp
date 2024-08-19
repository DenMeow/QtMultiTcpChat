#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget) //конструктор
{
    ui->setupUi(this);
    setWindowTitle("Server");
    ui -> inputEdit -> setPlaceholderText("Введите сообщение...");
    ui -> nameEdit -> setPlaceholderText("Server");
    ui -> colorButton ->setStyleSheet("color: " + colorSendMessage.name() + ";");
    ui->pushButton_Send->setEnabled(false);
    ui -> sendImage -> setEnabled(false);

    server = new QTcpServer();
    server -> setMaxPendingConnections(1);
    connect(server, &QTcpServer::newConnection, this, &Widget::server_New_Connect);

    timer -> start(3000);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimerTimeout()));
}


void Widget::keyPressEvent(QKeyEvent *event) //обработка нажатия return
{
    if ((event -> key() == Qt::Key_Return) and ui -> inputEdit -> text().length() > 0 and !sockets.isEmpty() and !ui -> inputEdit -> text().trimmed().isEmpty())
        on_pushButton_Send_clicked();
}

void Widget::on_colorButton_clicked() //выбор цвета
{
    colorSendMessage = QColorDialog::getColor(QColor(Qt::white),this,"Выберите цвет сообщений");
    ui -> colorButton ->setStyleSheet("color: " + colorSendMessage.name() + ";");
}


Widget::~Widget() //деструктор
{
    if(!sockets.isEmpty()) {
        QString text = userName + " отключился";

        // Отправляем обычное сообщение
        DataToSend m_data;
        m_data.color = Qt::black;
        m_data.message = text;
        m_data.dateTime = QDateTime::currentDateTime();
        m_data.name = userName;

        QByteArray buffer_array;
        QDataStream dataStream(&buffer_array, QIODevice::WriteOnly);
        dataStream << m_data; // Сериализация данных

        for (QTcpSocket *client : sockets) {
            client -> write(buffer_array);
            client -> flush();
            client -> disconnectFromHost();
        }

        server -> close();
        server -> deleteLater();
    } else {
        server -> close();
        server -> deleteLater();
    }
    delete ui;
}

void Widget::on_pushButton_Listen_clicked() //кнопка подключения / отключения сервера
{
    if (ui->pushButton_Listen->text() == "Connect")
    {
        QHostAddress ip;
        if(!ip.setAddress(ui -> lineEdit_IP -> text())) {
            QMessageBox::warning(this, "Ошибка!", "Вы ввели некорректный IP адрес");
            return;
        }
        ushort port = ui->portServer->value();

        if(!server -> listen(QHostAddress::Any, port))
        {
            QMessageBox::critical(this,"Ошибка",server->errorString());
            return;
        }

        ui->pushButton_Listen->setText("Disconnect");
        ui -> textBrowser -> clear();
        ui -> textBrowser -> append("Вы включили сервер, теперь к вам может подключиться клиент!");

        ui -> groupBox -> setEnabled(false);
        if (ui -> inputEdit -> text().length() > 0 and !sockets.isEmpty()) {
            ui -> pushButton_Send -> setEnabled(true);
        }
        if (!ui -> nameEdit -> text().isEmpty())
            userName = ui -> nameEdit -> text();
    }
    else
    {
        if(!sockets.isEmpty()) {
            QString text = userName + " закрылся";

            // Отправляем обычное сообщение
            DataToSend m_data;
            m_data.color = Qt::black;
            m_data.message = text;
            m_data.dateTime = QDateTime::currentDateTime();
            m_data.name = userName;

            QByteArray buffer_array;
            QDataStream dataStream(&buffer_array, QIODevice::WriteOnly);
            dataStream << m_data; // Сериализация данных

            for (QTcpSocket *client : sockets) {
                client -> write(buffer_array);
                client -> flush();
                client -> disconnectFromHost();
            }

            server -> close();
        } else {
            server -> close();
        }

        ui -> textBrowser -> append("Вы отключили сервер!");

        ui -> pushButton_Listen -> setText("Connect");

        ui -> pushButton_Send -> setEnabled(false);
        ui -> sendImage -> setEnabled(false);
        ui -> groupBox -> setEnabled(true);
    }
}

void Widget::on_pushButton_Send_clicked() //отправка сообщения
{
    QString text = userName + ": " + ui-> inputEdit -> text().toHtmlEscaped();

    // Отправляем обычное сообщение
    DataToSend m_data;
    m_data.color = colorSendMessage.name();
    m_data.message = text;
    m_data.dateTime = QDateTime::currentDateTime();
    m_data.name = userName;

    QByteArray buffer_array;
    QDataStream dataStream(&buffer_array, QIODevice::WriteOnly);
    dataStream << m_data; // Сериализация данных

    for (QTcpSocket *client : sockets) {
        client -> write(buffer_array);
        client -> flush();
    }

    QString messageText = text; // Текст сообщения
    QColor messageColor = colorSendMessage; // Цвет сообщения
    QString formattedMessage = QString("<span style='color:%1;'>[%2] %3</span>")
                                   .arg(messageColor.name())
                                   .arg(m_data.dateTime.toString("hh:mm:ss"))
                                   .arg(messageText);

    ui -> textBrowser -> append(formattedMessage);

    ui -> inputEdit -> clear();
}

void Widget::server_New_Connect() //подключение клиента к серверу
{
    QTcpSocket *new_socket;
    new_socket = server -> nextPendingConnection();

    //проверка находится ли новый сокет в бане
    if (banClients.indexOf(new_socket -> peerAddress().toString()) != -1) {
        new_socket -> disconnectFromHost();
        new_socket -> close();
        return;
    }

    sockets.append(new_socket);
    countMessage.push_back(0); //заводим счетчик на клиента

    //добавили нового пользователя в list и вектор подключенных пользователей
    ui->listClients -> addItem(new_socket -> peerAddress().toString());
    connectedUsers.push_back(new_socket -> peerAddress().toString());

    QString text = "Вы подключились к серверу: " + userName;

    // Отправляем обычное сообщение
    DataToSend m_data;
    m_data.color = Qt::black;
    m_data.message = text;
    m_data.dateTime = QDateTime::currentDateTime();
    m_data.name = userName;
    m_data.nameUsers = connectedUsersNames;

    QByteArray buffer_array;
    QDataStream dataStream(&buffer_array, QIODevice::WriteOnly);
    dataStream << m_data; // Сериализация данных

    new_socket -> write(buffer_array);

    connect(new_socket, &QTcpSocket::readyRead, this, &Widget::readData);
    connect(new_socket, &QTcpSocket::disconnected, this, &Widget::socket_Disconnect);

    if (ui -> inputEdit -> text().length() > 0) {
        ui -> pushButton_Send -> setEnabled(true);
    }
    ui -> sendImage -> setEnabled(true);
}

void Widget::socket_Disconnect() //сигнал отключения
{
    QTcpSocket * close_c = static_cast<QTcpSocket*>(QObject::sender()); // так можно найти виновника отключения
    sockets.removeOne(close_c);

    ui -> listClients -> takeItem(connectedUsers.indexOf(close_c->peerAddress().toString()));
    connectedUsers.remove(connectedUsers.indexOf(close_c -> peerAddress().toString()));

    // Отправляем обычное сообщение
    DataToSend m_data;
    m_data.color = Qt::black;
    m_data.message = "";
    m_data.dateTime = QDateTime::currentDateTime();
    m_data.name = userName;
    //m_data.nameUsers = connectedUsersNames;

    QByteArray buffer_array;
    QDataStream dataStream(&buffer_array, QIODevice::WriteOnly);
    dataStream << m_data; // Сериализация данных

    for (QTcpSocket *client : sockets) {
        client -> write(buffer_array);
        client -> flush();
    }

    ui -> pushButton_Send -> setEnabled(false);
    if (sockets.isEmpty())
        ui -> sendImage -> setEnabled(false);
}

void Widget::readData() //чтение данных
{
    QTcpSocket *new_socket;
    new_socket = (QTcpSocket*)sender(); //отслеживание того, что подал сигнал
    QDataStream in(new_socket);

    DataToSend recieveData;

    if(in.status() == QDataStream::Ok)
    {
        in.startTransaction();
        in >> recieveData;
        if(!in.commitTransaction())
        {
            //qDebug() << "сообщение не дошло";
            return;
        }


        QByteArray buffer_array;
        QDataStream dataStream(&buffer_array, QIODevice::WriteOnly);
        if (connectedUsersNames.indexOf(recieveData.name) == -1)
            connectedUsersNames.push_back(recieveData.name);
        recieveData.nameUsers = connectedUsersNames;
        dataStream << recieveData;

        int i = 0;
        for (QTcpSocket *client : sockets) {
            if (client == new_socket) {
                countMessage[i]++;
                ++i;
                // continue;
            }
            client -> write(buffer_array);
            client -> flush();
            ++i;
        }

        QString messageText = recieveData.message;
        if (messageText.size() > 2) {
            QColor messageColor = recieveData.color;

            if (!recieveData.image.isNull()) { // сообщение с фотографией
                QImage image = recieveData.image;
                // Преобразование изображения в байтовый массив
                QByteArray byteArray;
                QBuffer buffer(&byteArray);
                buffer.open(QIODevice::WriteOnly);
                image.save(&buffer, "PNG");  // Сохранение изображения в формате PNG
                buffer.close();

                // Преобразование байтового массива в base64 строку
                QString base64Image = QString::fromLatin1(byteArray.toBase64().data());

                // Формируем HTML-тег для вставки изображения
                QString newText = "<img src='data:image/png;base64," + base64Image + "' width='180' height='100' />";
                QString formattedMessage = QString("<span style='color:%1;'>[%2] %3</span>")
                                               .arg(messageColor.name())
                                               .arg(recieveData.dateTime.toString("hh:mm:ss"))
                                               .arg(messageText + ": ");

                ui -> textBrowser -> append(formattedMessage + newText);
            } else { //обычное сообщение
                QString formattedMessage = QString("<span style='color:%1;'>[%2] %3</span>")
                                               .arg(messageColor.name())
                                               .arg(recieveData.dateTime.toString("hh:mm:ss"))
                                               .arg(messageText);

                ui -> textBrowser -> append(formattedMessage);
                if (messageText.contains("ушел :( !", Qt::CaseInsensitive))
                    connectedUsersNames.remove(connectedUsersNames.indexOf(recieveData.name));
            }
        }
    }
}

void Widget::onTimerTimeout() //проверка спама каждые 3 секунды
{
    if (!sockets.isEmpty()){
        for (int i = 0; i < countMessage.size(); ++i) {
            if (countMessage[i] > 4) {
                QTcpSocket *socket = sockets[i];
                QString text = "Кто спамит тот плохой";

                // Отправляем обычное сообщение
                DataToSend m_data;
                m_data.color = Qt::black;
                m_data.message = text;
                m_data.dateTime = QDateTime::currentDateTime();
                m_data.name = userName;

                QByteArray buffer_array;
                QDataStream dataStream(&buffer_array, QIODevice::WriteOnly);
                dataStream << m_data; // Сериализация данных

                socket -> write(buffer_array);

                ui -> banList -> addItem(socket -> peerAddress().toString());
                banClients.push_back(socket -> peerAddress().toString());

                sockets.removeOne(socket);
                countMessage.removeAt(i);

                socket -> disconnectFromHost();
            }
        }
        for (int i = 0; i < countMessage.size(); ++i) {
            countMessage[i] = 0;
        }
    }
}

void Widget::on_inputEdit_textChanged(const QString &arg1) //изменение отправляемого сообщения
{
    if (arg1.trimmed().isEmpty() or ui -> pushButton_Listen -> text() == "Connect" or sockets.isEmpty()) {
        ui -> pushButton_Send -> setEnabled(false);
        send = false;
    }
    else {
        ui -> pushButton_Send -> setEnabled(true);
        send = true;
    }
}

void Widget::on_listClients_itemDoubleClicked(QListWidgetItem *item) //бан выбранного пользователя
{
    ui -> banList -> addItem(item -> text());
    banClients.push_back((item -> text()));

    for (QTcpSocket *client : sockets) {
        if (client -> peerAddress().toString() == item -> text()) {
            QString text = "Вас забанил админ";

            // Отправляем обычное сообщение
            DataToSend m_data;
            m_data.color = Qt::black;
            m_data.message = text;
            m_data.dateTime = QDateTime::currentDateTime();
            m_data.name = userName;

            QByteArray buffer_array;
            QDataStream dataStream(&buffer_array, QIODevice::WriteOnly);
            dataStream << m_data; // Сериализация данных

            client -> write(buffer_array);

            client -> disconnectFromHost();
            client -> close();
            sockets.removeOne(client);
            break;
        }
    }
}

void Widget::on_banList_itemDoubleClicked(QListWidgetItem *item) //разбан выбранного пользователя
{
    ui -> banList -> takeItem(banClients.indexOf(item -> text()));
    banClients.remove(banClients.indexOf(item -> text()));
}


void Widget::on_sendImage_clicked() //отправление сообщения
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Выберите изображение"), "", tr("Изображения (*.png *.jpg *.jpeg)"));

    // Проверка, был ли выбран файл
    if (!filePath.isEmpty()) {
        QFileInfo fileInfo(filePath);
        qint64 fileSize = fileInfo.size() / (1024 * 1024);
        qDebug() << "Размер файла: " << fileSize << "Мб";
        if (fileSize >= 10) {
            QMessageBox::information(this,"Ошибка","Большая фотка");
            return;
        }

        QImage image(filePath);

        DataToSend m_data;
        m_data.color = colorSendMessage.name();
        m_data.message = userName;
        m_data.dateTime = QDateTime::currentDateTime();
        m_data.image = image;

        QByteArray buffer_array;
        QDataStream dataStream(&buffer_array, QIODevice::WriteOnly);
        dataStream << m_data; // Сериализация данных

        for (QTcpSocket *client : sockets) {
            client -> write(buffer_array);
            //client -> flush();
        }

        QString html = "<img src='file://" + filePath + "' width='180' height='100' />";

        // Вставляем HTML-тег с изображением в QTextBrowser
        QString formattedMessage = QString("<span style='color:%1;'>[%2] %3</span>")
                                       .arg(colorSendMessage.name())
                                       .arg(m_data.dateTime.toString("hh:mm:ss"))
                                       .arg(userName + ": " + html);

        ui -> textBrowser -> append(formattedMessage);
    }
}


