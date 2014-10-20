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

#include <unistd.h>
#include <glib.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gstdio.h>
#include <curl/curl.h>
#include <locale.h>

#ifdef WINDOWS_TEMPORARY_PATCH
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#endif

#include "util/canvas.hpp"

#include "version.h"
#include "contest.hpp"
#include "people_management/form.hpp"

#include "tournament.hpp"

// --------------------------------------------------------------------------------
Tournament::Tournament (gchar *filename)
  : Module ("tournament.glade")
{
  _contest_list = NULL;
  _referee_list = NULL;
  _referee_ref  = 1;
  _nb_matchs    = 0;

  _ecosystem = new EcoSystem (_glade);

  curl_global_init (CURL_GLOBAL_ALL);

  EnumerateLanguages ();

  // Show the main window
  {
    GtkWidget *w = _glade->GetRootWidget ();

    gtk_widget_show_all (w);
    gtk_widget_hide (_glade->GetWidget ("notebook"));
  }

  gtk_widget_hide (_glade->GetWidget ("update_menuitem"));

  if (filename)
  {
    gchar *utf8_name = g_locale_to_utf8 (filename,
                                         -1,
                                         NULL,
                                         NULL,
                                         NULL);

    OpenUriContest (utf8_name);
    g_free (utf8_name);

    g_free (filename);
  }

  {
    GtkWidget *w       = _glade->GetWidget ("about_dialog");
    gchar     *version = g_strdup_printf ("V%s.%s%s", VERSION, VERSION_REVISION, VERSION_MATURITY);

    gtk_about_dialog_set_version (GTK_ABOUT_DIALOG (w),
                                  (const gchar *) version);
    g_free (version);
  }

  {
    GtkWidget   *w         = _glade->GetWidget ("about_dialog");
    const gchar *authors[] = {"Florence Blanchard",
      "Laurent Bonnot",
      "Tony (ajs New Mexico)",
      "Emmanuel Chaix",
      "Julien Diaz",
      "Olivier Larcher",
      "Yannick Le Roux",
      "Jean-Pierre Mahé",
      "Pierre Moro",
      "Killian Poulet",
      "Michel Relet",
      "Vincent Rémy",
      "Tina Schliemann",
      "Claude Simonnot",
      "Sébastien Vermandel",
      NULL};

    gtk_about_dialog_set_authors (GTK_ABOUT_DIALOG (w),
                                  authors);
  }

  {
    GtkWidget *w           = _glade->GetWidget ("about_dialog");
    gchar     *translators = g_strdup_printf ("Julien Diaz           (German)\n"
                                              "Tina Schliemann       (German)\n"
                                              "Michael Weber         (German)\n"
                                              "Aureliano Martini     (Italian)\n"
                                              "Jihwan Cho            (Korean)\n"
                                              "Marijn Somers         (Dutch)\n"
                                              "Werner Huysmans       (Dutch)\n"
                                              "Farid Elkhalki        (Japanese)\n"
                                              "Jean-François Brun    (Japanese)\n"
                                              "Alexis Pigeon         (Spanish)\n"
                                              "Paulo Morales         (Spanish)\n"
                                              "Ignacio Gil           (Spanish)\n"
                                              "Eduardo Alberto Calvo (Spanish, Portuguese)\n"
                                              "Rui Pedro Vieira      (Portuguese)\n"
                                              "Mohamed Rebai         (Arabic)\n"
                                              "Mikael Hiort af Ornäs (Swedish)\n"
                                              "Sergev Makhtanov      (Russian)");

    gtk_about_dialog_set_translator_credits (GTK_ABOUT_DIALOG (w),
                                             translators);
    g_free (translators);
  }

  {
    gchar *last_backup = g_key_file_get_string (_config_file,
                                                "Tournament",
                                                "backup_location",
                                                NULL);
    SetBackupLocation (last_backup);
    g_free (last_backup);
  }

  {
    GtkWidget     *main_window = GTK_WIDGET (_glade->GetWidget ("root"));
    GtkTargetList *target_list;

    gtk_drag_dest_set (main_window,
                       GTK_DEST_DEFAULT_ALL,
                       NULL,
                       0,
                       GDK_ACTION_COPY);

    target_list = gtk_drag_dest_get_target_list (main_window);

    if (target_list == NULL)
    {
      target_list = gtk_target_list_new (NULL, 0);
      gtk_drag_dest_set_target_list (main_window, target_list);
      gtk_target_list_unref (target_list);
    }
    gtk_target_list_add_uri_targets (target_list, 100);
  }

  {
    _version_downloader = new Net::Downloader ("_version_downloader",
                                               OnLatestVersionReceived,
                                               this);
    _version_downloader->Start ("http://betton.escrime.free.fr/documents/BellePoule/latest.html");
  }

  {
    gchar *ip_addr;
    gchar *html_url;

    _http_server = new Net::HttpServer (this,
                                        HttpPostCbk,
                                        HttpGetCbk,
                                        35830);
    ip_addr = _http_server->GetIpV4 ();
    html_url = g_strdup_printf ("http://%s:35830/tournament/list.html", ip_addr);

    SetFlashRef (html_url);

    g_free (html_url);
    g_free (ip_addr);
  }
}

// --------------------------------------------------------------------------------
Tournament::~Tournament ()
{
  {
    GError *error       = NULL;
    gchar  *config_path = g_strdup_printf ("%s/BellePoule/config.ini", g_get_user_config_dir ());

    if (config_path)
    {
      gsize  config_length;
      gchar *config = g_key_file_to_data (_config_file,
                                          &config_length,
                                          &error);

      if (error)
      {
        g_error ("g_key_file_to_data: %s", error->message);
        g_error_free (error);
      }
      else if (g_file_set_contents (config_path,
                                    config,
                                    config_length,
                                    &error) == FALSE)
      {
        g_error ("g_file_set_contents: %s", error->message);
        g_error_free (error);
      }
      g_free (config);
    }
    g_free (config_path);
  }

  g_slist_free_full (_referee_list,
                     (GDestroyNotify) Object::TryToRelease);

  g_key_file_free (_config_file);

  if (_version_downloader)
  {
    _version_downloader->Kill    ();
    _version_downloader->Release ();
  }

  _http_server->Release ();
  curl_global_cleanup ();

  _ecosystem->Release ();
}

// --------------------------------------------------------------------------------
void Tournament::Init ()
{
  gchar *dir_path  = g_strdup_printf ("%s/BellePoule", g_get_user_config_dir ());
  gchar *file_path = g_strdup_printf ("%s/config.ini", dir_path);

  _config_file = g_key_file_new ();
  if (g_key_file_load_from_file (_config_file,
                                 file_path,
                                 G_KEY_FILE_KEEP_COMMENTS,
                                 NULL) == FALSE)
  {
    g_mkdir_with_parents (dir_path,
                          0700);

    g_key_file_set_string (_config_file,
                           "Competiton",
                           "default_dir_name",
                           g_get_user_special_dir (G_USER_DIRECTORY_DOCUMENTS));

    g_key_file_set_string (_config_file,
                           "Checkin",
                           "default_import_dir_name",
                           g_get_user_special_dir (G_USER_DIRECTORY_DOCUMENTS));
  }

  g_free (dir_path);
  g_free (file_path);
}

// --------------------------------------------------------------------------------
gchar *Tournament::GetSecretKey (const gchar *authentication_scheme)
{
  WifiCode *wifi_code = NULL;

  if (authentication_scheme)
  {
    gchar **tokens = g_strsplit_set (authentication_scheme,
                                     "/",
                                     0);

    if (tokens && tokens[0] && tokens[1] && tokens[2])
    {
      if (strcmp (tokens[1], "ScoreSheet") == 0)
      {
        wifi_code = _ecosystem->GetAdminCode ();
      }
      else if (   (strcmp (tokens[1], "HandShake") == 0)
               || (strcmp (tokens[1], "Score")     == 0))
      {
        GSList *current = _referee_list;
        guint   ref     = atoi (tokens[2]);

        if (ref == 0)
        {
          wifi_code = _ecosystem->GetAdminCode ();
        }
        else
        {
          while (current)
          {
            Player *referee = (Player *) current->data;

            if (referee->GetRef () == ref)
            {
              wifi_code = (WifiCode *) referee->GetFlashCode ();
              break;
            }

            current = g_slist_next (current);
          }
        }
      }
      g_strfreev (tokens);
    }
  }

  if (wifi_code)
  {
    return wifi_code->GetKey ();
  }

  return NULL;
}

// --------------------------------------------------------------------------------
guint Tournament::PreparePrint (GtkPrintOperation *operation,
                                GtkPrintContext   *context)
{
  guint list_length = g_slist_length (_referee_list);
  guint n;

  if (_print_meal_tickets)
  {
    n = list_length / (NB_TICKET_Y_PER_SHEET * NB_TICKET_X_PER_SHEET);

    if (list_length % (NB_TICKET_Y_PER_SHEET * NB_TICKET_X_PER_SHEET))
    {
      n++;
    }
  }
  else
  {
    n = list_length / NB_REFEREE_PER_SHEET;

    if (list_length % NB_REFEREE_PER_SHEET)
    {
      n++;
    }
  }

  return n;
}

// --------------------------------------------------------------------------------
void Tournament::DrawPage (GtkPrintOperation *operation,
                           GtkPrintContext   *context,
                           gint               page_nr)
{
  gdouble        paper_w  = gtk_print_context_get_width  (context);
  gdouble        paper_h  = gtk_print_context_get_height (context);
  cairo_t       *cr       = gtk_print_context_get_cairo_context (context);
  GooCanvas     *canvas   = Canvas::CreatePrinterCanvas (context);
  GooCanvasItem *main_table;
  GooCanvasItem *item;
  GSList        *current;

  cairo_save (cr);

  if (_print_meal_tickets)
  {
    gdouble spacing  = 2.0;
    gdouble ticket_w = (100.0/NB_TICKET_X_PER_SHEET) - (NB_TICKET_X_PER_SHEET-1) *spacing;
    gdouble ticket_h = (100.0/NB_TICKET_Y_PER_SHEET) *paper_h/paper_w - (NB_TICKET_Y_PER_SHEET-1) *spacing;
    gdouble border_w = 0.7;

    main_table = goo_canvas_table_new (goo_canvas_get_root_item (canvas),
                                       "row-spacing",         2.0,
                                       "column-spacing",      2.0,
                                       "homogeneous-columns", TRUE,
                                       "homogeneous-rows",    TRUE,
                                       NULL);

    current = g_slist_nth (_referee_list,
                           NB_TICKET_Y_PER_SHEET*NB_TICKET_X_PER_SHEET*page_nr);

    for (guint r = 0; current && (r < NB_TICKET_Y_PER_SHEET); r++)
    {
      for (guint c = 0; current && (c < NB_TICKET_X_PER_SHEET); c++)
      {
        Player        *referee = (Player *) current->data;
        GooCanvasItem *ticket_table = goo_canvas_table_new (main_table,
                                                            "x-border-spacing", 1.0,
                                                            "y-border-spacing", 1.0,
                                                            NULL);
        // Border
        {
          item = goo_canvas_rect_new (main_table,
                                      0.0,
                                      0.0,
                                      ticket_w,
                                      ticket_h,
                                      "stroke-color", "Grey",
                                      "line-width",   border_w,
                                      NULL);
          Canvas::PutInTable (main_table,
                              item,
                              r,
                              c);
        }

        {
          {
            GooCanvasItem *name_table = goo_canvas_table_new (ticket_table, NULL);

            // Name
            {
              gchar               *font   = g_strdup_printf ("Sans Bold %fpx", 2*PRINT_FONT_HEIGHT);
              Player::AttributeId  attr_id ("name");
              Attribute           *attr   = referee->GetAttribute (&attr_id);
              gchar               *string = attr->GetUserImage (AttributeDesc::LONG_TEXT);

              Canvas::NormalyzeDecimalNotation (font);
              item = Canvas::PutTextInTable (name_table,
                                             string,
                                             0,
                                             0);
              g_object_set (G_OBJECT (item),
                            "font",      font,
                            "ellipsize", PANGO_ELLIPSIZE_END,
                            "width",     ticket_w/4.0 - 2.0*border_w,
                            NULL);
              g_free (string);
              g_free (font);
            }

            // First name
            {
              gchar               *font   = g_strdup_printf ("Sans %fpx", 1.5*PRINT_FONT_HEIGHT);
              Player::AttributeId  attr_id ("first_name");
              Attribute           *attr   = referee->GetAttribute (&attr_id);
              gchar               *string = attr->GetUserImage (AttributeDesc::LONG_TEXT);

              Canvas::NormalyzeDecimalNotation (font);
              item = Canvas::PutTextInTable (name_table,
                                             string,
                                             1,
                                             0);
              g_object_set (G_OBJECT (item),
                            "font", font,
                            "ellipsize", PANGO_ELLIPSIZE_END,
                            "width",     ticket_w/4.0 - 2.0*border_w,
                            NULL);
              g_free (string);
              g_free (font);
            }

            Canvas::PutInTable (ticket_table,
                                name_table,
                                0,
                                0);
            Canvas::SetTableItemAttribute (name_table, "x-expand", 1U);
            Canvas::SetTableItemAttribute (name_table, "x-fill",   1U);
          }

          // Food
          {
            GooCanvasItem *food_table = goo_canvas_table_new (ticket_table, NULL);
            gchar         *font   = g_strdup_printf ("Sans %fpx", 1.8*PRINT_FONT_HEIGHT);
            const gchar   *strings[] = {"Meal", "Drink", "Dessert", "Coffee", NULL};

            Canvas::NormalyzeDecimalNotation (font);

            g_object_set (G_OBJECT (food_table),
                          "horz-grid-line-width", 0.3,
                          "stroke-color", "White",
                          "fill-color",   "grey",
                          NULL);
            for (guint i = 0; strings[i] != NULL; i++)
            {
              item = Canvas::PutTextInTable (food_table,
                                             gettext (strings[i]),
                                             i,
                                             0);
              g_object_set (G_OBJECT (item),
                            "font",       font,
                            "fill-color", "Black",
                            NULL);
              Canvas::SetTableItemAttribute (item, "x-align", 1.0);
              Canvas::SetTableItemAttribute (item, "x-fill",   0U);
            }

            g_free (font);

            Canvas::PutInTable (ticket_table,
                                food_table,
                                0,
                                1);
            Canvas::SetTableItemAttribute (food_table, "right-padding", 1.0);
          }
        }

        Canvas::PutInTable (main_table,
                            ticket_table,
                            r,
                            c);
        Canvas::SetTableItemAttribute (ticket_table, "x-expand", 1U);
        Canvas::SetTableItemAttribute (ticket_table, "x-fill",   1U);
        current = g_slist_next (current);
      }
    }
  }
  else
  {
    gchar         *font = g_strdup_printf ("Sans %fpx", 1.0*PRINT_FONT_HEIGHT);
    GooCanvasItem *header;

    Canvas::NormalyzeDecimalNotation (font);

    current = g_slist_nth (_referee_list,
                           NB_REFEREE_PER_SHEET*page_nr);

    main_table = goo_canvas_table_new (goo_canvas_get_root_item (canvas),
                                       "row-spacing",          4.0,
                                       "y-border-spacing",     2.0,
                                       "column-spacing",       10.0,
                                       "horz-grid-line-width", 0.2,
                                       "homogeneous-rows",     TRUE,
                                       NULL);

    {
      gchar *font = g_strdup_printf ("Sans Bold %fpx", 3.0*PRINT_FONT_HEIGHT);

      Canvas::NormalyzeDecimalNotation (font);
      header = goo_canvas_text_new (goo_canvas_get_root_item (canvas),
                                    gettext ("Payment book"),
                                    50.0,
                                    1.0,
                                    -1.0,
                                    GTK_ANCHOR_CENTER,
                                    "font", font,
                                    NULL);
      g_free (font);
    }

    {
      gchar *font   = g_strdup_printf ("Sans Bold %fpx", 1.0*PRINT_FONT_HEIGHT);
      gchar *string = g_strdup_printf ("%s %d", gettext ("Page"), page_nr+1);

      Canvas::NormalyzeDecimalNotation (font);
      goo_canvas_text_new (goo_canvas_get_root_item (canvas),
                           string,
                           100.0,
                           100.0 * paper_h/paper_w,
                           -1.0,
                           GTK_ANCHOR_SE,
                           "font", font,
                           NULL);
      g_free (string);
      g_free (font);
    }

    for (guint r = 0; current && (r < NB_REFEREE_PER_SHEET); r++)
    {
      Player *referee = (Player *) current->data;
      guint   c       = 0;

      {
        const gchar *column[] = {"name", "first_name", "level", NULL};

        for (guint i = 0; column[i] != NULL; i++)
        {
          Player::AttributeId  attr_id (column[i]);
          Attribute           *attr   = referee->GetAttribute (&attr_id);
          gchar               *string;

          if (attr)
          {
            string = attr->GetUserImage (AttributeDesc::LONG_TEXT);
          }
          else
          {
            string = g_strdup ("-");
          }

          item = Canvas::PutTextInTable (main_table,
                                         string,
                                         r,
                                         c);
          Canvas::SetTableItemAttribute (item, "y-align", 0.5);
          g_object_set (G_OBJECT (item),
                        "font", font,
                        NULL);
          g_free (string);
          c++;
        }
      }

      {
        struct lconv *local_conv = localeconv ();
        const gchar  *column[]   = {local_conv->currency_symbol, gettext ("Signature"), NULL};

        for (guint i = 0; column[i] != NULL; i++)
        {
          item = Canvas::PutTextInTable (main_table,
                                         gettext (column[i]),
                                         r,
                                         c);
          Canvas::SetTableItemAttribute (item, "x-align", 0.5);
          Canvas::SetTableItemAttribute (item, "y-align", 0.5);
          g_object_set (G_OBJECT (item),
                        "font", font,
                        "fill-color", "grey",
                        NULL);
          c++;
        }
      }

      current = g_slist_next (current);
    }

    Canvas::Anchor (main_table,
                    header,
                    NULL,
                    40);
#if 0
    if (_print_scale != 1.0)
    {
      cairo_matrix_t matrix;

      goo_canvas_item_get_transform (goo_canvas_get_root_item (canvas),
                                     &matrix);
      cairo_matrix_scale (&matrix,
                          _print_scale,
                          _print_scale);

      goo_canvas_item_set_transform (goo_canvas_get_root_item (canvas),
                                     &matrix);
    }
#endif
  }

  goo_canvas_render (canvas,
                     cr,
                     NULL,
                     1.0);
  gtk_widget_destroy (GTK_WIDGET (canvas));

  cairo_restore (cr);
}

// --------------------------------------------------------------------------------
Player *Tournament::UpdateConnectionStatus (GSList      *player_list,
                                            guint        ref,
                                            const gchar *ip_address,
                                            const gchar *status)
{
  GSList *current = player_list;

  while (current)
  {
    Player *current_player = (Player *) current->data;

    if (current_player->GetRef () == ref)
    {
      Player::AttributeId connection_attr_id ("connection");
      Player::AttributeId ip_attr_id         ("IP");

      current_player = current_player;
      current_player->SetAttributeValue (&connection_attr_id,
                                         "OK");
      if (ip_address)
      {
        current_player->SetAttributeValue (&ip_attr_id,
                                           ip_address);
      }
      return current_player;
    }
    current = g_slist_next (current);
  }

  return NULL;
}

// --------------------------------------------------------------------------------
Contest *Tournament::FetchContest (const gchar *id)
{
  if (id)
  {
    GSList *current = _contest_list;

    while (current)
    {
      Contest *contest = (Contest *) current->data;

      if (strcmp (contest->GetId (), id) == 0)
      {
        return contest;
      }
      current = g_slist_next (current);
    }
  }
  return NULL;
}

// --------------------------------------------------------------------------------
gboolean Tournament::OnHttpPost (const gchar *data)
{
  gboolean result = FALSE;

  if (data)
  {
    gchar **lines = g_strsplit_set (data,
                                    "\n",
                                    0);
    if (lines[0])
    {
      const gchar *body = data;

      body = strstr (body, "\n"); if (body) body++;
      // Source
      {
        gchar **tokens = g_strsplit_set (lines[0],
                                         "/",
                                         0);

        if (tokens && tokens[0] && tokens[1] && tokens[2])
        {
          // Status feedback
          UpdateConnectionStatus (_referee_list,
                                  atoi (tokens[2]),
                                  tokens[1],
                                  "OK");
        }
        g_strfreev (tokens);
      }

      // Request type
      if (lines[1])
      {
        gchar **tokens = g_strsplit_set (lines[1],
                                         "/",
                                         0);

        body = strstr (body, "\n"); if (body) body++;
        if (tokens && tokens[0] && tokens[1])
        {
          // Competition data
          if (   (strcmp (tokens[1], "Score") == 0)
              || (strcmp (tokens[1], "ScoreSheet") == 0))
          {
            if (tokens[2] && (strcmp (tokens[2], "Competition") == 0))
            {
              gchar *competition_id = tokens[3];

              if (competition_id)
              {
                GSList *current = _contest_list;

                while (current)
                {
                  Contest *contest = (Contest *) current->data;

                  if (strcmp (contest->GetId (), competition_id) == 0)
                  {
                    if (strcmp (tokens[1], "ScoreSheet") == 0)
                    {
                      GtkNotebook *nb  = GTK_NOTEBOOK (_glade->GetWidget ("notebook"));
                      gint        page = gtk_notebook_page_num (nb,
                                                                contest->GetRootWidget ());

                      g_object_set (G_OBJECT (nb),
                                    "page", page,
                                    NULL);
                    }

                    result = contest->OnHttpPost (tokens[1],
                                                  (const gchar**) &tokens[4],
                                                  body);
                    break;
                  }
                  current = g_slist_next (current);
                }
              }
            }
          }
        }
        g_strfreev (tokens);
      }
    }
    g_strfreev (lines);
  }

  return result;
}

// --------------------------------------------------------------------------------
gchar *Tournament::OnHttpGet (const gchar *url)
{
  gchar *result = NULL;

  if (strstr (url, "/tournament/competition/"))
  {
    gchar *id = (gchar *) strrchr (url, '/');

    if (id && id[1])
    {
      GSList *current = _contest_list;

      id++;
      while (current)
      {
        Contest *contest = (Contest *) current->data;

        if (    contest->GetFilename ()
             && (strcmp (contest->GetId (), id) == 0))
        {
          g_file_get_contents (contest->GetFilename (),
                               &result,
                               NULL,
                               NULL);
          break;
        }
        current = g_slist_next (current);
      }
    }
  }
  else if (strstr (url, "/tournament/html/competition/"))
  {
    gchar *id = (gchar *) strrchr (url, '/');

    if (id && id[1])
    {
      GSList *current = _contest_list;

      id++;
      while (current)
      {
        Contest *contest = (Contest *) current->data;

        if (    contest->GetHtmlFileName ()
             && (strcmp (contest->GetId (), id) == 0))
        {
          g_file_get_contents (contest->GetHtmlFileName (),
                               &result,
                               NULL,
                               NULL);
          break;
        }
        current = g_slist_next (current);
      }
    }
  }
  else if (strcmp (url, "/tournament/list.html") == 0)
  {
    GSList  *current  = _contest_list;
    GString *response = g_string_new ("");
    gchar   *ip_addr  = _http_server->GetIpV4 ();

    response = g_string_append (response, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd\">\n");
    response = g_string_append (response, "<html>\n");
    response = g_string_append (response, "  <head>\n");
    response = g_string_append (response, "    <meta http-equiv=\"Content-Type\" content=\"text/html;charset=UTF-8\">\n");
    response = g_string_append (response, "  </head>\n");

    response = g_string_append (response, "  <body>\n");
    while (current)
    {
      Contest *contest = (Contest *) current->data;

      response = g_string_append (response, "    <div>\n");
      response = g_string_append (response, "      <a href=\"http://");
      response = g_string_append (response, ip_addr);
      response = g_string_append (response, ":35830/tournament/html/competition/");
      response = g_string_append (response, contest->GetId ());
      response = g_string_append (response, "\">");

      response = g_string_append (response, contest->GetName ());
      response = g_string_append (response, "-");
      response = g_string_append (response, contest->GetWeapon ());
      response = g_string_append (response, "-");
      response = g_string_append (response, contest->GetGenderCode ());
      response = g_string_append (response, "-");
      response = g_string_append (response, contest->GetCategory ());

      response = g_string_append (response, "      </a>\n");
      response = g_string_append (response, "    </div>\n");

      current = g_slist_next (current);
    }
    response = g_string_append (response, "  </body>\n");
    response = g_string_append (response, "</html>\n");

    result = response->str;
    g_string_free (response,
                   FALSE);
    g_free (ip_addr);
  }
  else if (strcmp (url, "/tournament") == 0)
  {
    GSList  *current  = _contest_list;
    GString *response = g_string_new ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");

    response = g_string_append (response,
                                "<Competitions>\n");
    while (current)
    {
      Contest *contest = (Contest *) current->data;

      response = g_string_append (response,
                                  "<Competition ");

      response = g_string_append (response, "ID=\"");
      response = g_string_append (response, contest->GetId ());
      response = g_string_append (response, "\" ");

      response = g_string_append (response, "Name=\"");
      response = g_string_append (response, contest->GetName ());
      response = g_string_append (response, "\" ");

      response = g_string_append (response, "Weapon=\"");
      response = g_string_append (response, contest->GetWeapon ());
      response = g_string_append (response, "\" ");

      response = g_string_append (response, "Gender=\"");
      response = g_string_append (response, contest->GetGenderCode ());
      response = g_string_append (response, "\" ");

      response = g_string_append (response, "Category=\"");
      response = g_string_append (response, contest->GetCategory ());
      response = g_string_append (response,
                                  "\"/>\n");

      current = g_slist_next (current);
    }
    response = g_string_append (response,
                                "</Competitions>");

    result = response->str;
    g_string_free (response,
                   FALSE);
  }

  return result;
}

// --------------------------------------------------------------------------------
gboolean Tournament::HttpPostCbk (Net::HttpServer::Client *client,
                                  const gchar             *data)
{
  Tournament *tournament = (Tournament *) client;

  return tournament->OnHttpPost (data);
}

// --------------------------------------------------------------------------------
gchar *Tournament::HttpGetCbk (Net::HttpServer::Client *client,
                               const gchar             *url)
{
  Tournament *tournament = (Tournament *) client;

  return tournament->OnHttpGet (url);
}

// --------------------------------------------------------------------------------
Player *Tournament::Share (Player  *referee,
                           Contest *from)
{
  Player *original = NULL;

  {
    GSList *current   = _referee_list;
    GSList *attr_list = NULL;
    Player::AttributeId  name_attr_id       ("name");
    Player::AttributeId  first_name_attr_id ("first_name");

    attr_list = g_slist_prepend (attr_list, &first_name_attr_id);
    attr_list = g_slist_prepend (attr_list, &name_attr_id);

    while (current)
    {
      Player *current_referee = (Player *) current->data;

      if (Player::MultiCompare (referee,
                                current_referee,
                                attr_list) == 0)
      {
        original = current_referee;
        referee->SetRef (original->GetRef ());
        break;
      }

      current = g_slist_next (current);
    }

    g_slist_free (attr_list);
  }

  if (original == NULL)
  {
    {
      Player::AttributeId attr_id ("name");

      _referee_list = g_slist_insert_sorted_with_data (_referee_list,
                                                       referee,
                                                       (GCompareDataFunc) Player::Compare,
                                                       &attr_id);
      referee->Retain ();
      referee->SetRef (_referee_ref++);
    }

    {
      GSList *current = _contest_list;

      while (current)
      {
        Contest *contest = (Contest *) current->data;

        if (   (contest != from)
            && (contest->GetWeaponCode () == referee->GetWeaponCode ()))
        {
          contest->AddReferee (referee);
        }

        current = g_slist_next (current);
      }
    }
  }

  return original;
}

// --------------------------------------------------------------------------------
void Tournament::RefreshMatchRate (gint delta)
{
  _nb_matchs += delta;

  {
    GSList *current = _referee_list;

    while (current)
    {
      Player *referee = (Player *) current->data;

      RefreshMatchRate (referee);

      current = g_slist_next (current);
    }
  }
}

// --------------------------------------------------------------------------------
void Tournament::RefreshMatchRate (Player *referee)
{
  Player::AttributeId attr_id ("participation_rate");

  if (_nb_matchs)
  {
    referee->SetAttributeValue (&attr_id,
                                referee->GetNbMatchs () * 100 / _nb_matchs);
  }
  else
  {
    referee->SetAttributeValue (&attr_id,
                                (guint) 0);
  }
}

// --------------------------------------------------------------------------------
void Tournament::EnumerateLanguages ()
{
  gchar  *contents;
  gchar  *filename     = g_build_filename (_program_path, "resources", "translations", "index.txt", NULL);

  if (g_file_get_contents (filename,
                           &contents,
                           NULL,
                           NULL) == TRUE)
  {
    gchar **lines = g_strsplit_set (contents, "\n", 0);

    if (lines)
    {
      const gchar *env     = g_getenv ("LANGUAGE");
      gchar  *original_env = NULL;
      GSList *group        = NULL;
      gchar  *last_setting = g_key_file_get_string (_config_file,
                                                    "Tournament",
                                                    "interface_language",
                                                    NULL);
      if (env)
      {
        original_env = g_strdup (env);
        gtk_menu_item_set_label (GTK_MENU_ITEM (_glade->GetWidget ("locale_menuitem")),
                                 env);
      }

      for (guint l = 0; lines[l] && lines[l][0]; l++)
      {
        gchar **tokens = g_strsplit_set (lines[l], ";", 0);

        if (tokens)
        {
          GtkWidget *item;

          g_setenv ("LANGUAGE",
                    tokens[0],
                    TRUE);

          g_strdelimit (tokens[1],
                        "\n\r",
                        '\0');

          item  = gtk_radio_menu_item_new_with_label (group, tokens[1]);
          group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (item));

          gtk_menu_shell_append (GTK_MENU_SHELL (_glade->GetWidget ("locale_menu")),
                                 item);
          gtk_widget_set_tooltip_text (item,
                                       gettext ("Restart BellePoule for this change to take effect"));

          g_signal_connect (item, "toggled",
                            G_CALLBACK (OnLocaleToggled), (void *)
                            g_strdup (tokens[0]));
          if (last_setting && strcmp (last_setting, tokens[0]) == 0)
          {
            gtk_menu_item_set_label (GTK_MENU_ITEM (_glade->GetWidget ("locale_menuitem")),
                                     tokens[0]);
            gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item),
                                            TRUE);
          }

          gtk_widget_show (item);
          g_strfreev (tokens);
        }
      }

      if (original_env)
      {
        g_setenv ("LANGUAGE",
                  original_env,
                  TRUE);
        g_free (original_env);
      }
      else
      {
        g_unsetenv ("LANGUAGE");
      }

      g_strfreev (lines);
    }
    g_free (contents);
  }

  g_free (filename);
}

// --------------------------------------------------------------------------------
gchar *Tournament::GetUserLanguage ()
{
  return g_key_file_get_string (_config_file,
                                "Tournament",
                                "interface_language",
                                NULL);
}

// --------------------------------------------------------------------------------
void Tournament::Manage (Contest *contest)
{
  if (contest)
  {
    GtkWidget *nb = _glade->GetWidget ("notebook");

    contest->AttachTo (GTK_NOTEBOOK (nb));

    _contest_list = g_slist_prepend (_contest_list,
                                     contest);
    contest->ImportReferees (_referee_list);

    if (g_slist_length (_contest_list) == 1)
    {
      gtk_widget_show (_glade->GetWidget ("notebook"));
      gtk_widget_hide (_glade->GetWidget ("logo"));
    }
  }
}

// --------------------------------------------------------------------------------
void Tournament::OnContestDeleted (Contest *contest)
{
  if (_contest_list)
  {
    _contest_list = g_slist_remove (_contest_list,
                                    contest);
    if (g_slist_length (_contest_list) == 0)
    {
      gtk_widget_show (_glade->GetWidget ("logo"));
      gtk_widget_hide (_glade->GetWidget ("notebook"));
    }
  }
}

// --------------------------------------------------------------------------------
Contest *Tournament::GetContest (const gchar *filename)
{
  GSList *current = _contest_list;

  while (current)
  {
    Contest *contest = (Contest *) current->data;

    if (strcmp (filename, contest->GetFilename ()) == 0)
    {
      return contest;
    }

    current = g_slist_next (current);
  }

  return NULL;
}

// --------------------------------------------------------------------------------
void Tournament::OnNew ()
{
  Contest *contest = Contest::Create ();

  Plug (contest,
        NULL);
  Manage (contest);
}

// --------------------------------------------------------------------------------
void Tournament::OnOpen (gchar *current_folder)
{
  GtkWidget *chooser = gtk_file_chooser_dialog_new (gettext ("Choose a competion file to open... "),
                                                    NULL,
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    GTK_STOCK_CANCEL,
                                                    GTK_RESPONSE_CANCEL,
                                                    GTK_STOCK_OPEN,
                                                    GTK_RESPONSE_ACCEPT,
                                                    NULL);

  {
    GtkFileFilter *filter = gtk_file_filter_new ();

    gtk_file_filter_set_name (filter,
                              gettext ("Competition files (.cotcot / .xml)"));
    gtk_file_filter_add_pattern (filter,
                                 "*.cotcot");
    gtk_file_filter_add_pattern (filter,
                                 "*.xml");
    gtk_file_filter_add_pattern (filter,
                                 "*.XML");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser),
                                 filter);
  }

  {
    GtkFileFilter *filter = gtk_file_filter_new ();

    gtk_file_filter_set_name (filter,
                              gettext ("All files"));
    gtk_file_filter_add_pattern (filter,
                                 "*");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser),
                                 filter);
  }

  if (current_folder && (g_file_test (current_folder,
                                      G_FILE_TEST_IS_DIR)))
  {
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (chooser),
                                         current_folder);
  }
  else
  {
    gchar *last_dirname = g_key_file_get_string (_config_file,
                                                 "Competiton",
                                                 "default_dir_name",
                                                 NULL);
    if (last_dirname && (g_file_test (last_dirname,
                                      G_FILE_TEST_IS_DIR)))
    {
      gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (chooser),
                                           last_dirname);

      g_free (last_dirname);
    }
  }

  if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT)
  {
    gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));

    OpenUriContest (filename);
    g_free (filename);
  }

  gtk_widget_destroy (chooser);
}

// --------------------------------------------------------------------------------
void Tournament::OnSave ()
{
  GSList *current = _contest_list;

  while (current)
  {
    Contest *contest = (Contest *) current->data;

    contest->Save ();

    current = g_slist_next (current);
  }
}

// --------------------------------------------------------------------------------
void Tournament::OnOpenUserManual ()
{
  gchar *uri;

#ifdef WINDOWS_TEMPORARY_PATCH
  uri = g_build_filename (_program_path, "resources", "user_manual.pdf", NULL);

  ShellExecute (NULL,
                "open",
                uri,
                NULL,
                NULL,
                SW_SHOWNORMAL);
#else
  uri = g_build_filename ("file://", _program_path, "resources", "user_manual.pdf", NULL);

  gtk_show_uri (NULL,
                uri,
                GDK_CURRENT_TIME,
                NULL);
#endif

  g_free (uri);
}

// --------------------------------------------------------------------------------
void Tournament::OnOpenTemplate ()
{
  GString *contents = g_string_new ("");
  gchar   *filename = g_build_filename (g_get_user_special_dir (G_USER_DIRECTORY_TEMPLATES),
                                        "fencer_file_template.csv",
                                        NULL);

  {
    GSList *current_desc = AttributeDesc::GetList ();

    while (current_desc)
    {
      AttributeDesc *desc = (AttributeDesc *) current_desc->data;

      if (desc->MatchCriteria ("CSV ready"))
      {
        gchar *locale_string = g_locale_from_utf8 (desc->_user_name,
                                                   -1,
                                                   NULL,
                                                   NULL,
                                                   NULL);
        contents = g_string_append (contents,
                                    locale_string);
        contents = g_string_append_c (contents,
                                      ';');

        g_free (locale_string);
      }

      current_desc = g_slist_next (current_desc);
    }
  }

  if (g_file_set_contents (filename,
                           contents->str,
                           -1,
                           NULL))
  {
    gchar *uri;

#ifdef WINDOWS_TEMPORARY_PATCH
    uri = g_locale_from_utf8 (filename,
                              -1,
                              NULL,
                              NULL,
                              NULL);

    ShellExecute (NULL,
                  "open",
                  uri,
                  NULL,
                  NULL,
                  SW_SHOWNORMAL);
#else
    uri = g_filename_to_uri (filename,
                             NULL,
                             NULL);
    gtk_show_uri (NULL,
                  uri,
                  GDK_CURRENT_TIME,
                  NULL);
#endif

    g_free (uri);
  }

  g_string_free (contents,
                 TRUE);
  g_free (filename);
}

// --------------------------------------------------------------------------------
void Tournament::OpenUriContest (const gchar *uri)
{
  if (uri)
  {
    SetCursor (GDK_WATCH);

    {
      gchar *dirname = g_path_get_dirname (uri);

      g_key_file_set_string (_config_file,
                             "Competiton",
                             "default_dir_name",
                             dirname);
      g_free (dirname);
    }

    {
      static const gchar *contest_suffix_table[] = {".cotcot", ".COTCOT", ".xml", ".XML", NULL};
      static const gchar *people_suffix_table[]  = {".fff", ".FFF", ".csv", ".CSV", ".txt", ".TXT", NULL};
      Contest            *contest = NULL;

      for (guint i = 0; contest_suffix_table[i] != NULL; i++)
      {
        if (g_str_has_suffix (uri,
                              contest_suffix_table[i]))
        {
          contest = new Contest ();

          Plug (contest,
                NULL);

          contest->LoadXml (uri);
          break;
        }
      }

      for (guint i = 0; people_suffix_table[i] != NULL; i++)
      {
        if (g_str_has_suffix (uri,
                              people_suffix_table[i]))
        {
          contest = Contest::Create ();

          if (contest)
          {
            Plug (contest,
                  NULL);

            contest->LoadFencerFile (uri);
          }
          break;
        }
      }

      Manage (contest);
    }

    ResetCursor ();

    while (gtk_events_pending ())
    {
      gtk_main_iteration ();
    }
  }
}

// --------------------------------------------------------------------------------
void Tournament::OnOpenExample ()
{
  if (_program_path)
  {
    gchar *example_dirname = g_build_filename (_program_path, "Exemples", NULL);

    OnOpen (example_dirname);
    g_free (example_dirname);
  }
}

// --------------------------------------------------------------------------------
void Tournament::OnRecent ()
{
  GtkWidget *dialog = gtk_recent_chooser_dialog_new (gettext ("Recently opened files"),
                                                     NULL,
                                                     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                     GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                                     NULL);

  gtk_recent_chooser_set_show_tips  (GTK_RECENT_CHOOSER (dialog),
                                     TRUE);

  {
    GtkRecentFilter *filter = gtk_recent_filter_new ();

    gtk_recent_filter_add_pattern (filter,
                                   "*.cotcot");
    gtk_recent_filter_set_name (filter,
                                gettext ("BellePoule files"));

    gtk_recent_chooser_add_filter (GTK_RECENT_CHOOSER (dialog),
                                   filter);
  }

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
  {
    GtkRecentInfo *info;

    info = gtk_recent_chooser_get_current_item (GTK_RECENT_CHOOSER (dialog));

    if (info)
    {
      OpenUriContest (gtk_recent_info_get_uri_display (info));

      gtk_recent_info_unref (info);
    }
  }

  gtk_widget_destroy (dialog);
}

// --------------------------------------------------------------------------------
void Tournament::OnMenuDialog (const gchar *dialog)
{
  GtkWidget *w = _glade->GetWidget (dialog);

  gtk_dialog_run (GTK_DIALOG (w));
  gtk_widget_hide (w);
}

// --------------------------------------------------------------------------------
void Tournament::PrintMealTickets ()
{
  _print_meal_tickets = TRUE;
  Print (gettext ("Meal tickets"));
}

// --------------------------------------------------------------------------------
void Tournament::PrintPaymentBook ()
{
  _print_meal_tickets = FALSE;
  Print (gettext ("Payment book"));
}

// --------------------------------------------------------------------------------
void Tournament::OnBackupfileLocation ()
{
  GtkWidget *chooser = GTK_WIDGET (gtk_file_chooser_dialog_new (gettext ("Choose a backup files location..."),
                                                                NULL,
                                                                GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                                                GTK_STOCK_CANCEL,
                                                                GTK_RESPONSE_CANCEL,
                                                                GTK_STOCK_APPLY,
                                                                GTK_RESPONSE_ACCEPT,
                                                                NULL));

  {
    gchar *last_location = g_key_file_get_string (_config_file,
                                                  "Tournament",
                                                  "backup_location",
                                                  NULL);
    if (last_location)
    {
      gtk_file_chooser_select_uri (GTK_FILE_CHOOSER (chooser),
                                   last_location);

      g_free (last_location);
    }
  }

  if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT)
  {
    gchar *foldername = gtk_file_chooser_get_uri (GTK_FILE_CHOOSER (chooser));

    if (foldername)
    {
      g_key_file_set_string (_config_file,
                             "Tournament",
                             "backup_location",
                             foldername);
      SetBackupLocation (foldername);
      g_free (foldername);
    }
  }

  gtk_widget_destroy (chooser);
}

// --------------------------------------------------------------------------------
void Tournament::SetBackupLocation (gchar *location)
{
  if (location)
  {
    gchar *readable_name = g_filename_from_uri (location,
                                                NULL,
                                                NULL);

    gtk_menu_item_set_label (GTK_MENU_ITEM (_glade->GetWidget ("backup_location_menuitem")),
                             readable_name);
    g_free (readable_name);
  }
}

// --------------------------------------------------------------------------------
const gchar *Tournament::GetBackupLocation ()
{
  if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (_glade->GetWidget ("activate_radiomenuitem"))))
  {
    const gchar *location = gtk_menu_item_get_label (GTK_MENU_ITEM (_glade->GetWidget ("backup_location_menuitem")));

    if (g_file_test (location, G_FILE_TEST_IS_DIR))
    {
      return location;
    }
  }
  return NULL;
}

// --------------------------------------------------------------------------------
void Tournament::OnActivateBackup ()
{
  if (GetBackupLocation () == NULL)
  {
    OnBackupfileLocation ();
  }
}

// --------------------------------------------------------------------------------
void Tournament::OnLocaleToggled (GtkCheckMenuItem *checkmenuitem,
                                  gchar            *locale)
{
  if (gtk_check_menu_item_get_active (checkmenuitem))
  {
    g_key_file_set_string (_config_file,
                           "Tournament",
                           "interface_language",
                           locale);
  }
}

// --------------------------------------------------------------------------------
gboolean Tournament::OnLatestVersionReceived (Net::Downloader::CallbackData *cbk_data)
{
  Tournament *tournament = (Tournament *) cbk_data->_user_data;
  gchar      *data       = tournament->_version_downloader->GetData ();

  if (data)
  {
    GKeyFile *version_file = g_key_file_new ();

    if (g_key_file_load_from_data (version_file,
                                   data,
                                   strlen (data) + 1,
                                   G_KEY_FILE_NONE,
                                   NULL))
    {
      gchar    *version;
      gchar    *revision;
      gchar    *maturity;
      gboolean  new_version_detected = FALSE;

      version = g_key_file_get_string (version_file,
                                       VERSION_BRANCH,
                                       "version",
                                       NULL);
      revision = g_key_file_get_string (version_file,
                                        VERSION_BRANCH,
                                        "revision",
                                        NULL);
      maturity = g_key_file_get_string (version_file,
                                        VERSION_BRANCH,
                                        "maturity",
                                        NULL);

      if (version && (atoi (VERSION) < atoi (version)))
      {
        new_version_detected = TRUE;
      }
      else if (revision && (atoi (VERSION_REVISION) < atoi (revision)))
      {
        new_version_detected = TRUE;
      }
      else if (maturity && (strcmp (VERSION_MATURITY, maturity) < 0))
      {
        new_version_detected = TRUE;
      }
      else if (version && (strcmp (VERSION_BRANCH, "UNSTABLE") == 0))
      {
        char *stable_version = g_key_file_get_string (version_file,
                                                      "STABLE",
                                                      "version",
                                                      NULL);
        if (stable_version && strcmp (version, stable_version) <= 0)
        {
          new_version_detected = TRUE;
        }
      }

      if (new_version_detected)
      {
        gchar *label = g_strdup_printf ("%s.%s.%s", version, revision, maturity);

        gtk_menu_item_set_label (GTK_MENU_ITEM (tournament->_glade->GetWidget ("new_version_menuitem")),
                                 label);
        gtk_widget_show (tournament->_glade->GetWidget ("update_menuitem"));
        g_free (label);
      }
    }

    g_key_file_free (version_file);
  }

  tournament->_version_downloader->Kill    ();
  tournament->_version_downloader->Release ();
  tournament->_version_downloader = NULL;

  return FALSE;
}

// --------------------------------------------------------------------------------
Net::Uploader *Tournament::GetFtpUpLoader ()
{
  return _ecosystem->GetUpLoader ();
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_new_menuitem_activate (GtkWidget *w,
                                                          Object    *owner)
{
  Tournament *t = dynamic_cast <Tournament *> (owner);

  t->OnNew ();
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_open_menuitem_activate (GtkWidget *w,
                                                           Object    *owner)
{
  Tournament *t = dynamic_cast <Tournament *> (owner);

  t->OnOpen ();
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_save_menuitem_activate (GtkWidget *w,
                                                           Object    *owner)
{
  Tournament *t = dynamic_cast <Tournament *> (owner);

  t->OnSave ();
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_about_menuitem_activate (GtkWidget *w,
                                                           Object    *owner)
{
  Tournament *t = dynamic_cast <Tournament *> (owner);

  t->OnMenuDialog ("about_dialog");
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_example_menuitem_activate (GtkWidget *w,
                                                              Object    *owner)
{
  Tournament *t = dynamic_cast <Tournament *> (owner);

  t->OnOpenExample ();
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_user_manual_activate (GtkWidget *w,
                                                         Object    *owner)
{
  Tournament *t = dynamic_cast <Tournament *> (owner);

  t->OnOpenUserManual ();
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_template_imagemenuitem_activate (GtkWidget *w,
                                                                    Object    *owner)
{
  Tournament *t = dynamic_cast <Tournament *> (owner);

  t->OnOpenTemplate ();
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT gboolean on_root_delete_event (GtkWidget *w,
                                                          GdkEvent  *event,
                                                          Object    *owner)
{
  GtkWidget *dialog = gtk_message_dialog_new_with_markup (NULL,
                                                          GTK_DIALOG_MODAL,
                                                          GTK_MESSAGE_QUESTION,
                                                          GTK_BUTTONS_OK_CANCEL,
                                                          gettext ("<b><big>Do you really want to quit BellePoule</big></b>"));

  gtk_window_set_title (GTK_WINDOW (dialog),
                        gettext ("Quit BellePoule?"));

  gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                            gettext ("All the unsaved competions will be lost."));

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
  {
    Tournament *t = dynamic_cast <Tournament *> (owner);

    t->Release ();
    gtk_main_quit ();
  }
  else
  {
    gtk_widget_destroy (dialog);
  }

  return TRUE;
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_recent_menuitem_activate (GtkWidget *w,
                                                             Object    *owner)
{
  Tournament *t = dynamic_cast <Tournament *> (owner);

  t->OnRecent ();
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_translate_menuitem_activate (GtkWidget *w,
                                                                Object    *owner)
{
#ifdef WINDOWS_TEMPORARY_PATCH
  ShellExecuteA (NULL,
                 "open",
                 "http://betton.escrime.free.fr/index.php/developpement/translation-guidelines",
                 NULL,
                 NULL,
                 SW_SHOWNORMAL);
#else
  gtk_show_uri (NULL,
                "http://betton.escrime.free.fr/index.php/developpement/translation-guidelines",
                GDK_CURRENT_TIME,
                NULL);
#endif
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_network_config_menuitem_activate (GtkWidget *w,
                                                                     Object    *owner)
{
  Tournament *t = dynamic_cast <Tournament *> (owner);

  t->OnMenuDialog ("network_dialog");
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_ftp_menuitem_activate (GtkWidget *w,
                                                          Object    *owner)
{
  Tournament *t = dynamic_cast <Tournament *> (owner);

  t->OnMenuDialog ("publication_dialog");
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_scanner_menuitem_activate (GtkWidget *w,
                                                              Object    *owner)
{
  Tournament *t = dynamic_cast <Tournament *> (owner);

  t->OnMenuDialog ("scanner_dialog");
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_backup_location_menuitem_activate (GtkWidget *widget,
                                                                      Object    *owner)
{
  Tournament *t = dynamic_cast <Tournament *> (owner);

  t->OnBackupfileLocation ();
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_activate_radiomenuitem_toggled (GtkCheckMenuItem *checkmenuitem,
                                                                   Object           *owner)
{
  Tournament *t = dynamic_cast <Tournament *> (owner);

  if (gtk_check_menu_item_get_active (checkmenuitem))
  {
    t->OnActivateBackup ();
  }
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_root_drag_data_received (GtkWidget        *widget,
                                                            GdkDragContext   *context,
                                                            gint              x,
                                                            gint              y,
                                                            GtkSelectionData *selection_data,
                                                            guint             info,
                                                            guint             timestamp,
                                                            Object           *owner)
{
  Tournament *t = dynamic_cast <Tournament *> (owner);

  if (info == 100)
  {
    gchar **uris = g_uri_list_extract_uris ((gchar *) gtk_selection_data_get_data (selection_data));

    for (guint i = 0; uris[i] != NULL; i++)
    {
      gchar *filename = g_filename_from_uri (uris[i],
                                             NULL,
                                             NULL);

      t->OpenUriContest (filename);
      g_free (filename);
    }
    g_strfreev (uris);
  }
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_tickets_menuitem_activate (GtkMenuItem *menuitem,
                                                              Object      *owner)
{
  Tournament *t = dynamic_cast <Tournament *> (owner);

  t->PrintMealTickets ();
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_payment_menuitem_activate (GtkMenuItem *menuitem,
                                                              Object      *owner)
{
  Tournament *t = dynamic_cast <Tournament *> (owner);

  t->PrintPaymentBook ();
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_new_version_menuitem_activate (GtkMenuItem *menuitem,
                                                                  Object      *owner)
{
#ifdef WINDOWS_TEMPORARY_PATCH
  ShellExecuteA (NULL,
                 "open",
                 "http://betton.escrime.free.fr/index.php/bellepoule-telechargement",
                 NULL,
                 NULL,
                 SW_SHOWNORMAL);
#else
  gtk_show_uri (NULL,
                "http://betton.escrime.free.fr/index.php/bellepoule-telechargement",
                GDK_CURRENT_TIME,
                NULL);
#endif
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT gboolean on_root_key_press_event (GtkWidget   *widget,
                                                             GdkEventKey *event,
                                                             Object      *owner)
{
#ifdef DEBUG
  if (event->keyval == GDK_KEY_F11)
  {
    Object::DumpList ();
  }
#endif

  return FALSE;
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_QuickPoule_button_clicked (GtkButton *widget,
                                                              gpointer   user_data)
{
#ifdef WINDOWS_TEMPORARY_PATCH
  ShellExecute (NULL,
                "open",
                "https://play.google.com/store/apps/details?id=betton.escrime.quickpoule",
                NULL,
                NULL,
                SW_SHOWNORMAL);
#else
  gtk_show_uri (NULL,
                "https://play.google.com/store/apps/details?id=betton.escrime.quickpoule",
                GDK_CURRENT_TIME,
                NULL);
#endif
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_SmartPoule_button_clicked (GtkButton *widget,
                                                              gpointer   user_data)
{
#ifdef WINDOWS_TEMPORARY_PATCH
  ShellExecute (NULL,
                "open",
                "https://play.google.com/store/apps/details?id=betton.escrime.smartpoule",
                NULL,
                NULL,
                SW_SHOWNORMAL);
#else
  gtk_show_uri (NULL,
                "https://play.google.com/store/apps/details?id=betton.escrime.smartpoule",
                GDK_CURRENT_TIME,
                NULL);
#endif
}