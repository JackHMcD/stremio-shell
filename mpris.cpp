#include "mpris.h"
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMetaType>
#include <QtDBus/QDBusArgument>

inline QDBusArgument &operator<<(QDBusArgument &argument, const mpv::qt::ErrorReturn &myStruct) {
    argument.beginStructure();
    argument << myStruct.error;
    argument.endStructure();
    return argument;
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument, mpv::qt::ErrorReturn &myStruct) {
    argument.beginStructure();
    argument >> myStruct.error;
    argument.endStructure();
    return argument;
}

MprisPlayerAdaptor::MprisPlayerAdaptor(MpvObject *player) 
    : QDBusAbstractAdaptor(player), m_player(player) {
}

QString MprisPlayerAdaptor::playbackStatus() const {
    if (!m_player) return "Stopped";
    
    // Check if there's an active file/stream playing
    bool idleActive = m_player->getProperty("idle-active").toBool();
    if (idleActive) return "Stopped";
    
    // Check if pause property is set
    bool paused = m_player->getProperty("pause").toBool();
    return paused ? "Paused" : "Playing";
}

QVariantMap MprisPlayerAdaptor::metadata() const {
    QVariantMap map;
    if (!m_player) return map;
    
    map["mpris:trackid"] = QVariant::fromValue(QDBusObjectPath("/org/stremio/track/0"));
    map["xesam:title"] = m_player->getProperty("media-title");
    
    double duration = m_player->getProperty("duration").toDouble();
    if (duration > 0) {
        map["mpris:length"] = qlonglong(duration * 1000000);
    }
    
    return map;
}

double MprisPlayerAdaptor::volume() const {
    if (!m_player) return 0.0;
    return m_player->getProperty("volume").toDouble() / 100.0;
}

void MprisPlayerAdaptor::setVolume(double volume) {
    if (!m_player) return;
    m_player->setProperty("volume", volume * 100.0);
}

qlonglong MprisPlayerAdaptor::position() const {
    if (!m_player) return 0;
    double timePos = m_player->getProperty("time-pos").toDouble();
    return qlonglong(timePos * 1000000);
}

void MprisPlayerAdaptor::PlayPause() {
    if (m_player) {
        m_player->command(QVariantList() << "cycle" << "pause");
    }
}

void MprisPlayerAdaptor::Pause() {
    if (m_player) {
        m_player->setProperty("pause", true);
    }
}

void MprisPlayerAdaptor::Play() {
    if (m_player) {
        m_player->setProperty("pause", false);
    }
}

void MprisPlayerAdaptor::Stop() {
    if (m_player) {
        m_player->command(QVariantList() << "stop");
    }
}

void MprisPlayerAdaptor::Seek(qlonglong Offset) {
    if (m_player) {
        m_player->command(QVariantList() << "seek" << (double)Offset / 1000000.0 << "relative");
    }
}

void MprisPlayerAdaptor::SetPosition(const QDBusObjectPath &TrackId, qlonglong Position) {
    Q_UNUSED(TrackId);
    if (m_player) {
        m_player->setProperty("time-pos", (double)Position / 1000000.0);
    }
}

MprisManager& MprisManager::instance() {
    static MprisManager m;
    return m;
}

void MprisManager::registerPlayer(MpvObject *player) {
    if (!player) return;
    
    m_adaptor = new MprisPlayerAdaptor(player);
    
    m_dbus.registerObject("/org/mpris/MediaPlayer2", player);
    m_dbus.registerService("org.mpris.MediaPlayer2.stremio");
}

void MprisManager::unregisterPlayer() {
    m_dbus.unregisterService("org.mpris.MediaPlayer2.stremio");
    m_dbus.unregisterObject("/org/mpris/MediaPlayer2");
    
    if (m_adaptor) {
        delete m_adaptor;
        m_adaptor = nullptr;
    }
}

MprisManager::~MprisManager() {
    if (m_adaptor) {
        delete m_adaptor;
        m_adaptor = nullptr;
    }
    unregisterPlayer();
}
