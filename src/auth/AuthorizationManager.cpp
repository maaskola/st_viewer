#include "AuthorizationManager.h"

#include "auth/OAuth2.h"
#include "data/DataProxy.h"

AuthorizationManager::AuthorizationManager(QSharedPointer<DataProxy> dataProxy, QObject *parent)
    : QObject(parent)
    , m_oAuth2(nullptr)
    , m_dataProxy(dataProxy)
{
}

AuthorizationManager::~AuthorizationManager()
{
}

void AuthorizationManager::startAuthorization()
{
    // Lazy init
    if (m_oAuth2.isNull()) {
        m_oAuth2.reset(new OAuth2(m_dataProxy, this));
        connect(m_oAuth2.data(),
                SIGNAL(signalLoginDone(const QUuid &, int, const QUuid &)),
                this,
                SLOT(slotLoginDone(const QUuid &, int, const QUuid &)));
        connect(m_oAuth2.data(),
                SIGNAL(signalError(QSharedPointer<Error>)),
                this,
                SIGNAL(signalError(QSharedPointer<Error>)));
    }
    // Check if we already have been authorized and have access token saved
    // initialize authentication on valid token storage
    if (isAuthenticated()) {
        emit signalAuthorize();
    } else if (m_tokenStorage.hasRefreshToken()) {
        m_oAuth2->startQuietLogin(m_tokenStorage.getRefreshToken());
    } else {
        m_oAuth2->startInteractiveLogin();
    }
}

void AuthorizationManager::cleanAccesToken()
{
    m_tokenStorage.cleanAll();
}

void AuthorizationManager::slotLoginDone(const QUuid &accessToken,
                                         const int expiresIn,
                                         const QUuid &refreshToken)
{
    m_tokenStorage.setAccessToken(accessToken, expiresIn);
    m_tokenStorage.setRefreshToken(refreshToken);
    emit signalAuthorize();
}

bool AuthorizationManager::isAuthenticated() const
{
    return m_tokenStorage.hasAccessToken() && !m_tokenStorage.isExpired();
}

bool AuthorizationManager::hasAccessToken() const
{
    return m_tokenStorage.hasAccessToken();
}

QUuid AuthorizationManager::getAccessToken() const
{
    return m_tokenStorage.getAccessToken();
}
