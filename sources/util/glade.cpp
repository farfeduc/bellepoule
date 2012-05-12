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

#include "glade.hpp"

// --------------------------------------------------------------------------------
Glade::Glade (const gchar *file_name,
              Object      *owner)
: Object ("Glade")
{
  if (file_name)
  {
    GError *error = NULL;
    gchar  *path = g_build_filename (_program_path, "resources", "glade", file_name, NULL);

    _glade_xml = gtk_builder_new ();

    gtk_builder_add_from_file (_glade_xml,
                               path,
                               &error);
    if (error != NULL)
    {
      g_print ("<<%s>> %s\n", path, error->message);
    }
    g_free (path);

    if (error != NULL)
    {
      gchar *spare_file_name = g_build_filename (_program_path, "..", "..", "resources", "glade", file_name, NULL);

      g_clear_error (&error);
      gtk_builder_add_from_file (_glade_xml,
                                 spare_file_name,
                                 &error);
      g_free (spare_file_name);

      if (error != NULL)
      {
        g_print ("<<%s>> %s\n", spare_file_name, error->message);
        g_clear_error (&error);

        gtk_main_quit ();
      }
    }

    gtk_builder_connect_signals (_glade_xml,
                                 owner);
  }
  else
  {
    _glade_xml = NULL;
  }
}

// --------------------------------------------------------------------------------
Glade::~Glade ()
{
  if (_glade_xml)
  {
    g_object_unref (G_OBJECT (_glade_xml));
  }
}

// --------------------------------------------------------------------------------
void Glade::DetachFromParent (GtkWidget *widget)
{
  if (widget)
  {
    GtkWidget *parent = gtk_widget_get_parent (widget);

    if (parent)
    {
      gtk_container_forall (GTK_CONTAINER (parent),
                            (GtkCallback) g_object_ref,
                            NULL);
      gtk_container_remove (GTK_CONTAINER (parent),
                            widget);
    }
  }
}

// --------------------------------------------------------------------------------
GtkWidget *Glade::GetWidget (const gchar *name)
{
  return (GtkWidget *) (gtk_builder_get_object (_glade_xml,
                                                name));
}

// --------------------------------------------------------------------------------
GObject *Glade::GetObject (const gchar *name)
{
  return gtk_builder_get_object (_glade_xml,
                                 name);
}

// --------------------------------------------------------------------------------
GtkWidget *Glade::GetRootWidget ()
{
  return GTK_WIDGET (gtk_builder_get_object (_glade_xml,
                                             "root"));
}