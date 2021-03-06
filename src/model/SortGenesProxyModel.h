#ifndef SORTGENESPROXYMODEL_H
#define SORTGENESPROXYMODEL_H

#include <QSortFilterProxyModel>

// Sort proxy class used to sort genes features table
// by some specific criteria (for instance name)
class SortGenesProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    enum SortCategory { ambiguousGene = 0, numericGene = 1, normalGene = 2 };

    explicit SortGenesProxyModel(QObject *parent = 0);
    virtual ~SortGenesProxyModel();

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    Q_DISABLE_COPY(SortGenesProxyModel)
};

#endif // SORTGENESPROXYMODEL_H
