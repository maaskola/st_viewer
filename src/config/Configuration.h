#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QSettings>
#include <QString>
#include <QPointer>

// Configuration is a convenience class that simplifies the access of
// statically stored configuration data by providing a globally
// accessible instance thanks to the fact that the data
// is stored in QSettings
class Configuration
{

public:
    Configuration();
    ~Configuration();

    // TODO if more settings are added let each object accessing the
    //     configurations store the access string literals as static values
    //     and make readSetting public instead. doesn't make sense to provide
    //     each setting as a function.

    // url
    const QString EndPointUrl() const;

    // version
    const QString dataEndpointMinVersion() const;

    // oauth
    const QString oauthClientID() const;
    const QString oauthScope() const;
    const QString oauthSecret() const;
    const QString oauthEndpointAuthorize() const;
    const QString oauthEndpointToken() const;

    // data access
    const QString dataEndpointChips() const;
    const QString dataEndpointDatasets() const;
    const QString dataEndpointImageAlingment() const;
    const QString dataEndpointFeatures() const;
    const QString dataEndpointUsers() const;
    const QString dataEndpointSelections() const;
    const QString dataEndpointFigures() const;

    // True if the configuration file has a url defined
    bool has_network() const;
    // True if the QSettings object is initilized and valid
    bool is_valid() const;
    // The path to the SSL key file if present in the configuration file
    const QString pathSSL() const;

private:
    // reads the setting stored in the key given and returns
    // its value or empty string if there was a problem
    const QString readSetting(const QString &key) const;

    QScopedPointer<QSettings> m_settings;

    Q_DISABLE_COPY(Configuration)
};

#endif /* CONFIGURATION_H */
