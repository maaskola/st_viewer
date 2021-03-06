#ifndef NETWORKREPLY_H
#define NETWORKREPLY_H

#include <QObject>
#include <QNetworkReply>
#include <QPointer>
#include <QList>

class Error;
class QSslError;
class QJsonDocument;
class QTimer;

// NetworkReply represents a handle to the asynchronous network request
// managed by the NetworkManager. This handle provides a means of parsing
// the resulting data as well as listening to any errors.
// NetworkReply extends AsyncRequest which means that it is guaranteed to
// emit a signalFinished when it has completed.
class NetworkReply : public QObject
{
    Q_OBJECT

public:
    enum ReturnCode { CodeSuccess = 0x01, CodeAbort = 0x02, CodeError = 0x04 };

    typedef QList<QSharedPointer<Error>> ErrorList;

    explicit NetworkReply(QNetworkReply *networkReply = 0);
    ~NetworkReply();

    // parse body (once the data has been parsed cannot be parsed again)
    QJsonDocument getJSON();
    QString getText() const;
    QByteArray getRaw() const;

    // reply status
    bool isFinished() const;
    bool hasErrors() const;
    ReturnCode return_code() const;

    // return reply errors if any
    const NetworkReply::ErrorList &errors() const;

    // this function will create a single error with all the error messages
    // if no errors are present it returns nullptr
    QSharedPointer<Error> parseErrors();

    // true if the reply request was obtained from the disk cache
    bool wasCached() const;

    // adds an error to the list
    void registerError(QSharedPointer<Error> error);

public slots:

    // These slots are invoked from the Qt network reply object
    void slotAbort();
    void slotFinished();
    void slotMetaDataChanged();
    void slotError(QNetworkReply::NetworkError networkError);
    void slotSslErrors(QList<QSslError> sslErrorList);

signals:

    // signal operation Finished (code = abort, error, ok)
    void signalFinished(QVariant code);

private:
    // Qt network reply
    QScopedPointer<QNetworkReply> m_reply;
    // errors
    ErrorList m_errors;
    // return status code
    ReturnCode m_code;
    // header content type
    QString m_mime;

    Q_DISABLE_COPY(NetworkReply)
};

#endif // NETWORKREPLY_H //
