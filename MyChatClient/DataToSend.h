#ifndef DATATOSEND_H
#define DATATOSEND_H

#include <QString>
#include <QColor>
#include <QDate>
#include <QTcpSocket>
#include <QImage>

struct DataToSend { // Структура для передачи данных
    QString name;
    QString message;
    QColor color;
    QDateTime dateTime;
    QList <QString> nameUsers;
    QImage image;

    // Оператор для сериализации
    friend QDataStream &operator << (QDataStream &out, const DataToSend &data) {
        out << data.name;
        out << data.message;
        out << data.color;
        out << data.dateTime;
        out << data.nameUsers;
        out << data.image;
        return out;
    }

    // Оператор для десериализации
    friend QDataStream &operator >> (QDataStream &in, DataToSend &data) {
        in >> data.name;
        in >> data.message;
        in >> data.color;
        in >> data.dateTime;
        in >> data.nameUsers;
        in >> data.image;
        return in;
    }

    // "friend" указывает на то, что эта функция будет дружественной для структуры,
    // и она имеет доступ к его закрытым членам.
};
#endif // DATATOSEND_H
