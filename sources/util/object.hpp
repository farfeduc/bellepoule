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

#ifndef object_hpp
#define object_hpp

#include <glib/gi18n.h>
#include <gtk/gtk.h>

class Object
{
  public:
    Object (const gchar *class_name = NULL);

    void SetData (Object         *owner,
                  const gchar    *key,
                  void           *data,
                  GDestroyNotify  destroy_cbk = NULL);

    void *GetPtrData (Object      *owner,
                      const gchar *key);

    guint GetUIntData (Object      *owner,
                       const gchar *key);

    gint GetIntData (Object      *owner,
                     const gchar *key);

    void RemoveData (Object      *owner,
                     const gchar *key);

    void Retain ();

    void Release ();

    void RemoveAllData ();

    static void Dump ();

    static void TryToRelease (Object *object);

    static void SetProgramPath (gchar *path);

  protected:
    static gchar *_program_path;

    virtual ~Object ();

    static gchar *GetUndivadableText (const gchar *text);

  private:
    GData       *_datalist;
    guint        _ref_count;
    const gchar *_class_name;

    static GList *_list;
    static guint  _nb_objects;
};

#endif
