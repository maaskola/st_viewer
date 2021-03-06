#ifndef JSONERROR_H
#define JSONERROR_H

#include <QJsonParseError>

#include "Error.h"

class JSONError : public Error
{
    Q_OBJECT

public:
    explicit JSONError(QObject *parent = 0);
    JSONError(QJsonParseError::ParseError error, QObject *parent = 0);
    virtual ~JSONError();

private:
    void init(QJsonParseError::ParseError error);
};

#endif // JSONERROR_H //
