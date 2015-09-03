// Copyright (C) 2011 Yannick Le Roux.
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

#ifndef message_hpp
#define message_hpp

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include "util/object.hpp"

namespace Net
{
  class Message : public Object
  {
    public:
      Message (const gchar *name);

      Message (const guint8       *data,
               struct sockaddr_in *from = NULL);

      gboolean Is (const gchar *name);

      gboolean IsValid ();

      gchar *GetSenderIp ();

      void Spread ();

      void Recall ();

      void Set (const gchar *field,
                const gchar *value);

      void Set (const gchar *field,
                const guint  value);

      gchar *GetString (const gchar *field);

      guint GetInteger (const gchar *field);

      gchar *GetParcel ();

      void Dump ();

    private:
      virtual ~Message ();

    private:
      gboolean  _is_valid;
      GKeyFile *_key_file;
  };
}
#endif
