/*
    Copyright (C) 2012  Spatial Transcriptomics AB,
    read LICENSE for licensing terms.
    Contact : Jose Fernandez Navarro <jose.fernandez.navarro@scilifelab.se>

*/

#ifndef GENEVIEWDELEGATE_H
#define GENEVIEWDELEGATE_H

#include <QStyledItemDelegate>

// Specialized delegate to manage the visualizing and modification of gene data
// in a table view.

//NOTE this delegate is not finished yet

class GeneViewDelegate : public QStyledItemDelegate
{

public:

    explicit GeneViewDelegate(QObject* parent = 0);
    virtual ~GeneViewDelegate();

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

    Q_DISABLE_COPY(GeneViewDelegate)
};

#endif // GENEVIEWDELEGATE_H //