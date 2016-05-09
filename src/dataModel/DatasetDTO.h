#ifndef DATASETDTO_H
#define DATASETDTO_H

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>

#include "dataModel/Dataset.h"
#include "utils/SerializationFunctions.h"

// DTOs (Data transform objects) provides the bridge between the client
// specific data model objects and the server data. This is accomplished by
// defining dynamic properties that enable automated serialization and
// deserialization of server data.

// TODO move definitions to CPP and/or consider removing DTOs
class DatasetDTO : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString id READ id WRITE id)
    Q_PROPERTY(QString name READ name WRITE name)
    Q_PROPERTY(QString image_alignment_id READ imageAlignmentId WRITE imageAlignmentId)
    Q_PROPERTY(QString tissue READ statTissue WRITE statTissue)
    Q_PROPERTY(QString species READ statSpecies WRITE statSpecies)
    Q_PROPERTY(unsigned overall_feature_count READ statBarcodes WRITE statBarcodes)
    Q_PROPERTY(unsigned overall_hit_count READ statGenes WRITE statGenes)
    Q_PROPERTY(unsigned unique_barcode_count READ statUniqueBarcodes WRITE statUniqueBarcodes)
    Q_PROPERTY(unsigned unique_gene_count READ statUniqueGenes WRITE statUniqueGenes)
    Q_PROPERTY(QVariantList overall_hit_quartiles READ hitsQuartiles WRITE hitsQuartiles)
    Q_PROPERTY(QVariantList gene_pooled_hit_quartiles READ hitsPooledQuartiles WRITE
                   hitsPooledQuartiles)
    Q_PROPERTY(QString comment READ statComments WRITE statComments)
    Q_PROPERTY(bool enabled READ enabled WRITE enabled)
    Q_PROPERTY(QVariantList granted_accounts READ grantedAccounts WRITE grantedAccounts)
    Q_PROPERTY(QString created_by_account_id READ createdByAccount WRITE createdByAccount)
    Q_PROPERTY(QString created_at READ created WRITE created)
    Q_PROPERTY(QString last_modified READ lastModified WRITE lastModified)

public:
    explicit DatasetDTO(QObject *parent = 0)
        : QObject(parent)
    {
    }
    DatasetDTO(const Dataset &dataset, QObject *parent = 0)
        : QObject(parent)
        , m_dataset(dataset)
    {
    }
    ~DatasetDTO() {}

    // binding
    void id(const QString &id) { m_dataset.id(id); }
    void name(const QString &name) { m_dataset.name(name); }
    void imageAlignmentId(const QString &alignmentId) { m_dataset.imageAlignmentId(alignmentId); }
    void statTissue(const QString &tissue) { m_dataset.statTissue(tissue); }
    void statSpecies(const QString &species) { m_dataset.statSpecies(species); }
    void statBarcodes(unsigned barcodes) { m_dataset.statBarcodes(barcodes); }
    void statGenes(unsigned genes) { m_dataset.statGenes(genes); }
    void statUniqueBarcodes(unsigned unique_barcodes)
    {
        m_dataset.statUniqueBarcodes(unique_barcodes);
    }
    void statUniqueGenes(unsigned unique_genes) { m_dataset.statUniqueGenes(unique_genes); }
    void hitsQuartiles(const QVariantList &hitQuartiles)
    {
        m_dataset.hitsQuartiles(unserializeVector<float>(hitQuartiles));
    }
    void hitsPooledQuartiles(const QVariantList &hitPooledQuartiles)
    {
        m_dataset.hitsPooledQuartiles(unserializeVector<float>(hitPooledQuartiles));
    }
    void statComments(const QString &comments) { m_dataset.statComments(comments); }
    void enabled(const bool enabled) { m_dataset.enabled(enabled); }
    void grantedAccounts(QVariantList grantedAccounts)
    {
        m_dataset.grantedAccounts(unserializeVector<QString>(grantedAccounts));
    }
    void created(const QString &created) { m_dataset.created(created); }
    void createdByAccount(const QString &created) { m_dataset.createdByAccount(created); }
    void lastModified(const QString &lastModified) { m_dataset.lastModified(lastModified); }

    // read
    const QString id() const { return m_dataset.id(); }
    const QString name() const { return m_dataset.name(); }
    const QString imageAlignmentId() const { return m_dataset.imageAlignmentId(); }
    const QString statTissue() const { return m_dataset.statTissue(); }
    const QString statSpecies() const { return m_dataset.statSpecies(); }
    unsigned statBarcodes() const { return m_dataset.statBarcodes(); }
    unsigned statGenes() const { return m_dataset.statGenes(); }
    unsigned statUniqueBarcodes() const { return m_dataset.statUniqueBarcodes(); }
    unsigned statUniqueGenes() const { return m_dataset.statUniqueGenes(); }
    const QVariantList hitsQuartiles() const
    {
        return serializeVector<float>(m_dataset.hitsQuartiles());
    }
    const QVariantList hitsPooledQuartiles() const
    {
        return serializeVector<float>(m_dataset.hitsPooledQuartiles());
    }
    const QString statComments() const { return m_dataset.statComments(); }
    bool enabled() const { return m_dataset.enabled(); }
    const QVariantList grantedAccounts() const
    {
        return serializeVector<QString>(m_dataset.grantedAccounts());
    }
    const QString createdByAccount() const { return m_dataset.createdByAccount(); }
    const QString created() const { return m_dataset.created(); }
    const QString lastModified() const { return m_dataset.lastModified(); }

    // toJson is needed to send PUT/POST requests as the JSON content of the
    // object
    // is appended to the request
    // TODO transform this to obtain fields dynamically using the meta_properties
    QByteArray toJson() const
    {
        QJsonObject jsonObj;
        jsonObj["id"] = !id().isNull() ? QJsonValue(id()) : QJsonValue::Null;
        jsonObj["name"] = name();
        jsonObj["image_alignment_id"] = imageAlignmentId();
        jsonObj["tissue"] = statTissue();
        jsonObj["species"] = statSpecies();
        jsonObj["overall_feature_count"] = static_cast<int>(statBarcodes());
        jsonObj["overall_hit_count"] = static_cast<int>(statGenes());
        jsonObj["unique_barcode_count"] = static_cast<int>(statUniqueBarcodes());
        jsonObj["unique_gene_count"] = static_cast<int>(statUniqueGenes());
        QJsonArray hitsQuartiles;
        for (const auto &item : m_dataset.hitsQuartiles()) {
            hitsQuartiles.append(item);
        }
        jsonObj["overall_hit_quartiles"] = hitsQuartiles;
        QJsonArray hitsPooledQuartiles;
        for (const auto &item : m_dataset.hitsPooledQuartiles()) {
            hitsPooledQuartiles.append(item);
        }
        jsonObj["gene_pooled_hit_quartiles"] = hitsPooledQuartiles;
        jsonObj["comment"] = !statComments().isNull() ? QJsonValue(statComments())
                                                      : QJsonValue::Null;
        jsonObj["enabled"] = enabled();
        QJsonArray grantedAccounts;
        for (const auto &item : m_dataset.grantedAccounts()) {
            grantedAccounts.append(item);
        }
        jsonObj["granted_accounts"] = grantedAccounts;
        jsonObj["created_by_account_id"] = createdByAccount();
        jsonObj["created_at"] = QJsonValue::Null;    // leave this empty the API will update it
        jsonObj["last_modified"] = QJsonValue::Null; // leave this empty the API will update it
        QJsonDocument doc(jsonObj);
        QByteArray serializedDoc = doc.toJson(QJsonDocument::Compact);
        return serializedDoc;
    }

    // get parsed data model
    const Dataset &dataset() const { return m_dataset; }
    Dataset &dataset() { return m_dataset; }

private:
    Dataset m_dataset;
};

#endif // DATASETDTO_H //
