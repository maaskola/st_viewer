/*
    Copyright (C) 2012  Spatial Transcriptomics AB,
    read LICENSE for licensing terms.
    Contact : Jose Fernandez Navarro <jose.fernandez.navarro@scilifelab.se>

*/

#include "AnalysisDEA.h"

#include "ui_ddaWidget.h"

#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>

#include <cmath>
#include "math/Common.h"

#include "qcustomplot/qcustomplot.h"
#include "model/GeneSelectionDEAItemModel.h"

static const QColor BORDER = QColor(0, 155, 60);

AnalysisDEA::AnalysisDEA(const GeneSelection& selObjectA,
                         const GeneSelection& selObjectB,
                         QWidget *parent, Qt::WindowFlags f) :
    QDialog(parent, f),
    m_ui(new Ui::ddaWidget),
    m_customPlot(nullptr),
    m_lowerThreshold(0),
    m_upperThreshold(1),
    m_lowerTPMsThreshold(0),
    m_upperTPMsThreshold(1)
{
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    setModal(true);

    m_ui->setupUi(this);

    m_ui->tableView->setStyleSheet("QTableView {alternate-background-color: rgb(245,245,245); "
                                   "background-color: transparent; "
                                   "selection-background-color: rgb(215,215,215); "
                                   "selection-color: rgb(0,155,60); "
                                   "gridline-color: rgb(240,240,240);"
                                   "border: 1px solid rgb(240,240,240);} "
                                   "QTableView::indicator:unchecked {image: url(:/images/unchecked-box.png);} "
                                   "QTableView::indicator:checked {image: url(:/images/checked-box.png);} "
                                   "QTableView::indicator {padding-left: 10px; "
                                                          "width: 15px; "
                                                          "height: 15px; "
                                                          "background-color: transparent;} "
                                    "QHeaderView::section {height: 35px; "
                                                          "padding-left: 4px; "
                                                          "padding-right: 2px; "
                                                          "spacing: 5px;"
                                                          "background-color: rgb(230,230,230); "
                                                          "border: 1px solid rgb(240,240,240);} "
                                    "QTableCornerButton::section {background-color: transparent;} ");
    // creating plotting object
    m_customPlot = new QCustomPlot(m_ui->plotWidget);
    Q_ASSERT(m_customPlot != nullptr);

    // add a scatter plot graph
    m_customPlot->addGraph();
    m_customPlot->graph(0)->setScatterStyle(
                QCPScatterStyle(QCPScatterStyle::ssCircle, QPen(BORDER), Qt::white, 5));
    m_customPlot->graph(0)->setAntialiasedScatters(true);
    m_customPlot->graph(0)->setLineStyle(QCPGraph::lsNone);
    m_customPlot->graph(0)->setName(tr("Correlation Scatter Plot"));
    m_customPlot->graph(0)->rescaleAxes(true);

    // sets the legend and attributes
    m_customPlot->legend->setVisible(false);
    m_customPlot->xAxis->setScaleType(QCPAxis::stLogarithmic);
    m_customPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    m_customPlot->xAxis->setTicks(true);
    m_customPlot->yAxis->setTicks(true);
    // make top right axes clones of bottom left axes. Looks prettier:
    m_customPlot->axisRect()->setupFullAxesBox();
    // plot and add mouse interaction (fixed min size)
    m_customPlot->setMinimumSize(500, 400);
    m_customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    // populate the gene to read pairs containers
    // computeGeneToReads will update the max thresholds (to initialize slider)
    computeGeneToReads(selObjectA, selObjectB);

    // update table
    selectionsModel()->loadCombinedSelectedGenes(m_combinedSelections);

    //initialize threshold sliders (minimum must be zero to allow to discard non-expressed genes)
    m_ui->tpmsThreshold->setMinimumValue(0);
    m_ui->tpmsThreshold->setMaximumValue(m_upperTPMsThreshold);
    m_ui->tpmsThreshold->slotSetLowerValue(m_lowerTPMsThreshold);
    m_ui->tpmsThreshold->slotSetUpperValue(m_upperTPMsThreshold);
    m_ui->tpmsThreshold->setTickInterval(1);
    m_ui->readsThreshold->setMinimumValue(0);
    m_ui->readsThreshold->setMaximumValue(m_upperThreshold);
    m_ui->readsThreshold->slotSetLowerValue(m_lowerThreshold);
    m_ui->readsThreshold->slotSetUpperValue(m_upperThreshold);
    m_ui->readsThreshold->setTickInterval(1);

    // update name fields in the UI
    m_ui->selectionA->setText(selObjectA.name());
    m_ui->selectionB->setText(selObjectB.name());
    m_customPlot->xAxis->setLabel(tr("Selection A (TPM + 1)"));
    m_customPlot->yAxis->setLabel(tr("Selection B (TPM + 1)"));

    // compute statistics
    const deaStats &stats = computeStatistics();

    // visualize statistics
    updateStatisticsUI(stats);

    // make connections
    connect(m_ui->buttonBox, SIGNAL(rejected()), this, SLOT(close()));
    connect(m_ui->buttonBox->button(QDialogButtonBox::Save),
            SIGNAL(clicked()), this, SLOT(slotSaveToPDF()));
    connect(m_ui->readsThreshold, SIGNAL(signalLowerValueChanged(int)),
            this, SLOT(slotSetLowerThreshold(int)));
    connect(m_ui->readsThreshold, SIGNAL(signalUpperValueChanged(int)),
            this, SLOT(slotSetUpperThreshold(int)));
    connect(m_ui->tpmsThreshold, SIGNAL(signalLowerValueChanged(int)),
            this, SLOT(slotSetLowerTPMsThreshold(int)));
    connect(m_ui->tpmsThreshold, SIGNAL(signalUpperValueChanged(int)),
            this, SLOT(slotSetUpperTPMsThreshold(int)));
    connect(m_ui->geneSearch, SIGNAL(textChanged(QString)), selectionsProxyModel(),
            SLOT(setFilterFixedString(QString)));;
    connect(m_ui->tableView, SIGNAL(clicked(QModelIndex)),
            this, SLOT(slotSelectionSelected(QModelIndex)));
}

AnalysisDEA::~AnalysisDEA()
{

}


QSortFilterProxyModel *AnalysisDEA::selectionsProxyModel()
{
    QSortFilterProxyModel* selectionsProxyModel =
            qobject_cast<QSortFilterProxyModel*>(m_ui->tableView->model());
    Q_ASSERT(selectionsProxyModel);
    return selectionsProxyModel;
}

GeneSelectionDEAItemModel *AnalysisDEA::selectionsModel()
{
    GeneSelectionDEAItemModel *model =
            qobject_cast<GeneSelectionDEAItemModel*>(selectionsProxyModel()->sourceModel());
    Q_ASSERT(model);
    return model;
}

void AnalysisDEA::slotSelectionSelected(QModelIndex index)
{
    Q_UNUSED(index);
    /*const auto selected = m_ui->tableView->geneTableItemSelection();
    const auto currentSelection = selectionsModel()->getSelections(selected);

    //TODO do some validation here

    //TODO validate coords are into threshold

    const qreal x = m_customPlot->xAxis->coordToPixel(currentSelection.first().normalizedReadsA);
    const qreal y = m_customPlot->xAxis->coordToPixel(currentSelection.first().normalizedReadsB);

    // get the plotting data
    //const QCPDataMap *dataMap = m_customPlot->selectedGraphs().first()->data();

    // add the phase tracer (red circle) which sticks to the graph data (and gets updated in bracketDataSlot by timer event):
    QCPItemTracer *phaseTracer = new QCPItemTracer(m_customPlot);
    m_customPlot->addItem(phaseTracer);
    phaseTracer->setGraph(m_customPlot->graph(0));
    phaseTracer->setGraphKey(currentSelection.first().normalizedReadsA);
    phaseTracer->setInterpolating(true);
    phaseTracer->setStyle(QCPItemTracer::tsCircle);
    phaseTracer->setPen(QPen(Qt::red));
    phaseTracer->setBrush(Qt::red);
    phaseTracer->setSize(7);

    // mark selected position
    QCPItemLine *arrow = new QCPItemLine(m_customPlot);
    m_customPlot->addItem(arrow);
    arrow->start->setCoords(x - 1, y);
    arrow->end->setCoords(x, y);
    //qDebug() << "Adding item with cordinates " << x << " " << y << " to the graph";
    arrow->setHead(QCPLineEnding::esSpikeArrow);

    update();*/
}

void AnalysisDEA::computeGeneToReads(const GeneSelection& selObjectA,
                                     const GeneSelection& selObjectB)
{
    // reset thresholds
    m_lowerThreshold = std::numeric_limits<int>::max();
    m_upperThreshold = std::numeric_limits<int>::min();
    m_lowerTPMsThreshold = std::numeric_limits<int>::max();
    m_upperTPMsThreshold = std::numeric_limits<int>::min();

    // get the list of selected items
    const auto &selA = selObjectA.selectedItems();
    const auto &selB = selObjectB.selectedItems();

    // get the size of the biggest list
    const int selectionAsize = selA.size();
    const int selectionBsize = selB.size();
    const int biggestSize = qMax(selectionAsize, selectionBsize);

    //take into account that some genes might be present in only one selection
    //therefore, we create a hash table (key gene name - value a pairt with value in selection A
    //and value in selection B) to later know what genes are present in which set
    //depending of the value of the hash (0.0 no present)
    QHash<QString, deaReads> geneToReadsMap;
    for (int i = 0; i < biggestSize; ++i) {

        if (selectionAsize > i) {
            const auto& selectionA = selA.at(i);
            geneToReadsMap[selectionA.name].gene = selectionA.name;
            geneToReadsMap[selectionA.name].readsA = selectionA.reads;
            geneToReadsMap[selectionA.name].normalizedReadsA = selectionA.normalizedReads;
            m_upperThreshold = std::max(selectionA.reads, m_upperThreshold);
            m_lowerThreshold = std::min(selectionA.reads, m_lowerThreshold);
            m_upperTPMsThreshold = std::max(selectionA.normalizedReads, m_upperTPMsThreshold);
            m_lowerTPMsThreshold = std::min(selectionA.normalizedReads, m_lowerTPMsThreshold);
        }

        if (selectionBsize > i) {
            const auto& selectionB = selB.at(i);
            geneToReadsMap[selectionB.name].gene = selectionB.name;
            geneToReadsMap[selectionB.name].readsB = selectionB.reads;
            geneToReadsMap[selectionB.name].normalizedReadsB = selectionB.normalizedReads;
            m_upperThreshold = std::max(selectionB.reads, m_upperThreshold);
            m_lowerThreshold = std::min(selectionB.reads, m_lowerThreshold);
            m_upperTPMsThreshold = std::max(selectionB.normalizedReads, m_upperTPMsThreshold);
            m_lowerTPMsThreshold = std::min(selectionB.normalizedReads, m_lowerTPMsThreshold);
        }
    }

    m_combinedSelections = geneToReadsMap.values();
}

const AnalysisDEA::deaStats AnalysisDEA::computeStatistics()
{
    deaStats stats;

    // some temp containers
    QVector<qreal> loggedValuesSelectionA;
    QVector<qreal> loggedValuesSelectionB;

    //iterate the list of combined reads to compute the DDA stats and populate the table
    foreach (deaReads readsValues, m_combinedSelections) {

        // get read values
        const int readsSelA = readsValues.readsA;
        const int readsSelB = readsValues.readsB;
        const qreal normReadsSelA = readsValues.normalizedReadsA;
        const qreal normReadsSelB = readsValues.normalizedReadsB;

        //check if values are outside threshold
        if ( ((readsSelA < m_lowerThreshold || readsSelA > m_upperThreshold)
             && (readsSelB < m_lowerThreshold || readsSelB > m_upperThreshold)
             && readsSelA > 0 && readsSelB > 0)
            ||
            ((normReadsSelA < m_lowerTPMsThreshold || normReadsSelA > m_upperTPMsThreshold)
             && (normReadsSelB < m_lowerTPMsThreshold || normReadsSelB > m_upperTPMsThreshold)
             && normReadsSelA > 1 && normReadsSelB > 1) ) {
            continue;
        }

        // compute overlapping counting values
        if (readsSelA == 0) {
            ++stats.countB;
        } else if (readsSelB == 0) {
            ++stats.countA;
        } else {
            ++stats.countAB;
        }

        // update lists of values that will be used in the scatter plot
        stats.valuesSelectionA.push_back(normReadsSelA);
        stats.valuesSelectionB.push_back(normReadsSelB);

        // update temp variables to compute stats
        loggedValuesSelectionA.push_back(std::log(normReadsSelA));
        loggedValuesSelectionB.push_back(std::log(normReadsSelB));
    }

    if (!m_combinedSelections.empty()) {
        stats.pearsonCorrelation = STMath::pearson(loggedValuesSelectionA, loggedValuesSelectionB);
    }

    return stats;
}

void AnalysisDEA::updateStatisticsUI(const deaStats &stats)
{
    //update plot data
    m_customPlot->graph(0)->setData(stats.valuesSelectionA, stats.valuesSelectionB);
    m_customPlot->graph(0)->rescaleAxes();
    m_customPlot->replot();

    //update UI fields for stats
    m_ui->numGenesSelectionA->setText(QString::number(stats.countA + stats.countAB));
    m_ui->numGenesSelectionB->setText(QString::number(stats.countB + stats.countAB));
    m_ui->correlation->setText(QString::number(stats.pearsonCorrelation));
    m_ui->overlappingGenes->setText(QString::number(stats.countAB));
    m_ui->genesOnlyA->setText(QString::number(stats.countA));
    m_ui->genesOnlyB->setText(QString::number(stats.countB));

    //update view
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

void AnalysisDEA::slotSetLowerTPMsThreshold(const int value)
{
    if (value != m_lowerTPMsThreshold) {
        m_lowerTPMsThreshold = value;
        updateStatisticsUI(computeStatistics());
    }
}

void AnalysisDEA::slotSetUpperTPMsThreshold(const int value)
{
    if (value != m_upperTPMsThreshold) {
        m_upperTPMsThreshold = value;
        updateStatisticsUI(computeStatistics());
    }
}

void AnalysisDEA::slotSaveToPDF()
{
    QString filename =
            QFileDialog::getSaveFileName(this, tr("Export File"), QDir::homePath(),
                                         QString("%1").arg(tr("PNG Files (*.png)")));
    // early out
    if (filename.isEmpty()) {
        return;
    }

    //TODO add most DEA genes and stats (use QPrinter)
    //TODO use PDF as output
    const bool saveOk = m_customPlot->savePng(filename, 800, 800, 1.0, 100);

    if (!saveOk) {
        QMessageBox::critical(this, tr("Save DEA"), tr("Error saving DEA to a file"));
    } else {
        QMessageBox::information(this, tr("Save DEA"), tr("DEA was saved successfully"));
    }
}
