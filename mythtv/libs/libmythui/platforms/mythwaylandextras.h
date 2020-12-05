#ifndef MYTHWAYLANDEXTRAS_H
#define MYTHWAYLANDEXTRAS_H

// Qt
#include <QRect>

// Std
#include <map>

class  QWidget;
struct wl_interface;
struct wl_display;
struct wl_registry;
struct wl_compositor;
struct wl_surface;

using MythWaylandRegistry = std::map<const wl_interface*,void*>;

class MythWaylandExtras
{
  public:
    static void AnnounceGlobal(void* Opaque, struct wl_registry* Reg, uint32_t Name,
                               const char * Interface, uint32_t Version);
    static const struct wl_registry_listener s_registryListener;
};

class MythWaylandDevice
{
  public:
    explicit MythWaylandDevice(QWidget* Widget);
    bool SetOpaqueRegion(const QRect Region);

    wl_display*    m_display    { nullptr };
    wl_compositor* m_compositor { nullptr };
    wl_surface*    m_surface    { nullptr };
};

#endif
