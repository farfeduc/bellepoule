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

#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <ctype.h>

#include <libxml/xmlwriter.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#include "attribute.hpp"
#include "player.hpp"
#include "filter.hpp"

#include "referees_list.hpp"

// --------------------------------------------------------------------------------
RefereesList::RefereesList ()
  : Checkin ("referees.glade",
             "Arbitre")
{
  {
    GSList *attr_list;
    Filter *filter;

    AttributeDesc::CreateList (&attr_list,
#ifndef DEBUG
                               "ref",
#endif
                               "start_rank",
                               "status",
                               "global_status",
                               "previous_stage_rank",
                               "exported",
                               "final_rank",
                               "victories_ratio",
                               "indice",
                               "pool_nr",
                               "HS",
                               "rank",
                               "ranking",
                               NULL);
    filter = new Filter (attr_list,
                         this);

    filter->ShowAttribute ("attending");
    filter->ShowAttribute ("participation_rate");
    filter->ShowAttribute ("name");
    filter->ShowAttribute ("first_name");
    filter->ShowAttribute ("level");
    filter->ShowAttribute ("club");
    filter->ShowAttribute ("league");
    filter->ShowAttribute ("country");
    filter->ShowAttribute ("birth_date");
    filter->ShowAttribute ("licence");

    SetFilter (filter);
    CreateForm (filter);
    filter->Release ();
  }
}

// --------------------------------------------------------------------------------
RefereesList::~RefereesList ()
{
}

// --------------------------------------------------------------------------------
void RefereesList::OnPrintRadioButtonToggled (GtkWidget *widget)
{
  GtkWidget *w = _glade->GetWidget ("list_alignment");

  gtk_widget_set_sensitive (w,
                            gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)));
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_print_radiobutton_toggled (GtkWidget *widget,
                                                              Object    *owner)
{
  RefereesList *r = dynamic_cast <RefereesList *> (owner);

  r->OnPrintRadioButtonToggled (widget);
}
