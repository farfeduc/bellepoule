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

#define GETTEXT_PACKAGE "gtk20"
#include <glib/gi18n-lib.h>

#include <string.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#include "contest.hpp"
#include "player.hpp"
#include "tournament.hpp"

#include "splitting.hpp"

const gchar *Splitting::_class_name     = "splitting";
const gchar *Splitting::_xml_class_name = "splitting_stage";

Tournament *Splitting::_tournament = NULL;

// --------------------------------------------------------------------------------
Splitting::Splitting (StageClass *stage_class)
: Stage_c (stage_class),
  PlayersList ("splitting.glade",
               SORTABLE)
{
  // Sensitive widgets
  {
    AddSensitiveWidget (_glade->GetWidget ("splitting_move_toolbutton"));
  }

  {
    GSList *attr_list;
    Filter *filter;

    AttributeDesc::CreateList (&attr_list,
                               "ref",
                               "attending",
                               "victories_ratio",
                               "indice",
                               NULL);
    filter = new Filter (attr_list,
                         this);

    filter->ShowAttribute ("exported");
    filter->ShowAttribute ("rank");
    filter->ShowAttribute ("name");
    filter->ShowAttribute ("first_name");
    filter->ShowAttribute ("birth_year");
    filter->ShowAttribute ("gender");
    filter->ShowAttribute ("club");
    filter->ShowAttribute ("country");
    filter->ShowAttribute ("licence");

    SetFilter (filter);
    filter->Release ();
  }
}

// --------------------------------------------------------------------------------
Splitting::~Splitting ()
{
  Wipe ();
}

// --------------------------------------------------------------------------------
void Splitting::Init ()
{
  RegisterStageClass (_class_name,
                      _xml_class_name,
                      CreateInstance,
                      EDITABLE);
}

// --------------------------------------------------------------------------------
void Splitting::SetHostTournament (Tournament *tournament)
{
  _tournament = tournament;
}

// --------------------------------------------------------------------------------
Stage_c *Splitting::CreateInstance (StageClass *stage_class)
{
  return new Splitting (stage_class);
}

// --------------------------------------------------------------------------------
void Splitting::Save (xmlTextWriter *xml_writer)
{
  xmlTextWriterStartElement (xml_writer,
                             BAD_CAST _xml_class_name);

  Stage_c::SaveConfiguration (xml_writer);

  xmlTextWriterEndElement (xml_writer);
}

// --------------------------------------------------------------------------------
void Splitting::Wipe ()
{
  PlayersList::Wipe ();
}

// --------------------------------------------------------------------------------
void Splitting::Display ()
{
  for (guint i = 0; i < g_slist_length (_attendees); i ++)
  {
    Player_c *player;

    player = (Player_c *) g_slist_nth_data (_attendees,
                                            i);
    Add (player);
  }
  OnAttrListUpdated ();
}

// --------------------------------------------------------------------------------
void Splitting::OnLocked ()
{
  DisableSensitiveWidgets ();
}

// --------------------------------------------------------------------------------
GSList *Splitting::GetCurrentClassification ()
{
  GSList *result = CreateCustomList (PresentPlayerFilter);

  if (result)
  {
    result = g_slist_sort_with_data (result,
                                     (GCompareDataFunc) Player_c::Compare,
                                     (void *) "rank");
  }
  return result;
}

// --------------------------------------------------------------------------------
gboolean Splitting::PresentPlayerFilter (Player_c *player)
{
  Attribute_c *attr = player->GetAttribute ("exported");

  return ((gboolean) attr->GetValue () == FALSE);
}

// --------------------------------------------------------------------------------
void Splitting::OnUnLocked ()
{
  EnableSensitiveWidgets ();
}

// --------------------------------------------------------------------------------
void Splitting::OnMove ()
{
  GtkWidget *chooser = gtk_file_chooser_dialog_new (gettext ("Create or select a competition file ..."),
                                                    NULL,
                                                    GTK_FILE_CHOOSER_ACTION_SAVE,
                                                    GTK_STOCK_CANCEL,
                                                    GTK_RESPONSE_CANCEL,
                                                    GTK_STOCK_OPEN,
                                                    GTK_RESPONSE_ACCEPT,
                                                    NULL);

  {
    GtkFileFilter *filter = gtk_file_filter_new ();

    gtk_file_filter_set_name (filter,
                              "All BellePoule files (.cotcot)");
    gtk_file_filter_add_pattern (filter,
                                 "*.cotcot");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser),
                                 filter);
  }

  {
    GtkFileFilter *filter = gtk_file_filter_new ();

    gtk_file_filter_set_name (filter,
                              "All files");
    gtk_file_filter_add_pattern (filter,
                                 "*");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser),
                                 filter);
  }

  if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT)
  {
    Contest_c *contest;
    gchar     *filename;

    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
    gtk_widget_hide (chooser);

    if (filename)
    {
      if (strcmp ((const char *) ".cotcot", (const char *) &filename[strlen (filename) - strlen (".cotcot")]) != 0)
      {
        gchar *with_suffix;

        with_suffix = g_strdup_printf ("%s.cotcot", filename);
        g_free (filename);
        filename = with_suffix;
      }
    }

    contest = _tournament->GetContest (filename);
    if (contest == NULL)
    {
      contest = new Contest_c (filename);
      _tournament->Manage (contest);
      contest->Save ();
    }

    {
      GSList *selected_players = GetSelectedPlayers ();

      for (guint i = 0; i < g_slist_length (selected_players); i++)
      {
        Player_c    *player;
        Attribute_c *attr;

        player = (Player_c *) g_slist_nth_data (selected_players,
                                                i);
        attr = player->GetAttribute ("exported");

        if (attr && attr->GetValue () == FALSE)
        {
          player->SetAttributeValue ("exported",
                                     TRUE);
          Update (player);

          contest->AddPlayer (player);
        }
      }

      g_slist_free (selected_players);
    }
  }

  gtk_widget_destroy (chooser);
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_splitting_print_toolbutton_clicked (GtkWidget *widget,
                                                                       Object_c  *owner)
{
  Splitting *s = dynamic_cast <Splitting *> (owner);

  s->Print ();
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_splitting_filter_toolbutton_clicked (GtkWidget *widget,
                                                                        Object_c  *owner)
{
  Splitting *s = dynamic_cast <Splitting *> (owner);

  s->SelectAttributes ();
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_splitting_move_toolbutton_clicked (GtkWidget *widget,
                                                                      Object_c  *owner)
{
  Splitting *s = dynamic_cast <Splitting *> (owner);

  s->OnMove ();
}
