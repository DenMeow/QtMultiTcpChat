#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget) //конструктор
{
    ui->setupUi(this);
    setWindowTitle("Client");
    ui -> nameEdit -> setPlaceholderText("Client#___");
    ui -> inputEdit -> setPlaceholderText("Введите сообщение...");
    ui -> pushButton_Send -> setEnabled(false);
    ui -> sendImage -> setEnabled(false);
    ui -> listClients -> setEnabled(true);

    m_socket = new QTcpSocket();

    connect(m_socket,&QTcpSocket::readyRead,this,&Widget::readData);
    connect(m_socket,&QTcpSocket::disconnected,this,&Widget::socket_disconnect);

    ui -> colorButton ->setStyleSheet("color: " + colorSendMessage.name() + ";");
}

Widget::~Widget() //деструктор
{
    if (isServerConnected){
        // Отобразите прощальное сообщение в чате
        QString welcomeMessage = "Пользователь " + userName + "  ушел :( !";
        DataToSend welcomeData;
        welcomeData.dateTime = QDateTime::currentDateTime();
        welcomeData.message = welcomeMessage;
        welcomeData.name = userName;

        QByteArray buffer_array;
        QDataStream dataStream(&buffer_array, QIODevice::WriteOnly);
        dataStream << welcomeData; // Сериализация данных

        m_socket -> write(buffer_array);
        m_socket -> disconnectFromHost();

        m_socket->deleteLater();
    }
    else {
        m_socket->deleteLater();
    }
    delete ui;
}

void Widget::keyPressEvent(QKeyEvent *event) //обработка нажатия return на клавиатуре
{
    if ((event -> key() == Qt::Key_Return) and send)
        on_pushButton_Send_clicked();
}


void Widget::on_pushButton_Connect_clicked() //кнопка подключения / отключения
{
    QHostAddress IP;
    ushort port;

    if (ui->pushButton_Connect->text() == "Connect")
    {
        if (!IP.setAddress(ui->lineEdit_IP->text())) {
            QMessageBox::warning(this, "Ошибка!", "Вы ввели некорректный IP адрес");
            return;
        }
        port = ui->portServer->value();

        m_socket -> abort();
        m_socket -> connectToHost(IP,port);
        isServerConnected = true;
        if (!m_socket -> waitForConnected(3000) /*or !isServerConnected*/)
        {
            QMessageBox::information(this,"Ошибка","Сервер не слушает");
            isServerConnected = false;
            return;
        }

        ui -> textBrowser -> clear();

        //считываем имя или даем случайное
        if(!ui -> nameEdit -> text().isEmpty())
            userName = ui -> nameEdit -> text();
        else
            userName = "Client #"+ QString::number(rand()%1002+1);

        // Отобразите приветственное сообщение в чате
        QString welcomeMessage = "Пользователь " + userName + "  подключился к серверу!";
        DataToSend welcomeData;
        welcomeData.dateTime = QDateTime::currentDateTime();
        welcomeData.message = welcomeMessage;
        welcomeData.name = userName;

        QByteArray buffer_array;
        QDataStream dataStream(&buffer_array, QIODevice::WriteOnly);
        dataStream << welcomeData; // Сериализация данных

        m_socket -> write(buffer_array);

        ui -> groupBox -> setEnabled(false);
        if (ui -> inputEdit -> text().length() > 0) {
            ui -> pushButton_Send -> setEnabled(true);
        }

        ui -> pushButton_Connect -> setText("Disconnect");
        ui -> sendImage -> setEnabled(true);
    }
    else
    {
        // Отобразите прощальное сообщение в чате
        QString welcomeMessage = "Пользователь " + userName + "  ушел :( !";
        DataToSend welcomeData;
        welcomeData.dateTime = QDateTime::currentDateTime();
        welcomeData.message = welcomeMessage;
        welcomeData.name = userName;

        QByteArray buffer_array;
        QDataStream dataStream(&buffer_array, QIODevice::WriteOnly);
        dataStream << welcomeData; // Сериализация данных

        m_socket -> write(buffer_array);
        m_socket -> disconnectFromHost();

        ui -> textBrowser -> append("Вы отключились от чата!");

        ui -> pushButton_Connect -> setText("Connect");
        ui -> pushButton_Send -> setEnabled(false);
        ui -> sendImage -> setEnabled(false);
        ui -> groupBox -> setEnabled(true);
    }
}

void Widget::on_pushButton_Send_clicked()  //отправление сообщения
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

    m_socket -> write(buffer_array);
    m_socket -> flush();

    // QString messageText = text; // Текст сообщения
    // QColor messageColor = colorSendMessage; // Цвет сообщения
    // QString formattedMessage = QString("<span style='color:%1;'>[%2] %3</span>")
    //                                .arg(messageColor.name())
    //                                .arg(m_data.dateTime.toString("hh:mm:ss"))
    //                                .arg(messageText);

    // ui -> textBrowser -> append(formattedMessage);

    ui -> inputEdit -> clear();
}

void Widget::on_sendImage_clicked() //кнопка обработки отправления фотографии
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

        m_socket -> write(buffer_array);

        QString html = "<img src='file://" + filePath + "' width='180' height='100' />";

        // Вставляем HTML-тег с изображением в QTextBrowser
        QString formattedMessage = QString("<span style='color:%1;'>[%2] %3</span>")
                                       .arg(colorSendMessage.name())
                                       .arg(m_data.dateTime.toString("hh:mm:ss"))
                                       .arg(userName + ": " + html);

        ui -> textBrowser -> append(formattedMessage);
    }
}


void Widget::readData() //чтение данных
{
    m_socket = (QTcpSocket*)sender();
    QDataStream in(m_socket);

    DataToSend recieveData;

    if(in.status() == QDataStream::Ok)
    {
        in.startTransaction();
        in >> recieveData;
        if(!in.commitTransaction()) {
            //qDebug() << "сообщение не дошло";
            //return;
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
            }
        }


        if (!recieveData.nameUsers.isEmpty()) { //прислали ip клиентов
            ui -> listClients ->clear();
            connectedUsers.clear();

            connectedUsers = recieveData.nameUsers;
            //добавляю в список других клиентов
            for (const QString &ip : connectedUsers) {
                ui -> listClients -> addItem(ip);
            }
        }
    }
}

void Widget::socket_disconnect() //отключение сервера
{
    isServerConnected = false;

    ui -> pushButton_Send -> setEnabled(false);
    ui -> sendImage -> setEnabled(false);
    ui -> pushButton_Connect -> setText("Connect");
    ui -> groupBox -> setEnabled(true);
    ui -> listClients -> clear();
    connectedUsers.clear();
}


void Widget::on_colorButton_clicked() //кнопка выбора цвета
{
    colorSendMessage = QColorDialog::getColor(QColor(Qt::white),this,"Выберите цвет сообщений");
    ui -> colorButton ->setStyleSheet("color: " + colorSendMessage.name() + ";");
}

void Widget::on_inputEdit_textChanged(const QString &arg1) //изменение в строке ввода сообщений
{
    if (arg1.trimmed().isEmpty() or ui->pushButton_Connect->text() == "Connect") {
        ui -> pushButton_Send -> setEnabled(false);
        send = false;
    }
    else {
        ui -> pushButton_Send -> setEnabled(true);
        send = true;
    }
}



