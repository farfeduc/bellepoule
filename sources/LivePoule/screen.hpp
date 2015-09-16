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

#ifndef screen_hpp
#define screen_hpp

#include <gtk/gtk.h>

#include "util/module.hpp"
#include "network/http_server.hpp"
#include "network/wifi_network.hpp"
#include "util/wifi_code.hpp"
#include "timer.hpp"
#include "button.hpp"
#include "light.hpp"
#include "scoring_machine.hpp"
#include "wpa.hpp"

class Screen : public Module, Net::HttpServer::Client
{
  public:
    Screen ();

    void ManageScoringMachine (ScoringMachine *machine);

    gboolean OnKeyPressed (GdkEventKey *event);

  private:
    Net::HttpServer *_http_server;
    WifiCode        *_wifi_code;
    gchar           *_wifi_configuration;
    Timer           *_timer;
    GData           *_lights;
    guint            _strip_id;
    GList           *_scoring_machines;
    Button          *_qr_code_pin;
    Button          *_strip_plus_pin;
    Button          *_strip_minus_pin;
    Wpa             *_wpa;

    virtual ~Screen ();

    void ResetDisplay ();

    void RefreshStripId ();

    void Rescale (gdouble factor);

    void ChangeNumber (gint step);

    void SetCompetition (GKeyFile *key_file);

    void SetTimer (GKeyFile *key_file);

    void SetScore (const gchar *light_color,
                   GKeyFile    *key_file);

    void SetColor (const gchar *color);

    void SetTitle (const gchar *title);

    void SetFencer (const gchar *light_color,
                    const gchar *data,
                    const gchar *name);

    gboolean OnHttpPost (const gchar *data);

    gchar *GetSecretKey (const gchar *authentication_scheme);

    void Unfullscreen ();

    void ToggleWifiCode ();

    void ChangeStripId (gint step);

  private:
    static gboolean HttpPostCbk (Net::HttpServer::Client *client,
                                 const gchar             *data);

    static gboolean OnLightEvent (Screen *screen);

    static gboolean OnQrCodeButton (Screen *context);

    static gboolean OnStripPlusPin (Screen *context);

    static gboolean OnStripMinusPin (Screen *context);

    static gboolean OnNetworkConfigured (Object *client);
};

#endif
