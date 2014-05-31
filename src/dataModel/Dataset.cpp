/*
    Copyright (C) 2012  Spatial Transcriptomics AB,
    read LICENSE for licensing terms.
    Contact : Jose Fernandez Navarro <jose.fernandez.navarro@scilifelab.se>

*/

#include "Dataset.h"
#include <QStringList>
#include <QVariant>

Dataset::Dataset()
    : m_id(),
      m_name(),
      m_alignmentId(),
      m_statBarcodes(0),
      m_statGenes(0),
      m_statUniqueBarcodes(0),
      m_statUniqueGenes(0),
      m_statTissue(),
      m_statSpecie(),
      m_statComments()
{

}

Dataset::Dataset(const Dataset& other)
{
    m_name = other.m_name;
    m_id = other.m_id;
    m_alignmentId = other.m_alignmentId;
    m_statBarcodes = other.m_statBarcodes;
    m_statGenes = other.m_statGenes;
    m_statUniqueBarcodes = other.m_statUniqueBarcodes;
    m_statUniqueGenes = other.m_statUniqueGenes;
    m_statTissue = other.m_statTissue;
    m_statSpecie = other.m_statSpecie;
    m_statComments = other.m_statComments;
    m_oboFroundryTerms = other.m_oboFroundryTerms;
    m_genePooledHitsQuartiles = other.m_genePooledHitsQuartiles;
}

Dataset::~Dataset()
{

}

Dataset& Dataset::operator=(const Dataset& other)
{
    m_id = other.m_id;
    m_name = other.m_name;
    m_alignmentId = other.m_alignmentId;
    m_statBarcodes = other.m_statBarcodes;
    m_statGenes = other.m_statGenes;
    m_statUniqueBarcodes = other.m_statUniqueBarcodes;
    m_statUniqueGenes = other.m_statUniqueGenes;
    m_statTissue = other.m_statTissue;
    m_statSpecie = other.m_statSpecie;
    m_statComments = other.m_statComments;
    m_oboFroundryTerms = other.m_oboFroundryTerms;
    m_genePooledHitsQuartiles = other.m_genePooledHitsQuartiles;
    return (*this);
}

bool Dataset::operator==(const Dataset& other) const
{
    return (
               m_id == other.m_id &&
               m_name == other.m_name &&
               m_alignmentId == other.m_alignmentId &&
               m_statBarcodes == other.m_statBarcodes &&
               m_statGenes == other.m_statGenes &&
               m_statUniqueBarcodes == other.m_statUniqueBarcodes &&
               m_statUniqueGenes == other.m_statUniqueGenes &&
               m_statTissue == other.m_statTissue &&
               m_statSpecie == other.m_statSpecie &&
               m_statComments == other.m_statComments &&
               m_oboFroundryTerms == other.m_oboFroundryTerms &&
               m_genePooledHitsQuartiles == other.m_genePooledHitsQuartiles
           );
}

const QString Dataset::id() const
{
    return m_id;
}

const QString Dataset::name() const
{
    return m_name;
}

const QString Dataset::imageAlignmentId() const
{
    return m_alignmentId;
}

int Dataset::statBarcodes() const
{
    return m_statBarcodes;
}

int Dataset::statGenes() const
{
    return m_statGenes;
}

int Dataset::statUniqueBarcodes() const
{
    return m_statUniqueBarcodes;
}

int Dataset::statUniqueGenes() const
{
    return m_statUniqueGenes;
}

const QString Dataset::statTissue() const
{
    return m_statTissue;
}

const QString Dataset::statSpecie() const
{
    return m_statSpecie;
}

const QString Dataset::statComments() const
{
    return m_statComments;
}

const QList<QString> Dataset::oboFoundryTerms() const
{
    return m_oboFroundryTerms;
}

const QVector<qreal> Dataset::hitsQuartiles() const
{
    return m_genePooledHitsQuartiles;
}

void Dataset::id(const QString& id)
{
    m_id = id;
}

void Dataset::name(const QString& name)
{
    m_name = name;
}

void Dataset::imageAlignmentId(const QString& alignmentId)
{
    m_alignmentId = alignmentId;
}

void Dataset::statBarcodes(int barcodes)
{
    m_statBarcodes = barcodes;
}

void Dataset::statGenes(int genes)
{
    m_statGenes = genes;
}

void Dataset::statUniqueBarcodes(int uniqueBarcodes)
{
    m_statUniqueBarcodes = uniqueBarcodes;
}

void Dataset::statUniqueGenes(int uniqueGenes)
{
    m_statUniqueGenes = uniqueGenes;
}

void Dataset::statTissue(const QString& statTissue)
{
    m_statTissue = statTissue;
}

void Dataset::statSpecie(const QString& statSpecie)
{
    m_statSpecie = statSpecie;
}

void Dataset::statComments(const QString& statComments)
{
    m_statComments = statComments;
}

void Dataset::oboFoundryTerms(const QList<QString>& oboFoundryTerms)
{
    m_oboFroundryTerms = oboFoundryTerms;
}

void Dataset::hitsQuartiles(const QVector<qreal>& hitsQuartiles)
{
    m_genePooledHitsQuartiles = hitsQuartiles;
}

qreal Dataset::statisticsMin() const
{
    Q_ASSERT(m_genePooledHitsQuartiles.size() == 5);
    // max ( q1 - 1.5 * (q3-q1), q0 )
    const qreal q0 = m_genePooledHitsQuartiles.at(0);
    const qreal q1 = m_genePooledHitsQuartiles.at(1);
    const qreal q3 = m_genePooledHitsQuartiles.at(3);
    return std::max(q1 - 1.5 * (q3 - q1), q0);
}

qreal Dataset::statisticsMax() const
{
    Q_ASSERT(m_genePooledHitsQuartiles.size() == 5);
    // min ( q3 + 1.5 * (q3-q1), q4 )
    const qreal q4 = m_genePooledHitsQuartiles.at(4);
    const qreal q1 = m_genePooledHitsQuartiles.at(1);
    const qreal q3 = m_genePooledHitsQuartiles.at(3);
    return std::min(q3 + 1.5 * (q3 - q1), q4);
}

qreal Dataset::statisticsPooledMin() const
{
    Q_ASSERT(m_genePooledHitsQuartiles.size() == 5);
    // max ( q1 - 1.5 * (q3-q1), q0 )
    const qreal q0 = m_genePooledHitsQuartiles.at(0);
    const qreal q1 = m_genePooledHitsQuartiles.at(1);
    const qreal q3 = m_genePooledHitsQuartiles.at(3);
    return std::max(q1 - 1.5 * (q3 - q1), q0);
}

qreal Dataset::statisticsPooledMax() const
{
    Q_ASSERT(m_genePooledHitsQuartiles.size() == 5);
    // min ( q3 + 1.5 * (q3-q1), q4 )
    const qreal q4 = m_genePooledHitsQuartiles.at(4);
    const qreal q1 = m_genePooledHitsQuartiles.at(1);
    const qreal q3 = m_genePooledHitsQuartiles.at(3);
    return std::min(q3 + 1.5 * (q3 - q1), q4);
}

