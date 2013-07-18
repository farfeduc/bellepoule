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

#include "http_server.hpp"

namespace Net
{
  // --------------------------------------------------------------------------------
  HttpServer::RequestBody::RequestBody ()
  {
    _data   = NULL;
    _length = 0;
  }

  // --------------------------------------------------------------------------------
  HttpServer::RequestBody::~RequestBody ()
  {
    g_free (_data);
  }

  // --------------------------------------------------------------------------------
  void HttpServer::RequestBody::Append (const char *buf,
                                        size_t      len)
  {
    _data = (gchar *) g_realloc (_data,
                                 _length + len);
    strncpy (&_data[_length],
             buf,
             len);
    _length += len;
  }
}

namespace Net
{
  // --------------------------------------------------------------------------------
  HttpServer::DeferedData::DeferedData (HttpServer  *server,
                                        const gchar *url,
                                        RequestBody *request_body)
  {
    _server = server;
    _server->Retain ();

    _url     = g_strdup (url);
    _content = g_strndup (request_body->_data,
                          request_body->_length);
  }

  // --------------------------------------------------------------------------------
  HttpServer::DeferedData::~DeferedData ()
  {
    g_free (_url);
    g_free (_content);
    _server->Release ();
  }
}

namespace Net
{
  // --------------------------------------------------------------------------------
  HttpServer::HttpServer (Object   *client,
                          HttpPost  http_post,
                          HttpGet   http_get)
    : Object ("HttpServer")
  {
    _daemon = MHD_start_daemon (MHD_USE_DEBUG | MHD_USE_SELECT_INTERNALLY,
                                35830,
                                NULL, NULL,
                                (MHD_AccessHandlerCallback) OnMicroHttpRequest, this,
                                MHD_OPTION_NOTIFY_COMPLETED, OnMicroHttpRequestCompleted, this,
                                MHD_OPTION_END);
    _client        = client;
    _http_POST_cbk = http_post;
    _http_GET_cbk  = http_get;
  }

  // --------------------------------------------------------------------------------
  HttpServer::~HttpServer ()
  {
    MHD_stop_daemon (_daemon);
  }

  // --------------------------------------------------------------------------------
  gboolean HttpServer::DeferedPost (DeferedData *defered_data)
  {
    defered_data->_server->_http_POST_cbk (defered_data->_server->_client,
                                           defered_data->_url,
                                           defered_data->_content);

    delete (defered_data);

    return FALSE;
  }

  // --------------------------------------------------------------------------------
  int HttpServer::OnRequestReceived (struct MHD_Connection *connection,
                                     RequestBody           *request_body,
                                     const char            *url,
                                     const char            *method,
                                     const char            *upload_data,
                                     size_t                *upload_data_size)
  {
    int ret = MHD_NO;

    if (strcmp (method, "GET") == 0)
    {
      if (*upload_data_size == 0)
      {
        gchar *client_response = _http_GET_cbk (_client,
                                                url);

        if (client_response)
        {
          struct MHD_Response *response;
          char                *page;

          page = g_strdup (client_response);
          g_free (client_response);

          response = MHD_create_response_from_data (strlen (page),
                                                    (void *) page,
                                                    MHD_YES,
                                                    MHD_NO);
          ret = MHD_queue_response (connection,
                                    MHD_HTTP_OK,
                                    response);

          MHD_destroy_response (response);
        }
      }
    }
    else if (strcmp (method, "POST") == 0)
    {
      if (*upload_data_size)
      {
        request_body->Append (upload_data,
                              *upload_data_size);
        *upload_data_size = 0;
        return MHD_YES;
      }
      else
      {
        {
          DeferedData *defered_data = new DeferedData (this,
                                                       url,
                                                       request_body);

          g_idle_add ((GSourceFunc) DeferedPost,
                      defered_data);
        }

        {
          struct MHD_Response *response;

          response = MHD_create_response_from_data (strlen ("POST received by BellePoule"),
                                                    (void *) "POST received by BellePoule",
                                                    MHD_NO,
                                                    MHD_NO);
          ret = MHD_queue_response (connection,
                                    MHD_HTTP_OK,
                                    response);

          MHD_destroy_response (response);
        }
      }
    }

    return ret;
  }

  // --------------------------------------------------------------------------------
  void HttpServer::OnMicroHttpRequestCompleted (HttpServer                      *server,
                                                struct MHD_Connection           *connection,
                                                RequestBody                     **request_body,
                                                enum MHD_RequestTerminationCode   code)
  {
    if (*request_body)
    {
      delete (*request_body);
    }

    *request_body = NULL;
  }

  // --------------------------------------------------------------------------------
  int HttpServer::OnMicroHttpRequest (HttpServer            *server,
                                      struct MHD_Connection *connection,
                                      const char            *url,
                                      const char            *method,
                                      const char            *version,
                                      const char            *upload_data,
                                      size_t                *upload_data_size,
                                      RequestBody           **request_body)
  {
    if (*request_body == NULL)
    {
      *request_body = new RequestBody ();
      return MHD_YES;
    }

    return server->OnRequestReceived (connection,
                                      *request_body,
                                      url,
                                      method,
                                      upload_data,
                                      upload_data_size);
  }
}
