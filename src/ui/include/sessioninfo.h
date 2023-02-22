/*
 * Copyright (C) 2011, 2014 Nicolas Bonnefon and other contributors
 *
 * This file is part of glogg.
 *
 * glogg is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * glogg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with glogg.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Copyright (C) 2019 Anton Filimonov and other contributors
 *
 * This file is part of klogg.
 *
 * klogg is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * klogg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with klogg.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SESSIONINFO_H
#define SESSIONINFO_H

#include <string>
#include <vector>

#include <QByteArray>
#include <QString>

#include "persistable.h"
#include "TimelineNodeInfo.h"

// Simple component class containing information related to the session
// to be persisted and reloaded upon start
class SessionInfo : public Persistable<SessionInfo, session_settings> {
  public:
    static const char* persistableName()
    {
        return "SessionInfo";
    }

    struct OpenFile {
        OpenFile( const QString& file, uint64_t top, const QString& context, const std::vector<TimelineNodeInfo>& nodes)
            : fileName{ file }
            , topLine{ top }
            , viewContext{ context }
        {
            timelineNodes = nodes;
        }

        QString fileName;
        uint64_t topLine;
        std::vector<TimelineNodeInfo> timelineNodes;

        // The view context contains parameter specific to the view's
        // implementation (such as geometry...)
        QString viewContext;
    };

    struct Window {

        explicit Window( const QString& windowId )
            : id{ windowId }
        {
        }

        QString id;
        QByteArray geometry;
        std::vector<OpenFile> openFiles;
    };

    void add( const QString& windowId )
    {
        if ( findWindow( windowId ) == nullptr ) {
            windows_.emplace_back( Window{ windowId } );
            LOG_INFO << "Created window session info for " << windowId;
        }
    }

    bool remove( const QString& windowId )
    {
        if ( windows_.size() > 1 ) {
            auto window
                = std::find_if( windows_.begin(), windows_.end(),
                                [ &windowId ]( const auto& w ) { return w.id == windowId; } );
            if ( window != windows_.end() ) {
                windows_.erase( window );
            }
            return true;
        }

        return false;
    }

    QStringList windows() const
    {
        QStringList ids;
        std::transform( windows_.cbegin(), windows_.cend(), std::back_inserter( ids ),
                        []( const auto& w ) { return w.id; } );

        return ids;
    }

    QByteArray geometry( const QString& windowId ) const
    {
        auto window = findWindow( windowId );
        return window ? window->geometry : QByteArray{};
    }

    void setGeometry( const QString& windowId, const QByteArray& geometry )
    {
        auto window = findWindow( windowId );
        if ( window ) {
            window->geometry = geometry;
        }
    }

    // List of the loaded files
    std::vector<OpenFile> openFiles( const QString& windowId ) const
    {
        auto window = findWindow( windowId );
        return window ? window->openFiles : std::vector<OpenFile>{};
    }

    void setOpenFiles( const QString& windowId, const std::vector<OpenFile>& loaded_files )
    {
        auto window = findWindow( windowId );
        if ( window ) {
            window->openFiles = loaded_files;
        }
    }

    // Reads/writes the current config in the QSettings object passed
    void saveToStorage( QSettings& settings ) const;
    void retrieveFromStorage( QSettings& settings );

  private:
    Window* findWindow( const QString& windowId ) const
    {
        auto window = std::find_if( windows_.begin(), windows_.end(),
                                    [ &windowId ]( const auto& w ) { return w.id == windowId; } );

        if ( window == windows_.end() ) {
            LOG_INFO << "Can't find window " << windowId;
            return nullptr;
        }
        else {
            return &*window;
        }
    }

  private:
    mutable std::vector<Window> windows_;
};

#endif
