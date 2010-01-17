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

#ifndef module_hpp
#define module_hpp

#include <gtk/gtk.h>

#include "object.hpp"
#include "filter.hpp"
#include "glade.hpp"

class Module_c : public virtual Object_c
{
  public:
    void Plug (Module_c   *module,
               GtkWidget  *in,
               GtkToolbar *toolbar = NULL);
    void UnPlug ();

    void SelectAttributes ();

    GtkWidget *GetConfigWidget ();

    GtkWidget *GetRootWidget ();

    GtkWidget *GetWidget (gchar *name);

    virtual void SetFilter (Filter *filter);

    virtual void Print () {};

    virtual void OnAttrListUpdated () {};

  protected:
    Filter  *_filter;
    Glade_c *_glade;

    Module_c (gchar *glade_file,
              gchar *root = NULL);

    virtual void OnPlugged   () {};
    virtual void OnUnPlugged () {};

    GtkToolbar *GetToolbar ();

    void AddSensitiveWidget (GtkWidget *w);
    void EnableSensitiveWidgets ();
    void DisableSensitiveWidgets ();

    void SetCursor (GdkCursorType cursor_type);

    void ResetCursor ();

    virtual ~Module_c ();

  private:
    GtkWidget    *_root;
    GtkToolbar   *_toolbar;
    GSList       *_sensitive_widgets;
    GSList       *_plugged_list;
    Module_c     *_owner;
    GtkWidget    *_config_widget;
};

#endif
