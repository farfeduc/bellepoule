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

#include "network/message.hpp"
#include "slot.hpp"
#include "batch.hpp"
#include "job.hpp"
#include "job_details.hpp"

#include "job_board.hpp"

namespace Marshaller
{
  // --------------------------------------------------------------------------------
  JobBoard::JobBoard ()
    : Module ("job_board.glade")
  {
    _dialog          = _glade->GetWidget ("dialog");
    _job_list        = NULL;
    _referee_details = NULL;
    _fencer_details  = NULL;
  }

  // --------------------------------------------------------------------------------
  JobBoard::~JobBoard ()
  {
    Object::TryToRelease (_referee_details);
    Object::TryToRelease (_fencer_details);
  }

  // --------------------------------------------------------------------------------
  void JobBoard::Display (GList *job_list)
  {
    _job_list = job_list;

    _current_job = _job_list;

    DisplayCurrent ();

    gtk_dialog_run (GTK_DIALOG (_dialog));
    gtk_widget_hide (_dialog);
  }

  // --------------------------------------------------------------------------------
  void JobBoard::DisplayCurrent ()
  {
    if (_current_job)
    {
      Job  *job  = (Job *) _current_job->data;
      Slot *slot = job->GetSlot ();

      Object::TryToRelease (_referee_details);
      Object::TryToRelease (_fencer_details);

      {
        GtkLabel *title = GTK_LABEL (_glade->GetWidget ("current_job_label"));

        gtk_label_set_text (title, job->GetName ());
      }

      if (slot)
      {
        _referee_details = new JobDetails (slot->GetRefereeList ());

        Plug (_referee_details,
              _glade->GetWidget ("referee_detail_hook"));
      }

      {
        _fencer_details  = new JobDetails (job->GetFencerList ());

        Plug (_fencer_details,
              _glade->GetWidget ("fencer_detail_hook"));
      }
    }

    SetProperties ();
  }

  // --------------------------------------------------------------------------------
  void JobBoard::SetProperty (GQuark    key_id,
                              gchar    *data,
                              JobBoard *job_board)
  {
    const gchar *property        = g_quark_to_string (key_id);
    gchar       *property_widget = g_strdup_printf ("%s_label", property);
    GtkLabel    *label           = GTK_LABEL (job_board->_glade->GetGObject (property_widget));

    gtk_label_set_text (label,
                        gettext (data));

    g_free (property_widget);
  }

  // --------------------------------------------------------------------------------
  void JobBoard::SetProperties ()
  {
    Job   *job        = (Job *) _current_job->data;
    Batch *batch      = job->GetBatch ();
    GData *properties = batch->GetProperties ();

    // Title
    g_datalist_foreach (&properties,
                        (GDataForeachFunc) SetProperty,
                        this);

    // Color
    {
      GtkWidget *header_box = _glade->GetWidget ("header_box");

      gtk_widget_modify_bg (header_box,
                            GTK_STATE_NORMAL,
                            batch->GetColor ());
    }

    // Arrows
    {
      GtkWidget *arrow;

      arrow = _glade->GetWidget ("previous_job");
      gtk_widget_set_sensitive (arrow, g_list_previous (_current_job) != NULL);

      arrow = _glade->GetWidget ("next_job");
      gtk_widget_set_sensitive (arrow, g_list_next (_current_job) != NULL);
    }
  }

  // --------------------------------------------------------------------------------
  void JobBoard::OnPreviousClicked ()
  {
    _current_job = g_list_previous (_current_job);
    DisplayCurrent ();
  }

  // --------------------------------------------------------------------------------
  void JobBoard::OnNextClicked ()
  {
    _current_job = g_list_next (_current_job);
    DisplayCurrent ();
  }

  // --------------------------------------------------------------------------------
  extern "C" G_MODULE_EXPORT void on_previous_job_clicked (GtkWidget *widget,
                                                           Object    *owner)
  {
    JobBoard *jb = dynamic_cast <JobBoard *> (owner);

    jb->OnPreviousClicked ();
  }

  // --------------------------------------------------------------------------------
  extern "C" G_MODULE_EXPORT void on_next_job_clicked (GtkWidget *widget,
                                                       Object    *owner)
  {
    JobBoard *jb = dynamic_cast <JobBoard *> (owner);

    jb->OnNextClicked ();
  }
}