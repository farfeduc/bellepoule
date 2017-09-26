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

#include "oauth/session.hpp"
#include "session.hpp"
#include "feed_request.hpp"

namespace Net
{
  // --------------------------------------------------------------------------------
  FeedRequest::FeedRequest (Oauth::Session *session,
                            const gchar    *message)
    : Object ("Facebook::FeedRequest"),
    Oauth::V2::Request (session, "user_id/feed", POST)
  {
    {
      Session *facebook_session = dynamic_cast <Session *> (_session);
      gchar   *signature = g_strdup_printf ("%s/feed",
                                            facebook_session->GetUserId ());

      SetSignature (signature);
      g_free (signature);
    }

    AddParameterField ("message",
                       message);
  }

  // --------------------------------------------------------------------------------
  FeedRequest::~FeedRequest ()
  {
  }

  // --------------------------------------------------------------------------------
  void FeedRequest::ParseResponse (GHashTable  *header,
                                   const gchar *body)
  {
    Oauth::Request::ParseResponse (header,
                                   body);

    if (GetStatus () == ACCEPTED)
    {
      if (LoadJson (body))
      {
        gchar *id = GetJsonAtPath ("$.name");

        _session->SetAccountId (id);
        g_free (id);
      }
    }
    else if (GetStatus () == REJECTED)
    {
      if (LoadJson (body))
      {
        char *code = GetJsonAtPath ("$.error.code");

        if (code && g_strcmp0 (code, "506") == 0)
        {
          printf ("Error << " RED "%s" ESC " >> dropped\n", code);
          ForgiveError ();
        }

        g_free (code);
      }
    }
  }
}
