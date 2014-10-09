/*
    Copyright (C) 2012  Spatial Transcriptomics AB,
    read LICENSE for licensing terms.
    Contact : Jose Fernandez Navarro <jose.fernandez.navarro@scilifelab.se>

*/

#ifndef stVi_H
#define stVi_H

#include <QMainWindow>
#include <QPointer>

class QSettings;
class QCloseEvent;
class QAction;
class QStatusBar;
class QMenu;
class Error;
class QVBoxLayout;
class ExtendedTabWidget;
class QWidget;
class DataProxy;
class AuthorizationManager;

class stVi : public QMainWindow
{
    Q_OBJECT

public:

    explicit stVi(QWidget* parent = 0);
    ~stVi();

    //initialize main visual components
    void init();

    // check that the host has all the required hardware (true yes : false no )
    bool checkSystemRequirements() const;

    //system settings
    void loadSettings();
    void saveSettings() const;

public slots:

    // handlemessage is intented to be called from another
    // instance trying to run at the same time
    void handleMessage(const QString& message);

private slots:

    // exit the application
    void slotExit();

    //clear the cache and local stored files
    void slotClearCache();

    // opens pop up static widget to show info about the application
    void slotShowAbout();

private:

    // create all the widgets
    void setupUi();
    // initialize and configure layout
    void createLayouts();
    // load style sheets
    void initStyle();
    // create keyboard shortcuts
    void createShorcuts();
    // create some connections
    void createConnections();

    // overloaded close Event function to handle the exit
    void closeEvent(QCloseEvent* event) override;

    QPointer<QAction> m_actionExit;
    QPointer<QAction> m_actionHelp;
    QPointer<QAction> m_actionVersion;
    QPointer<QAction> m_actionAbout;
    QPointer<QAction> m_actionClear_Cache;

    //main tab is a tab manager sort of widget that contains all the pages view
    QPointer<ExtendedTabWidget> m_mainTab;

    //stVi owns dataProxy and AuthorizationManager
    QPointer<DataProxy> m_dataProxy;
    QPointer<AuthorizationManager> m_authManager;
};

#endif // stVi_H
