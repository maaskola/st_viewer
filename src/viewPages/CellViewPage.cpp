
/*
    Copyright (C) 2012  Spatial Transcriptomics AB,
    read LICENSE for licensing terms.
    Contact : Jose Fernandez Navarro <jose.fernandez.navarro@scilifelab.se>

*/

#include "CellViewPage.h"

#include <QDebug>

#include <QMetaObject>
#include <QMetaMethod>
#include <QString>
#include <QPrintDialog>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QPrinter>
#include <QColorDialog>
#include <QImageReader>
#include <QPainter>
#include <QWidgetAction>

#include "error/Error.h"

#include "network/DownloadManager.h"

#include "CellViewPageToolBar.h"

#include "io/GeneExporter.h"

#include "utils/Utils.h"

#include "dialogs/SelectionDialog.h"
#include "dialogs/CreateSelectionDialog.h"

#include "viewOpenGL/CellGLView.h"
#include "viewOpenGL/ImageTextureGL.h"
#include "viewOpenGL/GridRendererGL.h"
#include "viewOpenGL/HeatMapLegendGL.h"
#include "viewOpenGL/MiniMapGL.h"
#include "viewOpenGL/GeneRendererGL.h"

#include "ui_cellview.h"

CellViewPage::CellViewPage(QWidget *parent)
    : Page(parent),
      m_minimap(nullptr),
      m_legend(nullptr),
      m_gene_plotter(nullptr),
      m_image(nullptr),
      m_grid(nullptr),
      m_view(nullptr),
      m_colorDialogGrid(nullptr),
      m_toolBar(nullptr),
      ui(nullptr)
{
    onInit();
}

CellViewPage::~CellViewPage()
{
    delete ui;
    ui = nullptr;

    m_colorDialogGrid->deleteLater();
    m_colorDialogGrid = nullptr;

    m_toolBar->deleteLater();
    m_toolBar = nullptr;
}

void CellViewPage::onInit()
{
    //create UIobjects
    ui = new Ui::CellView();
    ui->setupUi(this);

    // color dialogs
    m_colorDialogGrid = new QColorDialog(Globals::DEFAULT_COLOR_GRID);
    //OSX native color dialog gives problems
    m_colorDialogGrid->setOption(QColorDialog::DontUseNativeDialog, true);

    //create tool bar and add it
    createToolBar();

    // init OpenGL graphical objects
    initGLView();

    //create connections
    createConnections();

    //create OpenGL connections
    createGLConnections();
}

void CellViewPage::onEnter()
{
    setWaiting(true);

    if (!loadData()) {
        //TODO do something here,
        //setWaiting(false);
        return;
    }

    DataProxy *dataProxy = DataProxy::getInstance();

    const auto dataset =
            dataProxy->getDatasetById(dataProxy->getSelectedDataset());
    Q_ASSERT(dataset);

    const auto imageAlignment =
            dataProxy->getImageAlignment(dataset->imageAlignmentId());
    Q_ASSERT(imageAlignment);

    const auto currentChip = dataProxy->getChip(imageAlignment->chipId());
    Q_ASSERT(currentChip);

    const QTransform alignment = imageAlignment->alignment();
    const qreal min = dataset->statisticsMin(); //1st quantile
    const qreal max = dataset->statisticsMax(); // 2nd quantile;
    const qreal pooledMin = dataset->statisticsPooledMin();
    const qreal pooledMax = dataset->statisticsPooledMax();

    const QRectF chip_rect = QRectF(
                QPointF(currentChip->x1(), currentChip->y1()),
                QPointF(currentChip->x2(), currentChip->y2())
                );
    const QRectF chip_border = QRectF(
                QPointF(currentChip->x1Border(), currentChip->y1Border()),
                QPointF(currentChip->x2Border(), currentChip->y2Border())
                );

    // clear view data
    m_view->clearData();

    // updade grid size and data
    m_grid->clearData();
    m_grid->setDimensions(chip_border, chip_rect);
    m_grid->generateData();
    m_grid->setTransform(alignment);

    // update gene size an data
    m_gene_plotter->clearData();
    m_gene_plotter->setDimensions(chip_border);
    m_gene_plotter->generateData();
    m_gene_plotter->setTransform(alignment);
    m_gene_plotter->setHitCount(min, max, pooledMin, pooledMax);

    // updated legend size and data
    m_legend->setBoundaries(min, max);

    // load cell tissue
    slotLoadCellFigure();

    // reset main variabless
    resetActionStates();

    //loading finished
    setWaiting(false);
}

void CellViewPage::onExit()
{
    ui->genesWidget->clearFocus();
    ui->selectionsWidget->clearFocus();
}

bool CellViewPage::loadData()
{
    DataProxy *dataProxy = DataProxy::getInstance();

    if (dataProxy->getSelectedDataset().isNull()) {
        showWarning("Cell View", "No dataset has been selected");
        return false;
    }

    const auto dataset =
            dataProxy->getDatasetById(dataProxy->getSelectedDataset());

    if (dataset.isNull()) {
        showError("Cell View Error", "The current selected dataset is not valid.");
        return false;
    }

    //load the image alignment first
    {
        async::DataRequest request =
                dataProxy->loadImageAlignmentById(dataset->imageAlignmentId());
        if (request.return_code() == async::DataRequest::CodeError
                || request.return_code() == async::DataRequest::CodeAbort) {
            //TODO use text in request.getErrors()
            showError("Data loading Error", "Error loading the image alignment.");
            return false;
        }
    }

    //get image alignmet object
    const auto ImageAlignment =
            dataProxy->getImageAlignment(dataset->imageAlignmentId());
    Q_ASSERT(!ImageAlignment.isNull());

    //load cell tissue blue
    {
        async::DataRequest request =
                dataProxy->loadCellTissueByName(ImageAlignment->figureBlue());
        if (request.return_code() == async::DataRequest::CodeError
                || request.return_code() == async::DataRequest::CodeAbort) {
            //TODO use text in request.getErrors()
            showError("Data loading Error", "Error loading the cell tissue image(red).");
            return false;
        }
    }
    //load cell tissue red
    {
        async::DataRequest request =
                dataProxy->loadCellTissueByName(ImageAlignment->figureRed());
        if (request.return_code() == async::DataRequest::CodeError
                || request.return_code() == async::DataRequest::CodeAbort) {
            //TODO use text in request.getErrors()
            showError("Data loading Error", "Error loading the cell tissue image(blue).");
            return false;
        }
    }
    //load features
    {
        async::DataRequest request =
                dataProxy->loadFeatureByDatasetId(dataset->id());
        if (request.return_code() == async::DataRequest::CodeError
                || request.return_code() == async::DataRequest::CodeAbort) {
            //TODO use text in request.getErrors()
            showError("Data loading Error", "Error loading the features.");
            return false;
        }
    }
    //load genes
    {
        async::DataRequest request =
                dataProxy->loadGenesByDatasetId(dataset->id());
        if (request.return_code() == async::DataRequest::CodeError
                || request.return_code() == async::DataRequest::CodeAbort) {
            //TODO use text in request.getErrors()
            showError("Data loading Error", "Error loading the genes.");
            return false;
        }
    }
    //load chip
    {
        async::DataRequest request =
                dataProxy->loadChipById(ImageAlignment->chipId());
        if (request.return_code() == async::DataRequest::CodeError
                || request.return_code() == async::DataRequest::CodeAbort) {
            //TODO use text in request.getErrors()
            showError("Data loading Error", "Error loading the chip array.");
            return false;
        }
    }

    //succes downloading the data
    return true;
}

void CellViewPage::createConnections()
{
    // go back signal
    connect(m_toolBar->m_actionNavigate_goBack,
            SIGNAL(triggered(bool)), this, SIGNAL(moveToPreviousPage()));
    // go next signal
    connect(m_toolBar->m_actionNavigate_goNext,
            SIGNAL(triggered(bool)), this, SIGNAL(moveToNextPage()));

    // cell tissue
    connect(m_toolBar->m_actionShow_cellTissueBlue,
            SIGNAL(triggered(bool)), this, SLOT(slotLoadCellFigure()));
    connect(m_toolBar->m_actionShow_cellTissueRed,
            SIGNAL(triggered(bool)), this, SLOT(slotLoadCellFigure()));

    // graphic view signals
    connect(m_toolBar->m_actionZoom_zoomIn,
            SIGNAL(triggered(bool)), m_view.data(), SLOT(zoomIn()));
    connect(m_toolBar->m_actionZoom_zoomOut,
            SIGNAL(triggered(bool)), m_view.data(), SLOT(zoomOut()));
    connect(m_toolBar.data(), SIGNAL(rotateView(qreal)), m_view.data(), SLOT(rotate(qreal)));

    // print canvas
    connect(m_toolBar->m_actionSave_save,
            SIGNAL(triggered(bool)), this, SLOT(slotSaveImage()));
    connect(m_toolBar->m_actionSave_print,
            SIGNAL(triggered(bool)), this, SLOT(slotPrintImage()));

    // selection mode
    connect(m_toolBar->m_actionActivateSelectionMode,
            SIGNAL(triggered(bool)), m_view.data(), SLOT(setSelectionMode(bool)));
    connect(m_toolBar->m_actionSelection_showSelectionDialog,
            SIGNAL(triggered(bool)), this, SLOT(slotSelectByRegExp()));

    //color selectors
    connect(m_toolBar->m_actionColor_selectColorGrid, &QAction::triggered,
            [=]{ m_colorDialogGrid->show(); });
}


void CellViewPage::resetActionStates()
{
    // resets genes color and visible to default (must be done first)
    DataProxy *dataProxy = DataProxy::getInstance();
    auto &geneList = dataProxy->getGeneList(dataProxy->getSelectedDataset());
    for (auto gene : geneList) {
        gene->selected(false);
        gene->color(Globals::DEFAULT_COLOR_GENE);
    }

    // load data for gene model, also resets it
    ui->genesWidget->slotLoadModel();

    // resets gene selection model
    ui->selectionsWidget->slotLoadModel(GeneSelection::selectedItemsList());

    // reset color dialogs
    m_colorDialogGrid->setCurrentColor(Globals::DEFAULT_COLOR_GRID);

    // reset cell image to show
    m_image->setVisible(true);
    m_image->setAnchor(Globals::DEFAULT_ANCHOR_IMAGE);

    // reset gene grid to not show
    m_grid->setVisible(false);
    m_grid->setAnchor(Globals::DEFAULT_ANCHOR_GRID);

    // reset gene plotter to visible
    m_gene_plotter->setVisible(true);
    m_gene_plotter->setAnchor(Globals::DEFAULT_ANCHOR_GENE);

    // reset minimap to visible true
    m_minimap->setVisible(true);
    m_minimap->setAnchor(Globals::DEFAULT_ANCHOR_MINIMAP);

    // reset legend to visible true
    m_legend->setVisible(false);
    m_legend->setAnchor(Globals::DEFAULT_ANCHOR_LEGEND);

    // reset tool bar actions
    m_toolBar->resetActions();

    // restrict interface
    DataProxy::UserPtr current_user = dataProxy->getUser();
    m_toolBar->m_actionGroup_cellTissue->setVisible((current_user->role() == Globals::ROLE_CM));
}

void CellViewPage::createToolBar()
{
    m_toolBar = new CellViewPageToolBar();
    // add tool bar to the layout
    ui->pageLayout->insertWidget(0, m_toolBar.data());
}

void CellViewPage::initGLView()
{
    //ui->area contains the openGL window
    m_view = new CellGLView();
    ui->area->initializeView(m_view);

    // Setting stretch factors in the QSplitter to make the opengl window occupy more space
    ui->gridLayout->setStretchFactor(0, 0);
    ui->gridLayout->setStretchFactor(1, 8);

    // image texture graphical object
    m_image = new ImageTextureGL(this);
    m_image->setAnchor(Globals::DEFAULT_ANCHOR_IMAGE);
    m_view->addRenderingNode(m_image.data());

    // grid graphical object
    m_grid = new GridRendererGL(this);
    m_grid->setAnchor(Globals::DEFAULT_ANCHOR_GRID);
    m_view->addRenderingNode(m_grid.data());

    // gene plotter component
    m_gene_plotter = new GeneRendererGL(this);
    m_gene_plotter->setAnchor(Globals::DEFAULT_ANCHOR_GENE);
    m_view->addRenderingNode(m_gene_plotter.data());

    // heatmap component
    m_legend = new HeatMapLegendGL(this);
    m_legend->setAnchor(Globals::DEFAULT_ANCHOR_LEGEND);
    m_view->addRenderingNode(m_legend.data());

    // minimap component
    m_minimap = new MiniMapGL(this);
    m_minimap->setAnchor(Globals::DEFAULT_ANCHOR_MINIMAP);
    m_view->addRenderingNode(m_minimap.data());
    // minimap needs to be notified when the canvas is resized and when the image
    // is zoomed or moved
    connect(m_minimap.data(), SIGNAL(signalCenterOn(QPointF)),
            m_view.data(), SLOT(centerOn(QPointF)));
    connect(m_view.data(), SIGNAL(signalSceneUpdated(QRectF)),
            m_minimap.data(), SLOT(setScene(QRectF)));
    connect(m_view.data(), SIGNAL(signalViewPortUpdated(QRectF)),
            m_minimap.data(), SLOT(setViewPort(QRectF)));
    connect(m_view.data(), SIGNAL(signalSceneTransformationsUpdated(const QTransform)),
            m_minimap.data(), SLOT(setParentSceneTransformations(const QTransform)));
}

void CellViewPage::createGLConnections()
{
    //connect gene list model to gene plotter
    connect(ui->genesWidget, SIGNAL(signalSelectionChanged(DataProxy::GeneList)),
            m_gene_plotter.data(),
            SLOT(updateSelection(DataProxy::GeneList)));
    connect(ui->genesWidget, SIGNAL(signalColorChanged(DataProxy::GeneList)),
            m_gene_plotter.data(),
            SLOT(updateColor(DataProxy::GeneList)));

    //connect gene selection signals from selectionsWidget
    connect(ui->selectionsWidget, SIGNAL(signalClearSelection()),
            m_gene_plotter.data(), SLOT(clearSelection()));
    connect(ui->selectionsWidget, SIGNAL(signalExportSelection()), this, SLOT(slotExportSelection()));
    connect(ui->selectionsWidget, SIGNAL(signalSaveSelection()), this, SLOT(slotSaveSelection()));

    //connect gene plotter to gene selection model
    connect(m_gene_plotter.data(), SIGNAL(selectionUpdated()),
            this, SLOT(slotSelectionUpdated()));

    //threshold slider signal
    connect(m_toolBar.data(), SIGNAL(thresholdLowerValueChanged(int)),
            m_gene_plotter.data(), SLOT(setLowerLimit(int)));
    connect(m_toolBar.data(), SIGNAL(thresholdUpperValueChanged(int)),
            m_gene_plotter.data(), SLOT(setUpperLimit(int)));
    
    //gene attributes signals
    connect(m_toolBar.data(), SIGNAL(intensityValueChanged(qreal)),
            m_gene_plotter.data(), SLOT(setIntensity(qreal)));
    connect(m_toolBar.data(), SIGNAL(sizeValueChanged(qreal)),
            m_gene_plotter.data(), SLOT(setSize(qreal)));
    connect(m_toolBar.data(), SIGNAL(shapeIndexChanged(Globals::GeneShape)),
            m_gene_plotter.data(), SLOT(setShape(Globals::GeneShape)));
    connect(m_toolBar.data(), SIGNAL(shineValueChanged(qreal)),
            m_gene_plotter.data(), SLOT(setShine(qreal)));

    //show/not genes signal
    connect(m_toolBar->m_actionShow_showGenes,
            SIGNAL(triggered(bool)), m_gene_plotter.data(), SLOT(setVisible(bool)));

    //visual mode signal
    connect(m_toolBar->m_actionGroup_toggleVisualMode, SIGNAL(triggered(QAction*)), this,
            SLOT(slotSetGeneVisualMode(QAction*)));

    // grid signals
    connect(m_colorDialogGrid.data(), SIGNAL(colorSelected(const QColor&)),
            m_grid.data(), SLOT(setColor(const QColor&)));
    connect(m_toolBar->m_actionShow_showGrid, SIGNAL(triggered(bool)),
            m_grid.data(), SLOT(setVisible(bool)));

    // cell tissue canvas
    connect(m_toolBar->m_actionShow_showCellTissue, SIGNAL(triggered(bool)),
            m_image.data(), SLOT(setVisible(bool)));
    connect(m_toolBar.data(), SIGNAL(brightnessValueChanged(qreal)),
            m_image.data(), SLOT(setIntensity(qreal)));

    // legend signals
    connect(m_toolBar->m_actionShow_showLegend, SIGNAL(toggled(bool)),
            m_legend.data(), SLOT(setVisible(bool)));
    connect(m_toolBar->m_actionGroup_toggleLegendPosition,
            SIGNAL(triggered(QAction*)), this,
            SLOT(slotSetLegendAnchor(QAction*)));

    // minimap signals
    connect(m_toolBar->m_actionShow_showMiniMap, SIGNAL(toggled(bool)),
            m_minimap.data(), SLOT(setVisible(bool)));
    connect(m_toolBar->m_actionGroup_toggleMinimapPosition,
            SIGNAL(triggered(QAction*)), this,
            SLOT(slotSetMiniMapAnchor(QAction*)));

    // connect threshold slider to the heatmap
    connect(m_toolBar.data(), SIGNAL(thresholdLowerValueChanged(int)),
            m_legend.data(), SLOT(setLowerLimit(int)));
    connect(m_toolBar.data(), SIGNAL(thresholdUpperValueChanged(int)),
            m_legend.data(), SLOT(setUpperLimit(int)));
}

void CellViewPage::slotLoadCellFigure()
{
    DataProxy *dataProxy = DataProxy::getInstance();
    const auto current_user = dataProxy->getUser();
    Q_ASSERT(current_user);
    const auto dataset = dataProxy->getDatasetById(dataProxy->getSelectedDataset());
    Q_ASSERT(dataset);
    const auto imageAlignment = dataProxy->getImageAlignment(dataset->imageAlignmentId());
    Q_ASSERT(imageAlignment);

    const bool forceRedFigure = QObject::sender() == m_toolBar->m_actionShow_cellTissueRed;
    const bool forceBlueFigure = QObject::sender() == m_toolBar->m_actionShow_cellTissueBlue;
    const bool defaultRedFigure = current_user->role() == Globals::ROLE_CM;
    const bool loadRedFigure = (defaultRedFigure || forceRedFigure) && !forceBlueFigure;

    const QString figureid = (loadRedFigure) ? imageAlignment->figureRed() : imageAlignment->figureBlue();
    auto device = dataProxy->getFigure(figureid);

    //read image (TODO check file is present or corrupted)
    QImageReader reader(device.get());
    const QImage image = reader.read();

    //deallocate device
    device->close();
    device->deleteLater();

    // add image to the texture image holder
    m_image->createTexture(image);
    // set the scene size in the view to the image bounding box
    m_view->setScene(image.rect());
    // set the image in the gene plotter
    m_gene_plotter->setImage(image);

    //update checkboxes
    m_toolBar->m_actionShow_cellTissueBlue->setChecked(!loadRedFigure);
    m_toolBar->m_actionShow_cellTissueRed->setChecked(loadRedFigure);
}

void CellViewPage::slotPrintImage()
{
    QPrinter printer;
    printer.setOrientation(QPrinter::Landscape);
    QScopedPointer<QPrintDialog> dialog(new QPrintDialog(&printer, this));
    if (dialog->exec() != QDialog::Accepted) {
        return;
    }
    QPainter painter(&printer);
    QRect rect = painter.viewport();
    QImage image = m_view->grabPixmapGL();
    QSize size = image.size();
    size.scale(rect.size(), Qt::KeepAspectRatio);
    painter.setViewport(rect);
    painter.setWindow(image.rect());
    painter.drawImage(0, 0, image);
}

void CellViewPage::slotSaveImage()
{
    QString filename =
                       QFileDialog::getSaveFileName(this, tr("Save Image"), QDir::homePath(),
                       QString("%1;;%2").
                       arg(tr("JPEG Image Files (*.jpg *.jpeg)")).
                       arg(tr("PNG Image Files (*.png)")));
    // early out
    if (filename.isEmpty()) {
        return;
    }
    // append default extension
    QRegExp regex("^.*\\.(jpg|jpeg|png)$", Qt::CaseInsensitive);
    if (!regex.exactMatch(filename)) {
        filename.append(".jpg");
    }
    const int quality = 100; //quality format (100 max, 0 min, -1 default)
    const QString format = filename.split(".", QString::SkipEmptyParts).at(1); //get the file extension
    QImage image = m_view->grabPixmapGL();
    if (!image.save(filename, format.toStdString().c_str(), quality)) {
        showError("Save Image", tr("Error saving image."));
    }
}

void CellViewPage::slotExportSelection()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Export File"), QDir::homePath(),
                       QString("%1").
                       arg(tr("Text Files (*.txt)")));
    // early out
    if (filename.isEmpty()) {
        return;
    }
    // append default extension
    QRegExp regex("^.*\\.(txt)$", Qt::CaseInsensitive);
    if (!regex.exactMatch(filename)) {
        filename.append(".txt");
    }
    // get selected features
    const auto& geneSelection = m_gene_plotter->getSelectedIItems();
    //create file
    QFile textFile(filename);
    //export selection
    if (textFile.open(QFile::WriteOnly | QFile::Truncate)) {
        GeneExporter exporter = GeneExporter(GeneExporter::SimpleFull,
                                             GeneExporter::TabDelimited);
        exporter.exportItem(textFile, geneSelection);
    }

    textFile.close();
}

void CellViewPage::slotSetGeneVisualMode(QAction *action)
{
    const QVariant variant = action->property("mode");
    if (variant.canConvert(QVariant::Int)) {
        const Globals::GeneVisualMode mode = static_cast<Globals::GeneVisualMode>(variant.toInt());
        m_gene_plotter->setVisualMode(mode);
    } else {
        Q_ASSERT("[CellViewPage] Undefined gene visual mode!");
    }
}

void CellViewPage::slotSetMiniMapAnchor(QAction *action)
{
    const QVariant variant = action->property("mode");
    if (variant.canConvert(QVariant::Int)) {
        const Globals::Anchor mode = static_cast<Globals::Anchor>(variant.toInt());
        m_minimap->setAnchor(mode);
    } else {
        Q_ASSERT("[CellViewPage] Undefined minimap anchor!");
    }
}

void CellViewPage::slotSetLegendAnchor(QAction *action)
{
    const QVariant variant = action->property("mode");
    if (variant.canConvert(QVariant::Int)) {
        const Globals::Anchor mode = static_cast<Globals::Anchor>(variant.toInt());
        m_legend->setAnchor(mode);
    } else {
        Q_ASSERT("[CellViewPage] Undefined legend anchor!");
    }
}

void CellViewPage::slotSelectByRegExp()
{
    const DataProxy::GeneList& geneList = SelectionDialog::selectGenes(this);
    m_gene_plotter->selectGenes(geneList);
}

void CellViewPage::slotSelectionUpdated()
{
    // get selected features
    const auto& geneSelection = m_gene_plotter->getSelectedIItems();
    ui->selectionsWidget->slotLoadModel(geneSelection);
}

void CellViewPage::slotSaveSelection()
{
    QScopedPointer<CreateSelectionDialog> createSelection(new CreateSelectionDialog(this,
                                                                          Qt::CustomizeWindowHint | Qt::WindowTitleHint));
    if (createSelection->exec() == CreateSelectionDialog::Accepted) {
        DataProxy *dataProxy = DataProxy::getInstance();

        // get selected features
        const auto& geneSelection = m_gene_plotter->getSelectedIItems();

        //create the selection object
        GeneSelection selection;
        selection.name(createSelection->getName());
        selection.comment(createSelection->getComment());
        selection.type("Bounding box");
        //add datasets
        DataProxy::DatasetPtr dataset = dataProxy->getDatasetById(dataProxy->getSelectedDataset());
        Q_ASSERT(!dataset.isNull());
        selection.datasetId(dataset->id());
        //add selected genes
        selection.selectedItems(geneSelection);

        //save the selection object
        async::DataRequest request = dataProxy->addGeneSelection(selection);
        if (request.return_code() == async::DataRequest::CodeError
                || request.return_code() == async::DataRequest::CodeAbort) {
            //TODO use the test in request.getErrors()
            showError("Create Gene Selection", "Error saving the gene selection");
        }
        else {
            qDebug() << "Selection object saved succesfully";
        }

    }
}
