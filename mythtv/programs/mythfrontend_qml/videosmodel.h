#ifndef VIDEOSORTFILTERPROXYMODEL_H
#define VIDEOSORTFILTERPROXYMODEL_H

#include <QDate>
#include <QSortFilterProxyModel>
#include <QtQml/qjsvalue.h>
#include <QStringListModel>

class VideosSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QObject *source READ source WRITE setSource)

    Q_PROPERTY(QString filterTitle READ filterTitle WRITE setFilterTitle)
    Q_PROPERTY(QString filterType READ filterType WRITE setFilterType)
    Q_PROPERTY(QString filterGenres READ filterGenres WRITE setFilterGenres)

public:
    VideosSortFilterProxyModel(QObject *parent = 0, const QString &sourceModel = "");
    ~VideosSortFilterProxyModel();

    QString filterTitle() const { return m_filterTitle; }
    void setFilterTitle(const QString &title);

    QString filterType() const { return m_filterType; }
    void setFilterType(const QString &type);

    QString filterGenres() const { return m_filterGenres; }
    void setFilterGenres(const QString &genres);

    QObject *source() const;
    void setSource(QObject *source);

    int count() const;
    Q_INVOKABLE QJSValue get(int index) const;

    Q_INVOKABLE QStringList getTitles() const;
    Q_INVOKABLE QStringList getTypes() const;
    Q_INVOKABLE QStringList getGenres() const;

signals:
    void countChanged();

private slots:
    void sourceLoaded();

protected:
    int roleKey(const QByteArray &role) const;
    QHash<int, QByteArray> roleNames() const;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

private:
    QObject *m_sourceModel;

    QString m_filterTitle;
    QString m_filterType;
    QString m_filterGenres;
};

#endif // VIDEOSORTFILTERPROXYMODEL_H
