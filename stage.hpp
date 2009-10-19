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

#ifndef stage_hpp
#define stage_hpp

#include <libxml/xmlwriter.h>
#include <gtk/gtk.h>

#include "object.hpp"

class Player_c;

class Stage_c : public virtual Object_c
{
  public:
    typedef Stage_c * (*Creator) ();

    struct StageClass
    {
      const gchar      *_name;
      Stage_c::Creator  _creator;
    };

  public:
    void SetPrevious (Stage_c *previous);

    Stage_c *GetPreviousStage ();

    const gchar *GetClassName ();

    gchar *GetName ();

    gchar *GetFullName ();

    void SetName (gchar *name);

    gboolean Locked ();

    void Lock ();

    void UnLock ();

    GSList *GetResult ();

    Player_c *GetPlayerFromRef (guint ref);

    StageClass *GetClass ();

    virtual void Enter () {};

    virtual void Wipe () {};

    virtual void Load (xmlNode *xml_node) {};

    virtual void Save (xmlTextWriter *xml_writer) {};

    virtual void Dump ();

    virtual void ApplyConfig ();

    virtual void FillInConfig ();

    virtual gboolean CheckInputProvider (Stage_c *provider);

    virtual Stage_c *GetInputProvider ();

    static void RegisterStageClass (const gchar *name,
                                    Creator      creator);

    static guint  GetNbStageClass ();

    static void GetStageClass (guint    index,
                               gchar   **name,
                               Creator *creator);

    static Stage_c *CreateInstance (xmlNode *xml_node);

    static Stage_c *CreateInstance (const gchar *name);

  protected:
    GSList *_result;

    Stage_c (gchar *class_name);

    virtual ~Stage_c ();

  private:
    static GSList *_stage_base;

    Stage_c    *_previous;
    StageClass *_class;
    gchar      *_name;
    gboolean    _locked;

    void FreeResult ();
    virtual void OnLocked () {};
    virtual void OnUnLocked () {};
    static StageClass *GetClass (const gchar *name);
};

#endif
