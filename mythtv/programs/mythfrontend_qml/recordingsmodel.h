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
    Q_PROPERTY(bool descending READ descending WRITE setDescending NOTIFY descendingChanged)

    QString titleRegExp(void) { return m_title; }
    void setTitleRegExp(const QString &title);

    QString recGroup(void) { return m_recGroup; }
    void setRecGroup(const QString &recGroup);

    QString storageGroup(void) { return m_storageGroup; }
    void setStorageGroup(const QString &storageGroup);

    bool descending(void) { return m_descending; }
    void setDescending(bool descending);

 signals:
     void titleRegExpChanged(void);
     void recGroupChanged(void);
     void storageGroupChanged(void);
     void descendingChanged(void);

  protected slots:
    virtual void startDownload(void);
    virtual void processDownload(QByteArray buffer);

  private:
    QString m_title;
    QString m_recGroup;
    QString m_storageGroup;
    bool    m_descending;
};

#endif // RECORDINGSMODEL_H
