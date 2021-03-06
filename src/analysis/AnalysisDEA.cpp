#include "AnalysisDEA.h"

#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QSortFilterProxyModel>

#include <cmath>
#include "math/Common.h"
#include "model/GeneSelectionDEAItemModel.h"

#include "ui_ddaWidget.h"

static const QColor BORDER = QColor(238, 122, 0);

AnalysisDEA::AnalysisDEA(const UserSelection &selObjectA,
                         const UserSelection &selObjectB,
                         QWidget *parent,
                         Qt::WindowFlags f)
    : QDialog(parent, f)
    , m_ui(new Ui::ddaWidget)
    , m_lowerThreshold(0)
    , m_upperThreshold(1)
{
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    setModal(true);

    m_ui->setupUi(this);
    // We must set the style here again as this dialog does not inherit
    // style from its parent (TODO might be possible to fix this)
    m_ui->tableView->setStyleSheet(
        "QTableView {alternate-background-color: rgb(245,245,245); "
        "            background-color: transparent; "
        "            selection-background-color: rgb(215,215,215); "
        "            selection-color: rgb(238,122,0); "
        "            gridline-color: rgb(240,240,240);"
        "            border: 1px solid rgb(240,240,240);} "
        "QTableView::indicator:unchecked {image: url(:/images/unchecked-box.png);} "
        "QTableView::indicator:checked {image: url(:/images/checked-box.png);} "
        "QTableView::indicator {padding-left: 10px; "
        "                       width: 15px; "
        "                       height: 15px; "
        "                       background-color: transparent;} "
        "QHeaderView::section {height: 35px; "
        "                      padding-left: 4px; "
        "                      padding-right: 2px; "
        "                      spacing: 5px;"
        "                      background-color: rgb(230,230,230); "
        "                      border: 1px solid rgb(240,240,240);} "
        "QTableCornerButton::section {background-color: transparent;} ");


    //TODO
    //Set up scatter plot objects..

    // populate the gene to read pairs containers
    // computeGeneToReads will update the max|min thresholds variables (to initialize slider)
    computeGeneToReads(selObjectA, selObjectB);

    // initialize threshold sliders (minimum must be zero to allow to discard non-expressed genes)
    m_ui->readsThreshold->setMinimumValue(0);
    m_ui->readsThreshold->setMaximumValue(m_upperThreshold);
    m_ui->readsThreshold->slotSetLowerValue(m_lowerThreshold);
    m_ui->readsThreshold->slotSetUpperValue(m_upperThreshold);
    m_ui->readsThreshold->setTickInterval(1);

    // update name fields in the UI
    m_ui->selectionA->setText(selObjectA.name());
    m_ui->selectionB->setText(selObjectB.name());

    // compute statistics
    const deaStats &stats = computeStatistics();

    // visualize statistics
    updateStatisticsUI(stats);

    // make connections
    connect(m_ui->buttonBox, SIGNAL(rejected()), this, SLOT(close()));
    connect(m_ui->buttonBox->button(QDialogButtonBox::Save),
            SIGNAL(clicked()),
            this,
            SLOT(slotSaveToPDF()));
    connect(m_ui->readsThreshold,
            SIGNAL(signalLowerValueChanged(int)),
            this,
            SLOT(slotSetLowerThreshold(int)));
    connect(m_ui->readsThreshold,
            SIGNAL(signalUpperValueChanged(int)),
            this,
            SLOT(slotSetUpperThreshold(int)));
    connect(m_ui->geneSearch,
            SIGNAL(textChanged(QString)),
            selectionsProxyModel(),
            SLOT(setFilterFixedString(QString)));
    connect(m_ui->tableView,
            SIGNAL(clicked(QModelIndex)),
            this,
            SLOT(slotSelectionSelected(QModelIndex)));
}

AnalysisDEA::~AnalysisDEA()
{
}

QSortFilterProxyModel *AnalysisDEA::selectionsProxyModel()
{
    QSortFilterProxyModel *selectionsProxyModel
        = qobject_cast<QSortFilterProxyModel *>(m_ui->tableView->model());
    Q_ASSERT(selectionsProxyModel);
    return selectionsProxyModel;
}

GeneSelectionDEAItemModel *AnalysisDEA::selectionsModel()
{
    GeneSelectionDEAItemModel *model
        = qobject_cast<GeneSelectionDEAItemModel *>(selectionsProxyModel()->sourceModel());
    Q_ASSERT(model);
    return model;
}

void AnalysisDEA::slotSelectionSelected(QModelIndex index)
{
    //TODO clear scatter plot for selected points

    if (index.isValid()) {
        const auto selected = m_ui->tableView->geneTableItemSelection();
        const auto currentSelection = selectionsModel()->getSelections(selected);
        // obtain the coordinates of the elements of the table selected
        // so we can then highlight them in the scatter plot
        QVector<double> x;
        QVector<double> y;
        for (const auto &deaRead : currentSelection) {
            if (combinedSelectionThreholsd(deaRead)) {
                continue;
            }
            x.append(deaRead.readsA);
            y.append(deaRead.readsB);
        }
        //TODO set new data for the scatter plot for selected points
    }

    //TODO update plotting objet (replot)
}

void AnalysisDEA::computeGeneToReads(const UserSelection &selObjectA,
                                     const UserSelection &selObjectB)
{
    // reset thresholds
    m_lowerThreshold = std::numeric_limits<int>::max();
    m_upperThreshold = std::numeric_limits<int>::min();

    // get the list of aggregated genes from both selections
    const auto &selA = selObjectA.getGeneCounts();
    const auto &selB = selObjectB.getGeneCounts();

    // take into account that some genes might be present in only one selection
    // therefore, we create a hash table (key gene name - value a pair with counts in selection A
    // and counts in selection B) to later know what genes are present in which set
    // depending of the value of the hash (0.0 no present)
    auto it1 = selA.begin();
    auto it2 = selB.begin();
    QHash<QString, deaReads> tempMap;
    while (it1 != selA.end() || it2 != selB.end()) {

        if (it1 != selA.end()) {
            const auto gene_name = it1->first;
            const auto gene_counts = it1->second;
            tempMap[gene_name].gene = gene_name;
            tempMap[gene_name].readsA = gene_counts;
            m_upperThreshold = std::max(gene_counts, m_upperThreshold);
            m_lowerThreshold = std::min(gene_counts, m_lowerThreshold);
            ++it1;
        }

        if (it2 != selB.end()) {
            const auto gene_name = it2->first;
            const auto gene_counts = it2->second;
            tempMap[gene_name].gene = gene_name;
            tempMap[gene_name].readsB = gene_counts;
            m_upperThreshold = std::max(gene_counts, m_upperThreshold);
            m_lowerThreshold = std::min(gene_counts, m_lowerThreshold);
            ++it2;
        }
    }

    // clear the container and fill it with the deaReads objects
    m_combinedSelections = tempMap.values();

    // update table model for genes
    selectionsModel()->loadCombinedSelectedGenes(m_combinedSelections);
}

const AnalysisDEA::deaStats AnalysisDEA::computeStatistics()
{
    deaStats stats;

    // iterate the list of combined reads to compute the DDA stats and populate the table
    for (const auto &readsValues : m_combinedSelections) {

        // check if values are outside threshold
        if (combinedSelectionThreholsd(readsValues)) {
            continue;
        }

        // get read values
        const int readsSelA = readsValues.readsA;
        const int readsSelB = readsValues.readsB;

        // compute overlapping counting values
        if (readsSelA == 0) {
            ++stats.countB;
        } else if (readsSelB == 0) {
            ++stats.countA;
        } else {
            ++stats.countAB;
        }

        // update lists of values that will be used in the scatter plot (use log values)
        stats.valuesSelectionA.push_back(std::log1p(readsSelA));
        stats.valuesSelectionB.push_back(std::log1p(readsSelB));
    }

    if (!m_combinedSelections.empty()) {
        stats.pearsonCorrelation = Math::pearson(stats.valuesSelectionA, stats.valuesSelectionB);
    }

    return stats;
}

void AnalysisDEA::updateStatisticsUI(const deaStats &stats)
{
    // TODO update scatter plot data

    // clear selection
    m_ui->tableView->clearSelection();
    // TODO clear selected point scatter

    // TODO update plot object (replot)

    // update UI fields for stats
    m_ui->numGenesSelectionA->setText(QString::number(stats.countA + stats.countAB));
    m_ui->numGenesSelectionB->setText(QString::number(stats.countB + stats.countAB));
    m_ui->correlation->setText(QString::number(stats.pearsonCorrelation));
    m_ui->overlappingGenes->setText(QString::number(stats.countAB));
    m_ui->genesOnlyA->setText(QString::number(stats.countA));
    m_ui->genesOnlyB->setText(QString::number(stats.countB));

    // update view
    update();
}

void AnalysisDEA::slotSetLowerThreshold(const int value)
{
    if (value != m_lowerThreshold) {
        m_lowerThreshold = value;
        updateStatisticsUI(computeStatistics());
    }
}

void AnalysisDEA::slotSetUpperThreshold(const int value)
{
    if (value != m_upperThreshold) {
        m_upperThreshold = value;
        updateStatisticsUI(computeStatistics());
    }
}

void AnalysisDEA::slotSaveToPDF()
{
    return;
    // TODO add most DEA genes and stats (use QPrinter)
    // TODO use PDF as output
    // TODO save plot to file
}

bool AnalysisDEA::combinedSelectionThreholsd(const AnalysisDEA::deaReads &deaReads) const
{
    // check if values are outside threshold
    return (((deaReads.readsA < m_lowerThreshold || deaReads.readsA > m_upperThreshold)
             && (deaReads.readsB < m_lowerThreshold || deaReads.readsB > m_upperThreshold)
             && deaReads.readsA > 0 && deaReads.readsB > 0));
}
