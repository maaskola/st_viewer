/*
    Copyright (C) 2012  Spatial Transcriptomics AB,
    read LICENSE for licensing terms.
    Contact : Jose Fernandez Navarro <jose.fernandez.navarro@scilifelab.se>

*/

#include "User.h"

User::User()
    : m_id(),
      m_username(),
      m_instituion(),
      m_firstName(),
      m_address(),
      m_postcode(0),
      m_city(),
      m_country(),
      m_password(),
      m_role(),
      m_enabled(false)
{

}


User::User(const User& other)
    : m_id(other.m_id),
      m_username(other.m_username),
      m_instituion(other.m_instituion),
      m_firstName(other.m_firstName),
      m_address(other.m_address),
      m_postcode(other.m_postcode),
      m_city(other.m_city),
      m_country(other.m_country),
      m_password(other.m_password),
      m_role(other.m_role),
      m_enabled(other.m_enabled)
{

}

User::~User()
{

}

User& User::operator=(const User& other)
{
    m_id = other.m_id;
    m_username = other.m_username;
    m_instituion = other.m_instituion;
    m_firstName = other.m_firstName;
    m_address = other.m_address;
    m_postcode = other.m_postcode;
    m_city = other.m_city;
    m_country = other.m_country;
    m_password = other.m_password;
    m_role = other.m_role;
    m_enabled = other.m_enabled;
    return (*this);
}

bool User::operator==(const User& other) const
{
    return (
                m_id == other.m_id &&
                m_username == other.m_username &&
                m_instituion == other.m_instituion &&
                m_firstName == other.m_firstName &&
                m_address == other.m_address &&
                m_postcode == other.m_postcode &&
                m_city == other.m_city &&
                m_country == other.m_country &&
                m_password == other.m_password &&
                m_role == other.m_role &&
                m_enabled == other.m_enabled
        );
}

const QString User::id() const
{
    return m_id;
}

const QString User::username() const
{
    return m_username;
}

const QString User::institution() const
{
    return m_instituion;
}

const QString User::firstName() const
{
    return m_firstName;
}

const QString User::secondName() const
{
    return m_secondName;
}

const QString User::address() const
{
    return m_address;
}

int User::postcode() const
{
    return m_postcode;
}

const QString User::city() const
{
    return m_city;
}

const QString User::country() const
{
    return m_country;
}

const QString User::password() const
{
    return m_password;
}

const QString User::role() const
{
    return m_role;
}

bool User::enabled() const
{
    return m_enabled;
}

void User::id(const QString& id)
{
    m_id = id;
}

void User::username(const QString& username)
{
    m_username = username;
}

void User::institution(const QString& institution)
{
    m_instituion = institution;
}

void User::firstName(const QString& firstName)
{
    m_firstName = firstName;
}

void User::secondName(const QString& secondName)
{
    m_secondName = secondName;
}

void User::address(const QString& address)
{
    m_address = address;
}

void User::postcode(int postCode)
{
    m_postcode = postCode;
}

void User::city(const QString& city)
{
    m_city = city;
}

void User::country(const QString& country)
{
    m_country = country;
}

void User::password(const QString& password)
{
    m_password = password;
}

void User::role(const QString& role)
{
    m_role = role;
}

void User::enabled(bool enabled)
{
    m_enabled = enabled;
}
