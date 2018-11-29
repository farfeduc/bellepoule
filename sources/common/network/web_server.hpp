// Copyright (C) 2009 Yannick Le Roux.
// This file is part of BellePoule.
//
//   BellePoule is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   BellePoule is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with BellePoule.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <glib.h>

#include "util/object.hpp"

namespace Net
{
  class WebServer : public Object
  {
    public:
      struct Listener
      {
        virtual void OnWebServerState (gboolean in_progress,
                                       gboolean on) = 0;
      };

      WebServer (Listener *listener);

      void Start ();

      void Stop ();

    private:
      GMutex    _mutex;
      Listener *_listener;
      gboolean  _in_progress;
      gboolean  _on;
      gboolean  _failed;

      ~WebServer () override;

      void Prepare ();

      void Spawn (const gchar *script);

      static gpointer StartUp (WebServer *server);

      static gpointer ShutDown (WebServer *server);

      static guint OnProgress (WebServer *owner);
  };
}
