/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:GPL2+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QtCompositor/QWaylandSurface>
#include <QtCompositor/private/qwlinputdevice_p.h>
#include <QtCompositor/private/qwlsurface_p.h>

#include "xdgshell.h"
#include "xdgsurface.h"
#include "xdgpopup.h"
#include "xdgpopupgrabber.h"

Q_LOGGING_CATEGORY(XDGSHELL_PROTOCOL, "greenisland.protocols.xdgshell")

namespace GreenIsland {

XdgShellGlobal::XdgShellGlobal(QObject *parent)
    : QObject(parent)
{
#ifdef ENABLE_XDG_SHELL_TRACE
    qCDebug(XDGSHELL_PROTOCOL) << Q_FUNC_INFO;
#endif
}

const wl_interface *XdgShellGlobal::interface() const
{
#ifdef ENABLE_XDG_SHELL_TRACE
    qCDebug(XDGSHELL_PROTOCOL) << Q_FUNC_INFO;
#endif

    return &xdg_shell_interface;
}

void XdgShellGlobal::bind(wl_client *client, uint32_t version, uint32_t id)
{
#ifdef ENABLE_XDG_SHELL_TRACE
    qCDebug(XDGSHELL_PROTOCOL) << Q_FUNC_INFO;
#endif

    new XdgShell(client, id, version, this);
}

XdgShell::XdgShell(wl_client *client, uint32_t name, uint32_t version, QObject *parent)
    : QObject(parent)
    , QtWaylandServer::xdg_shell(client, name, version)
{
#ifdef ENABLE_XDG_SHELL_TRACE
    qCDebug(XDGSHELL_PROTOCOL) << Q_FUNC_INFO;
#endif
}

XdgShell::~XdgShell()
{
#ifdef ENABLE_XDG_SHELL_TRACE
    qCDebug(XDGSHELL_PROTOCOL) << Q_FUNC_INFO;
#endif

    wl_resource_set_implementation(resource()->handle, Q_NULLPTR, Q_NULLPTR, Q_NULLPTR);

    qDeleteAll(m_popupGrabbers);
}

void XdgShell::pingSurface(XdgSurface *surface)
{
#ifdef ENABLE_XDG_SHELL_TRACE
    qCDebug(XDGSHELL_PROTOCOL) << Q_FUNC_INFO;
#endif

    uint32_t serial = surface->nextSerial();
    m_pings[serial] = surface;

    send_ping(serial);
}

XdgPopupGrabber *XdgShell::popupGrabberForDevice(QtWayland::InputDevice *device)
{
#ifdef ENABLE_XDG_SHELL_TRACE
    qCDebug(XDGSHELL_PROTOCOL) << Q_FUNC_INFO;
#endif

    // Create popup grabbers on demand
    if (!m_popupGrabbers.contains(device))
        m_popupGrabbers.insert(device, new XdgPopupGrabber(device));
    return m_popupGrabbers.value(device);
}

void XdgShell::shell_destroy_resource(Resource *resource)
{
#ifdef ENABLE_XDG_SHELL_TRACE
    qCDebug(XDGSHELL_PROTOCOL) << Q_FUNC_INFO;
#endif

    Q_UNUSED(resource)
    delete this;
}

void XdgShell::shell_use_unstable_version(Resource *resource, int32_t version)
{
#ifdef ENABLE_XDG_SHELL_TRACE
    qCDebug(XDGSHELL_PROTOCOL) << Q_FUNC_INFO;
#endif

    if (version != version_current)
        wl_resource_post_error(resource->handle, WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "incompatible version, server is %d client wants %d",
                               version_current, version);
}

void XdgShell::shell_get_xdg_surface(Resource *resource, uint32_t id, wl_resource *surfaceResource)
{
#ifdef ENABLE_XDG_SHELL_TRACE
    qCDebug(XDGSHELL_PROTOCOL) << Q_FUNC_INFO;
#endif

    QWaylandSurface *surface = QWaylandSurface::fromResource(surfaceResource);
    Q_ASSERT(surface);

    // Fail if get_xdg_surface is called on a xdg_surface
    Q_FOREACH (QWaylandSurfaceInterface *interface, surface->interfaces()) {
        XdgSurface *surfaceInterface = static_cast<XdgSurface *>(interface);
        if (surfaceInterface) {
            wl_resource_post_error(resource->handle, QtWaylandServer::xdg_shell::error_role,
                                   "This wl_surface is already a xdg_surface");
            return;
        }
    }

    new XdgSurface(this, surface, resource->client(), id, resource->version());
}

void XdgShell::shell_get_xdg_popup(Resource *resource, uint32_t id, wl_resource *surfaceResource,
                                   wl_resource *parentResource, wl_resource *seatResource,
                                   uint32_t serial, int32_t x, int32_t y, uint32_t flags)
{
#ifdef ENABLE_XDG_SHELL_TRACE
    qCDebug(XDGSHELL_PROTOCOL) << Q_FUNC_INFO;
#endif

    Q_UNUSED(flags)

    QWaylandSurface *parent = QWaylandSurface::fromResource(parentResource);
    Q_ASSERT(parent);
    QWaylandSurface *surface = QWaylandSurface::fromResource(surfaceResource);
    Q_ASSERT(surface);

    Q_FOREACH (QWaylandSurfaceInterface *interface, surface->interfaces()) {
        XdgPopup *popupInterface = static_cast<XdgPopup *>(interface);
        if (popupInterface) {
            wl_resource_post_error(resource->handle, QtWaylandServer::xdg_shell::error_role,
                                   "This wl_surface is already a xdg_popup");
            return;
        }
    }

    new XdgPopup(this, parent, surface,
                 QtWayland::InputDevice::fromSeatResource(seatResource)->handle(),
                 resource->client(), id, resource->version(),
                 x, y, serial);
}

void XdgShell::shell_pong(Resource *resource, uint32_t serial)
{
#ifdef ENABLE_XDG_SHELL_TRACE
    qCDebug(XDGSHELL_PROTOCOL) << Q_FUNC_INFO;
#endif

    Q_UNUSED(resource)

    XdgSurface *surface = m_pings.take(serial);
    surface->surface()->pong();
}

}
