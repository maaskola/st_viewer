/*
    Copyright (C) 2012  Spatial Transcriptomics AB,
    read LICENSE for licensing terms.
    Contact : Jose Fernandez Navarro <jose.fernandez.navarro@scilifelab.se>

*/

#include "SelectionsWidget.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QSortFilterProxyModel>

#include "viewTables/GeneSelectionTableView.h"
#include "model/GeneSelectionItemModel.h"

SelectionsWidget::SelectionsWidget(QWidget *parent) :
    QWidget(parent)
{

    QVBoxLayout *selectionLayout = new QVBoxLayout();
    QHBoxLayout *selectionBottonsLayout = new QHBoxLayout();

    m_saveSelection = new QPushButton(this);
    m_saveSelection->setMinimumSize(QSize(25, 25));
    m_saveSelection->setIcon(QIcon(QStringLiteral(":/images/file_export.png")));
    selectionBottonsLayout->addWidget(m_saveSelection);

    m_exportSelection = new QPushButton(this);
    m_exportSelection->setMinimumSize(QSize(25, 25));
    m_exportSelection->setIcon(QIcon(QStringLiteral(":/images/export.png")));
    selectionBottonsLayout->addWidget(m_exportSelection);

    m_clearSelection = new QPushButton(this);
    m_clearSelection->setMinimumSize(QSize(25, 25));
    m_clearSelection->setIcon(QIcon(QStringLiteral(":/images/clear2.png")));
    selectionBottonsLayout->addWidget(m_clearSelection);

    m_geneSelectionFilterLineEdit = new QLineEdit(this);
    m_geneSelectionFilterLineEdit->setMinimumSize(QSize(50, 0));
    m_geneSelectionFilterLineEdit->setClearButtonEnabled(true);
    selectionBottonsLayout->addWidget(m_geneSelectionFilterLineEdit);

    selectionLayout->addLayout(selectionBottonsLayout);

    m_selections_tableview = new GeneSelectionTableView(this);
    m_selections_tableview->setMinimumSize(QSize(100, 0));
    selectionLayout->addWidget(m_selections_tableview);

    setLayout(selectionLayout);

    //connections
    connect(m_geneSelectionFilterLineEdit, SIGNAL(textChanged(QString)), m_selections_tableview,
            SLOT(setGeneNameFilter(QString)));
    // export selection
    connect(m_exportSelection, SIGNAL(clicked(bool)), this, SIGNAL(signalExportSelection()));
    // save selection
    connect(m_saveSelection, SIGNAL(clicked(bool)), this, SIGNAL(signalSaveSelection()));
    // selection actions
    connect(m_clearSelection, SIGNAL(clicked(bool)), this, SIGNAL(signalClearSelection()));
}

SelectionsWidget::~SelectionsWidget()
{

}

void SelectionsWidget::slotLoadModel(const GeneSelection::selectedItemsList &geneList)
{
    getModel()->loadSelectedGenes(geneList);
}

void SelectionsWidget::slotClearModel()
{
    getModel()->reset();
}

GeneSelectionItemModel *SelectionsWidget::getModel()
{
    GeneSelectionItemModel *selectionModel =
        qobject_cast<GeneSelectionItemModel*>(getProxyModel()->sourceModel());
    Q_ASSERT(selectionModel);
    return selectionModel;
}

QSortFilterProxyModel *SelectionsWidget::getProxyModel()
{
    QSortFilterProxyModel *selectionsProxyModel =
        qobject_cast<QSortFilterProxyModel*>(m_selections_tableview->model());
    Q_ASSERT(selectionsProxyModel);
    return selectionsProxyModel;
}