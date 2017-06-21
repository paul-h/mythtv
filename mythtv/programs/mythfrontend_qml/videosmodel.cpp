
// qt
#include <QDebug>
#include <QtQml/QQmlComponent>
#include <QtQml>

// mythfrontend_qml
#include "videosmodel.h"

VideosSortFilterProxyModel::VideosSortFilterProxyModel(QObject *parent, const QString &sourceModel)
    : QSortFilterProxyModel(parent)
{
    QQmlComponent component((QQmlEngine*)parent, sourceModel);

    if (component.isError())
        qDebug() << "VideosSortFilterProxyModel: ERROR: " << component.errorString();

    m_sourceModel = component.create();

    connect(m_sourceModel, SIGNAL(loaded()), this, SLOT(sourceLoaded()));
    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SIGNAL(countChanged()));
}

VideosSortFilterProxyModel::~VideosSortFilterProxyModel()
{
    delete m_sourceModel;
}

void VideosSortFilterProxyModel::sourceLoaded()
{
    setSource(m_sourceModel);

    QSortFilterProxyModel::sort(0, Qt::AscendingOrder);
}

void VideosSortFilterProxyModel::setFilterTitle(const QString &title)
{
    m_filterTitle = title;
    invalidateFilter();
}

void VideosSortFilterProxyModel::setFilterType(const QString &type)
{
    m_filterType = type;
    invalidateFilter();
}

void VideosSortFilterProxyModel::setFilterGenres(const QString &genres)
{
    m_filterGenres = genres;
    invalidateFilter();
}

int VideosSortFilterProxyModel::count() const
{
    return rowCount();
}

QObject *VideosSortFilterProxyModel::source() const
{
    return sourceModel();
}

void VideosSortFilterProxyModel::setSource(QObject *source)
{
    setSourceModel(qobject_cast<QAbstractItemModel *>(source));
}

QJSValue VideosSortFilterProxyModel::get(int idx) const
{
    QQmlEngine *engine = (QQmlEngine*)parent();
    QJSValue value = engine->newObject();
    if (idx >= 0 && idx < count())
    {
        QHash<int, QByteArray> roles = roleNames();
        QHashIterator<int, QByteArray> it(roles);
        while (it.hasNext())
        {
            it.next();
            value.setProperty(QString::fromUtf8(it.value()), data(index(idx, 0), it.key()).toString());
        }
    }
    return value;
}

QStringList VideosSortFilterProxyModel::getTitles() const
{
    QStringList list;
    QAbstractItemModel *srcModel = sourceModel();

    if (srcModel)
    {
        for (int x = 0; x < srcModel->rowCount(); x++)
        {
            QModelIndex srcIndex = srcModel->index(x, 0);

            if (!srcIndex.isValid())
                continue;

            QString title = srcModel->data(srcIndex, roleKey("Title")).toString();

            if (!list.contains(title))
                list << title;
        }
    }

    return list;
}

QStringList VideosSortFilterProxyModel::getTypes() const
{
    QStringList list;
    QAbstractItemModel *srcModel = sourceModel();

    if (srcModel)
    {
        for (int x = 0; x < srcModel->rowCount(); x++)
        {
            QModelIndex srcIndex = srcModel->index(x, 0);

            if (!srcIndex.isValid())
                continue;

            QString type = srcModel->data(srcIndex, roleKey("ContentType")).toString();

            if (!list.contains(type))
                list << type;
        }
    }

    return list;
}

QStringList VideosSortFilterProxyModel::getGenres() const
{
    QStringList list;
    QAbstractItemModel *srcModel = sourceModel();

    if (srcModel)
    {
        for (int x = 0; x < srcModel->rowCount(); x++)
        {
            QModelIndex srcIndex = srcModel->index(x, 0);

            if (!srcIndex.isValid())
                continue;

            QString genre = srcModel->data(srcIndex, roleKey("Genre")).toString();

            if (!list.contains(genre))
                list << genre;
        }
    }

    return list;
}

int VideosSortFilterProxyModel::roleKey(const QByteArray &role) const
{
    QHash<int, QByteArray> roles = roleNames();
    QHashIterator<int, QByteArray> it(roles);

    while (it.hasNext())
    {
        it.next();
        if (it.value() == role)
            return it.key();
    }

    return -1;
}

QHash<int, QByteArray> VideosSortFilterProxyModel::roleNames() const
{
    if (QAbstractItemModel *source = sourceModel())
        return source->roleNames();

    return QHash<int, QByteArray>();
}

bool VideosSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QAbstractItemModel *model = sourceModel();
    QModelIndex sourceIndex = model->index(sourceRow, 0, sourceParent);

    if (!sourceIndex.isValid())
        return true;

    if (!m_filterTitle.isEmpty())
    {
        QString key = model->data(sourceIndex, roleKey("Title")).toString();
        if (!key.contains(m_filterTitle))
            return false;
    }

    if (!m_filterType.isEmpty())
    {
        QString key = model->data(sourceIndex, roleKey("ContentType")).toString();
        if (key != m_filterType)
            return false;
    }

    if (!m_filterGenres.isEmpty())
    {
        QString key = model->data(sourceIndex, roleKey("Genre")).toString();
        if (!key .contains(m_filterGenres))
            return false;
    }

    return true;
}

bool VideosSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    QString leftTitle = sourceModel()->data(left, roleKey("Title")).toString();
    QString rightTitle = sourceModel()->data(right, roleKey("Title")).toString();

    int leftSeason = sourceModel()->data(left, roleKey("Season")).toInt();
    int rightSeason = sourceModel()->data(right, roleKey("Season")).toInt();

    int leftEpisode = sourceModel()->data(left, roleKey("Episode")).toInt();
    int rightEpisode = sourceModel()->data(right, roleKey("Episode")).toInt();

    if (QString::localeAwareCompare(leftTitle, rightTitle) < 0)
        return true;

    if (leftTitle == rightTitle)
    {
        if (leftSeason < rightSeason)
            return true;

        if (leftSeason == rightSeason)
        {
            if (leftEpisode < rightEpisode)
                return true;
        }
    }

    return false;
}
