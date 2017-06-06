/****************************************************************************
 * This file is part of Liri.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL2.1+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#ifndef XWAYLANDWINDOW_H
#define XWAYLANDWINDOW_H

#include <QtCore/QRect>
#include <QtCore/QPointer>

#include <QtWaylandCompositor/QWaylandOutput>
#include <QtWaylandCompositor/QWaylandSeat>
#include <QtWaylandCompositor/QWaylandSurface>

#include <xcb/xcb.h>

#include "xcbwindow.h"
#include "sizehints.h"

class XWaylandManager;
class XWaylandSurface;
class XWaylandQuickShellIntegration;
class XWaylandQuickShellSurfaceItem;

class XWaylandShellSurface : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Qt::WindowType windowType READ windowType NOTIFY windowTypeChanged)
    Q_PROPERTY(QWaylandSurface *surface READ surface NOTIFY surfaceChanged)
    Q_PROPERTY(XWaylandShellSurface *parentSurface READ parentSurface NOTIFY parentSurfaceChanged)
    Q_PROPERTY(bool activated READ isActivated NOTIFY activatedChanged)
    Q_PROPERTY(QString appId READ appId NOTIFY appIdChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(bool decorate READ decorate NOTIFY decorateChanged)
    Q_PROPERTY(int x READ x NOTIFY xChanged)
    Q_PROPERTY(int y READ y NOTIFY yChanged)
    Q_PROPERTY(bool maximized READ isMaximized NOTIFY maximizedChanged)
    Q_PROPERTY(bool fullscreen READ isFullscreen NOTIFY fullscreenChanged)
public:
    enum ResizeEdge {
        NoneEdge        =  0,
        TopEdge         =  1,
        BottomEdge      =  2,
        LeftEdge        =  4,
        TopLeftEdge     =  5,
        BottomLeftEdge  =  6,
        RightEdge       =  8,
        TopRightEdge    =  9,
        BottomRightEdge = 10
    };
    Q_ENUM(ResizeEdge)

    enum WmState {
        WithdrawnState = 0,
        NormalState = 1,
        IconicState = 3
    };

    XWaylandShellSurface(QObject *parent = nullptr);
    ~XWaylandShellSurface();

    Q_INVOKABLE void initialize(XWaylandManager *wm, quint32 window,
                                const QRect &geometry, bool overrideRedirect,
                                XWaylandShellSurface *parentShellSurface);

    Qt::WindowType windowType() const;

    quint32 surfaceId() const;
    void setSurfaceId(quint32 id);

    QWaylandSurface *surface() const;
    void setSurface(QWaylandSurface *surface);

    XWaylandShellSurface *parentSurface() const;

    bool isActivated() const;

    QString appId() const;
    QString title() const;

    bool decorate() const;

    QPoint position() const;
    QRect geometry() const;

    int x() const;
    int y() const;

    bool isMaximized() const;
    bool isFullscreen() const;

    inline WmState wmState() const {
        return m_wmState;
    }

    void setWmState(WmState state);

    void setNetWmState();

    inline int workspace() const {
        return m_workspace;
    }

    void setWorkspace(int workspace);

    void dirtyProperties();
    void readProperties();
    void setProperties();

    QSize sizeForResize(const QSizeF &initialSize, const QPointF &delta, ResizeEdge edges);
    void sendConfigure(const QRect &geometry);

    void moveTo(const QPoint &pos);
    void resize(const QSize &size);

    Q_INVOKABLE void sendPosition(const QPointF &pos);
    Q_INVOKABLE void sendResize(const QSizeF &size);

    Q_INVOKABLE void maximize(QWaylandOutput *output);
    Q_INVOKABLE void unmaximize();

    Q_INVOKABLE void close();

    XWaylandQuickShellIntegration *createIntegration(XWaylandQuickShellSurfaceItem *item);

    operator xcb_window_t() const;
    xcb_window_t window() const;

Q_SIGNALS:
    void windowTypeChanged();
    void surfaceChanged();
    void surfaceDestroyed();
    void parentSurfaceChanged();
    void activatedChanged();
    void appIdChanged();
    void titleChanged();
    void decorateChanged();
    void xChanged();
    void yChanged();
    void mapped();
    void unmapped();
    void setPosition(const QPoint &pos);
    void setSize(const QSize &size);
    void maximizedChanged();
    void fullscreenChanged();
    void startMove();
    void startResize(ResizeEdge edges);

private:
    XWaylandManager *m_wm;
    xcb_window_t m_window;
    QRect m_geometry;
    QRect m_savedGeometry;
    bool m_propsDirty;
    bool m_overrideRedirect;
    bool m_hasAlpha;
    XWaylandShellSurface *m_transientFor;
    Qt::WindowType m_windowType;
    quint32 m_surfaceId;
    QWaylandSurface *m_surface;
    WmState m_wmState;
    int m_workspace;
    WmSizeHints m_sizeHints;
    MotifWmHints m_motifHints;

    struct {
        QString title;
        QString appId;
        QSize savedSize;
        int deleteWindow;
    } m_properties;

    bool m_activated;
    bool m_decorate;
    bool m_maximized;
    bool m_fullscreen;

    friend class XWaylandManager;

private Q_SLOTS:
    void handleSeatChanged(QWaylandSeat *newSeat, QWaylandSeat *oldSeat);
    void handleFocusChanged(QWaylandSurface *newSurface, QWaylandSurface *oldSurface);
    void handleFocusReceived();
    void handleFocusLost();
    void handleSurfaceDestroyed();
};

#endif // XWAYLANDWINDOW_H
