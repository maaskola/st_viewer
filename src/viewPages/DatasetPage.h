/*
    Copyright (C) 2012  Spatial Transcriptomics AB,
    read LICENSE for licensing terms.
    Contact : Jose Fernandez Navarro <jose.fernandez.navarro@scilifelab.se>

*/

#ifndef DATASETPAGE_H
#define DATASETPAGE_H

#include "Page.h"

#include <memory>
#include "data/DataProxy.h"
#include <QModelIndex>

class QItemSelectionModel;
class QItemSelection;
class Error;
class DatasetItemModel;
class QSortFilterProxyModel;

namespace Ui {
class DataSets;
} // namespace Ui //

// this is the definition of the datasets page which contains a list of datasets to be selected
// it gets updated everytime we enter the page and by selecting a dataset
// we invoke the dataproxy to load the data
// as every page it implements the moveToNextPage and moveToPreviousPage
// the methods onEnter and onExit are called dynamically from the page manager.
class DatasetPage : public Page
{
    Q_OBJECT

public:

    DatasetPage(QPointer<DataProxy> dataProxy, QWidget *parent = 0);
    virtual ~DatasetPage();

public slots:

    void onEnter() override;
    void onExit() override;

private slots:

    //to handle when the user selects a dataset
    void slotDatasetSelected(QModelIndex index);

    //some slots for the actions buttons of the view
    void slotLoadDatasets();
    void slotOpenDataset();
    void slotRemoveDataset();
    void slotEditDataset();

    //slot used to be notified when the datasets have been downloaded,
    //updated or removed from network
    //status contains the status of the operation (ok, abort, error)
    void slotDatasetsDownloaded(const DataProxy::DownloadStatus status);
    void slotDatasetsModified(const DataProxy::DownloadStatus status);

private:

    //clear focus and resets to default
    void clearControls();

    //to get the model from the table
    QSortFilterProxyModel *datasetsProxyModel();
    DatasetItemModel *datasetsModel();

    std::unique_ptr<Ui::DataSets> m_ui;

    Q_DISABLE_COPY(DatasetPage)
};

#endif  /* DATASETPAGE_H */

