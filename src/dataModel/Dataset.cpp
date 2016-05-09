#include "Dataset.h"

#include <QStringList>
#include <QVariant>
#include <QDate>
#include <QDebug>

Dataset::Dataset()
    : m_id()
    , m_name()
    , m_alignmentId()
    , m_statBarcodes(0)
    , m_statGenes(0)
    , m_statUniqueBarcodes(0)
    , m_statUniqueGenes(0)
    , m_statTissue()
    , m_statSpecies()
    , m_statComments()
    , m_enabled(false)
    , m_grantedAccounts()
    , m_createdByAccount()
    , m_created(QDate::currentDate().toString())
    , m_lastMofidied(QDate::currentDate().toString())
    , m_downloaded(true)
{
}

Dataset::Dataset(const Dataset &other)
{
    m_name = other.m_name;
    m_id = other.m_id;
    m_alignmentId = other.m_alignmentId;
    m_statBarcodes = other.m_statBarcodes;
    m_statGenes = other.m_statGenes;
    m_statUniqueBarcodes = other.m_statUniqueBarcodes;
    m_statUniqueGenes = other.m_statUniqueGenes;
    m_statTissue = other.m_statTissue;
    m_statSpecies = other.m_statSpecies;
    m_statComments = other.m_statComments;
    m_oboFroundryTerms = other.m_oboFroundryTerms;
    m_geneHitsQuartiles = other.m_geneHitsQuartiles;
    m_genePooledHitsQuartiles = other.m_genePooledHitsQuartiles;
    m_enabled = other.m_enabled;
    m_grantedAccounts = other.m_grantedAccounts;
    m_createdByAccount = other.m_createdByAccount;
    m_created = other.m_created;
    m_lastMofidied = other.m_lastMofidied;
    m_downloaded = other.m_downloaded;
}

Dataset::~Dataset()
{
}

Dataset &Dataset::operator=(const Dataset &other)
{
    m_id = other.m_id;
    m_name = other.m_name;
    m_alignmentId = other.m_alignmentId;
    m_statBarcodes = other.m_statBarcodes;
    m_statGenes = other.m_statGenes;
    m_statUniqueBarcodes = other.m_statUniqueBarcodes;
    m_statUniqueGenes = other.m_statUniqueGenes;
    m_statTissue = other.m_statTissue;
    m_statSpecies = other.m_statSpecies;
    m_statComments = other.m_statComments;
    m_oboFroundryTerms = other.m_oboFroundryTerms;
    m_geneHitsQuartiles = other.m_geneHitsQuartiles;
    m_genePooledHitsQuartiles = other.m_genePooledHitsQuartiles;
    m_enabled = other.m_enabled;
    m_grantedAccounts = other.m_grantedAccounts;
    m_createdByAccount = other.m_createdByAccount;
    m_created = other.m_created;
    m_lastMofidied = other.m_lastMofidied;
    m_downloaded = other.m_downloaded;
    return (*this);
}

bool Dataset::operator==(const Dataset &other) const
{
    return (m_id == other.m_id && m_name == other.m_name && m_alignmentId == other.m_alignmentId
            && m_statBarcodes == other.m_statBarcodes && m_statGenes == other.m_statGenes
            && m_statUniqueBarcodes == other.m_statUniqueBarcodes
            && m_statUniqueGenes == other.m_statUniqueGenes && m_statTissue == other.m_statTissue
            && m_statSpecies == other.m_statSpecies && m_statComments == other.m_statComments
            && m_oboFroundryTerms == other.m_oboFroundryTerms
            && m_geneHitsQuartiles == other.m_geneHitsQuartiles
            && m_genePooledHitsQuartiles == other.m_genePooledHitsQuartiles
            && m_enabled == other.m_enabled && m_grantedAccounts == other.m_grantedAccounts
            && m_createdByAccount == other.m_createdByAccount && m_created == other.m_created
            && m_lastMofidied == other.m_lastMofidied && m_downloaded == other.m_downloaded);
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

unsigned Dataset::statBarcodes() const
{
    return m_statBarcodes;
}

unsigned Dataset::statGenes() const
{
    return m_statGenes;
}

unsigned Dataset::statUniqueBarcodes() const
{
    return m_statUniqueBarcodes;
}

unsigned Dataset::statUniqueGenes() const
{
    return m_statUniqueGenes;
}

const QString Dataset::statTissue() const
{
    return m_statTissue;
}

const QString Dataset::statSpecies() const
{
    return m_statSpecies;
}

const QString Dataset::statComments() const
{
    return m_statComments;
}

const QVector<float> Dataset::hitsQuartiles() const
{
    return m_geneHitsQuartiles;
}

const QVector<float> Dataset::hitsPooledQuartiles() const
{
    return m_genePooledHitsQuartiles;
}

bool Dataset::enabled() const
{
    return m_enabled;
}

const QVector<QString> Dataset::grantedAccounts() const
{
    return m_grantedAccounts;
}

const QString Dataset::createdByAccount() const
{
    return m_createdByAccount;
}

const QString Dataset::created() const
{
    return m_created;
}

const QString Dataset::lastModified() const
{
    return m_lastMofidied;
}

bool Dataset::downloaded() const
{
    return m_downloaded;
}

void Dataset::id(const QString &id)
{
    m_id = id;
}

void Dataset::name(const QString &name)
{
    m_name = name;
}

void Dataset::imageAlignmentId(const QString &alignmentId)
{
    m_alignmentId = alignmentId;
}

void Dataset::statBarcodes(unsigned barcodes)
{
    m_statBarcodes = barcodes;
}

void Dataset::statGenes(unsigned genes)
{
    m_statGenes = genes;
}

void Dataset::statUniqueBarcodes(unsigned uniqueBarcodes)
{
    m_statUniqueBarcodes = uniqueBarcodes;
}

void Dataset::statUniqueGenes(unsigned uniqueGenes)
{
    m_statUniqueGenes = uniqueGenes;
}

void Dataset::statTissue(const QString &statTissue)
{
    m_statTissue = statTissue;
}

void Dataset::statSpecies(const QString &statSpecies)
{
    m_statSpecies = statSpecies;
}

void Dataset::statComments(const QString &statComments)
{
    m_statComments = statComments;
}

void Dataset::hitsQuartiles(const QVector<float> &hitsQuartiles)
{
    m_geneHitsQuartiles = hitsQuartiles;
}

void Dataset::hitsPooledQuartiles(const QVector<float> &hitsPooledQuartiles)
{
    m_genePooledHitsQuartiles = hitsPooledQuartiles;
}

void Dataset::enabled(const bool enabled)
{
    m_enabled = enabled;
}

void Dataset::grantedAccounts(const QVector<QString> &grantedAccounts)
{
    m_grantedAccounts = grantedAccounts;
}

void Dataset::createdByAccount(const QString &created)
{
    m_createdByAccount = created;
}

void Dataset::created(const QString &created)
{
    m_created = created;
}

void Dataset::lastModified(const QString &lastModified)
{
    m_lastMofidied = lastModified;
}

void Dataset::downloaded(const bool downloaded)
{
    m_downloaded = downloaded;
}

float Dataset::statisticsMin() const
{
    Q_ASSERT(m_geneHitsQuartiles.size() == 5);
    // returning q0, alternatively could return
    // max ( q1 - 1.5 * (q3-q1), q0 )
    return m_geneHitsQuartiles.at(0);
}

float Dataset::statisticsMax() const
{
    Q_ASSERT(m_geneHitsQuartiles.size() == 5);
    // returning q4 an alternative would be
    // min ( q3 + 1.5 * (q3-q1), q4 )
    return m_geneHitsQuartiles.at(4);
}

float Dataset::statisticsPooledMin() const
{
    Q_ASSERT(m_genePooledHitsQuartiles.size() == 5);
    // returning q0, alternatively could return
    // max ( q1 - 1.5 * (q3-q1), q0 )
    return m_genePooledHitsQuartiles.at(0);
}

float Dataset::statisticsPooledMax() const
{
    Q_ASSERT(m_genePooledHitsQuartiles.size() == 5);
    // returning q4 an alternative would be
    // min ( q3 + 1.5 * (q3-q1), q4 )
    return m_genePooledHitsQuartiles.at(4);
}
