#ifndef FEATUREEXPORTER_H
#define FEATUREEXPORTER_H

#include <QObject>
#include <QStringList>
#include <QTextStream>

#include "data/DataProxy.h"

class QIODevice;
class Feature;

// Simple class to export an UserSelection to a file
class FeatureExporter
{
    Q_FLAGS(DetailLevel)
    Q_FLAGS(SeparationMode)

public:
    // exporter state enums
    enum DetailLevel {
        Simple = 0x00,
        Extended = 0x01,
        Comments = 0x10,
        // composite flags
        SimpleFull = Simple | Comments,
        ExtendedFull = Extended | Comments
    };
    Q_DECLARE_FLAGS(DetailLevels, DetailLevel)

    // separation mode for items in the file
    enum SeparationMode { TabDelimited, CommaDelimited };
    Q_DECLARE_FLAGS(SeparationModes, SeparationMode)

    FeatureExporter();
    FeatureExporter(DetailLevels detailLevel, SeparationModes separationMode);
    ~FeatureExporter();

    // the method to call to write the features to a file
    void exportItem(QIODevice &device, const DataProxy::FeatureList &featureList) const;

    // to add dybamic properties to the document
    void addExportProperty(const QString &property);
    void addExportProperty(const QStringList &properties);

    // to encode properties
    static const QString encodePropertyList(const QStringList &properties);
    static const QStringList decodePropertyList(const QString &properties);

private:
    // to get the list of properties of the document
    const QStringList exportPropertyList() const;
    const QString delimiterCharacter() const;
    // internal functions to do the writing to file object by object
    void exportStrings(QTextStream &otxt, const QStringList &strings) const;
    void exportItem(QTextStream &otxt, const Feature &feature) const;
    void exportItem(QTextStream &otxt, const DataProxy::FeatureList &selectionList) const;

    DetailLevels m_detailLevel;
    SeparationModes m_separationMode;
    QStringList m_propertyList;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(FeatureExporter::DetailLevels)
Q_DECLARE_OPERATORS_FOR_FLAGS(FeatureExporter::SeparationModes)

#endif // FEATUREEXPORTER_H
