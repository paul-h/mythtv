#ifndef RECORDINGSMODEL_H
#define RECORDINGSMODEL_H

#include <QAbstractListModel>
#include <QStringList>
#include <QDateTime>

#include "mythincrementalmodel.h"

class RecordingsModel : public MythIncrementalModel
{
    Q_OBJECT
public:
    RecordingsModel(QObject *parent, Settings *settings);
    ~RecordingsModel(void) {};

    Q_PROPERTY(QString titleRegExp READ titleRegExp WRITE setTitleRegExp NOTIFY titleRegExpChanged)
    Q_PROPERTY(QString recGroup READ recGroup WRITE setRecGroup NOTIFY recGroupChanged)
    Q_PROPERTY(QString storageGroup READ storageGroup WRITE setStorageGroup NOTIFY storageGroupChanged)

    QString titleRegExp(void) { return m_title; }
    void setTitleRegExp(const QString &title);

    QString recGroup(void) { return m_recGroup; }
    void setRecGroup(const QString &recGroup);

    QString storageGroup(void) { return m_storageGroup; }
    void setStorageGroup(const QString &storageGroup);

 signals:
     void titleRegExpChanged(void);
     void recGroupChanged(void);
     void storageGroupChanged(void);

  protected slots:
    virtual void startDownload(void);
    virtual void processDownload(QByteArray buffer);

  private:
    QString m_title;
    QString m_recGroup;
    QString m_storageGroup;
};

#endif // RECORDINGSMODEL_H
