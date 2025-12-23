#ifndef MPRIS_H
#define MPRIS_H

#include <QObject>
#include <QtDBus/QDBusAbstractAdaptor>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusObjectPath>
#include <QVariantMap>
#include "mpv.h"

class MprisPlayerAdaptor : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")
    Q_PROPERTY(QString PlaybackStatus READ playbackStatus)
    Q_PROPERTY(QVariantMap Metadata READ metadata)
    Q_PROPERTY(double Volume READ volume WRITE setVolume)
    Q_PROPERTY(qlonglong Position READ position)
    Q_PROPERTY(bool CanGoNext READ canGoNext)
    Q_PROPERTY(bool CanGoPrevious READ canGoPrevious)
    Q_PROPERTY(bool CanPlay READ canPlay)
    Q_PROPERTY(bool CanPause READ canPause)
    Q_PROPERTY(bool CanSeek READ canSeek)
    Q_PROPERTY(bool CanControl READ canControl)

public:
    MprisPlayerAdaptor(MpvObject *player);

    QString playbackStatus() const;
    QVariantMap metadata() const;
    double volume() const;
    void setVolume(double volume);
    qlonglong position() const;

    bool canGoNext() const { return false; }
    bool canGoPrevious() const { return false; }
    bool canPlay() const { return true; }
    bool canPause() const { return true; }
    bool canSeek() const { return true; }
    bool canControl() const { return true; }

public slots:
    void PlayPause();
    void Pause();
    void Play();
    void Stop();
    void Seek(qlonglong Offset);
    void SetPosition(const QDBusObjectPath &TrackId, qlonglong Position);

signals:
    void PropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidatedProperties);

private:
    MpvObject *m_player;
};

class MprisManager : public QObject {
    Q_OBJECT
public:
    static MprisManager& instance();
    void registerPlayer(MpvObject *player);
    MprisPlayerAdaptor* getAdaptor() { return m_adaptor; }

private:
    MprisManager() : m_adaptor(nullptr) {}
    MprisPlayerAdaptor *m_adaptor;
};

#endif
