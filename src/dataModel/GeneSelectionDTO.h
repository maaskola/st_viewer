/*
    Copyright (C) 2012  Spatial Transcriptomics AB,
    read LICENSE for licensing terms.
    Contact : Jose Fernandez Navarro <jose.fernandez.navarro@scilifelab.se>

*/

#ifndef GENESELECTIONDTO_H
#define GENESELECTIONDTO_H

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>

#include "dataModel/GeneSelection.h"

// DTOs (Data transform objects) provides the bridge between the client
// specific data model objects and the server data. This is accomplished by
// defining dynamic properties that enable automated serialization and
// deserialization of server data.

Q_DECLARE_METATYPE(SelectionType)

class GeneSelectionDTO : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString id READ id WRITE id)
    Q_PROPERTY(QString name READ name WRITE name)
    Q_PROPERTY(QString account_id READ userId WRITE userId)
    Q_PROPERTY(QString dataset_id READ datasetId WRITE datasetId)
    Q_PROPERTY(QVariantList gene_hits READ selectedItems WRITE selectedItems)
    Q_PROPERTY(QString type READ type WRITE type)
    Q_PROPERTY(QString status READ status WRITE status)
    Q_PROPERTY(QVariantList obo_foundry_terms READ oboFoundryTerms WRITE oboFoundryTerms)
    Q_PROPERTY(QString comment READ comment WRITE comment)
    Q_PROPERTY(bool enabled READ enabled WRITE enabled)
    Q_PROPERTY(QString created_at READ created WRITE created)
    Q_PROPERTY(QString last_modified READ lastModified WRITE lastModified)

public:

    explicit GeneSelectionDTO(QObject* parent = 0) : QObject(parent) {}
    GeneSelectionDTO(const GeneSelection& selection, QObject* parent = 0) :
        QObject(parent), m_geneSelection(selection) {}
    ~GeneSelectionDTO() {}

    //getters
    const QString id() const { return m_geneSelection.id(); }
    const QString name() const { return m_geneSelection.name(); }
    const QString userId() const { return m_geneSelection.userId(); }
    const QString datasetId() const { return m_geneSelection.datasetId(); }
    const QVariantList selectedItems() const
      { return serializeSelectiosVector(m_geneSelection.selectedItems()); }
    const QString type() const { return m_geneSelection.type(); }
    const QString status() const { return m_geneSelection.status(); }
    const QVariantList oboFoundryTerms() const
      { return serializeVector<QString>(m_geneSelection.oboFoundryTerms()); }
    const QString comment() const { return m_geneSelection.comment(); }
    bool enabled() const { return m_geneSelection.enabled(); }
    const QString created() const { return m_geneSelection.created(); }
    const QString lastModified() const { return m_geneSelection.lastModified(); }

    // binding
    void id(const QString& id) { m_geneSelection.id(id); }
    void name(const QString& name) { m_geneSelection.name(name); }
    void userId(const QString& userId) { m_geneSelection.userId(userId); }
    void datasetId(const QString& datasetId) { m_geneSelection.datasetId(datasetId); }
    void selectedItems(const QVariantList& selectedItems)
      { m_geneSelection.selectedItems(unserializeVector<SelectionType>(selectedItems)); }
    void type(const QString& type) { m_geneSelection.type(type); }
    void status(const QString& status) { m_geneSelection.status(status); }
    void oboFoundryTerms(const QVariantList& oboFoundryTerms)
      { m_geneSelection.oboFoundryTerms(unserializeVector<QString>(oboFoundryTerms)); }
    void comment(const QString& comment) { m_geneSelection.comment(comment); }
    void enabled(const bool enabled) { m_geneSelection.enabled(enabled); }
    void created(const QString& created) { m_geneSelection.created(created); }
    void lastModified(const QString& lastModified) { m_geneSelection.lastModified(lastModified); }

    // get parsed data model
    const GeneSelection& geneSelection() const { return m_geneSelection; }
    GeneSelection& geneSelection() { return m_geneSelection; }

    QByteArray toJson() const
    {
        QJsonObject jsonObj;
        //jsonObj["id"] = id();
        jsonObj["name"] = name();
        jsonObj["account_id"] = userId();
        jsonObj["dataset_id"] = datasetId();
        QJsonArray geneHits;
        foreach(const SelectionType &item, m_geneSelection.selectedItems()) {
            QJsonArray geneHit;
            geneHit.append(item.name);
            geneHit.append(QString::number(item.reads));
            geneHit.append(QString::number(item.normalizedReads));
            geneHit.append(QString::number(item.pixeIntensity));
            geneHits.append(geneHit);
        }
        jsonObj["gene_hits"] = geneHits;
        jsonObj["type"] = !type().isEmpty() ? QJsonValue(type()) : QJsonValue::Null;
        jsonObj["status"] = !status().isEmpty() ? QJsonValue(status()) : QJsonValue::Null;
        QJsonArray oboTerms;
        foreach(const QString &item, m_geneSelection.oboFoundryTerms()) {
            oboTerms.append(item);
        }
        jsonObj["obo_foundry_terms"] = oboTerms;
        jsonObj["comment"] = !comment().isEmpty() ? QJsonValue(comment()) : QJsonValue::Null;
        jsonObj["enabled"] = enabled();
        jsonObj["created_at"] = QJsonValue::Null; //null for now until we know what format is used in server
        jsonObj["last_modified"] = QJsonValue::Null; //null for now until we know what format is used in server

        QJsonDocument doc(jsonObj);
        QByteArray serializedDoc = doc.toJson(QJsonDocument::Compact);
        return serializedDoc;
    }

private:

    //TODO duplicated in other DTOs move to Utils class
    const QVariantList serializeSelectiosVector(const QVector<SelectionType> &unserializedVector) const
    {
        QVariantList newList;
        foreach(const SelectionType &item, unserializedVector.toList()) {
            QVariantList itemList;
            itemList << item.name << item.reads << item.normalizedReads << item.pixeIntensity;
            newList << QVariant::fromValue(itemList);
        }
        return newList;
    }

    template<typename N>
    const QVariantList serializeVector(const QVector<N>& unserializedVector) const
    {
        QVariantList newList;
        foreach(const N &item, unserializedVector.toList()) {
            newList << QVariant::fromValue(item);
        }
        return newList;
    }

    template<typename N>
    const QVector<N> unserializeVector(const QVariantList &serializedVector) const
    {
        // unserialize data
        QVector<N> values;
        QVariantList::const_iterator it;
        QVariantList::const_iterator end = serializedVector.end();
        for (it = serializedVector.begin(); it != end; ++it) {
            values.push_back(it->value<N>());
        }
        return values;
    }

    GeneSelection m_geneSelection;
};

#endif // GENESELECTIONDTO_H //
