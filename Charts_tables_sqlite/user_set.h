#ifndef USER_SET_H
#define USER_SET_H

#include <QString>
#include <QColor>

class User_set
{
public:
    int indx;
    QString label;
    QColor color;
    explicit User_set(int i, QString l, QColor c);
};

#endif // USER_SET_H
