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

#include <stdlib.h>
#include <string.h>

#include "pool_allocator.hpp"
#include "pool_supervisor.hpp"
#include "classification.hpp"
#include "player.hpp"

#include "stage.hpp"

GSList *Stage_c::_stage_base = NULL;

// --------------------------------------------------------------------------------
Stage_c::Stage_c (StageClass *stage_class)
: Object_c ("Stage_c")
{
  _name                  = g_strdup ("");
  _locked                = false;
  _result                = NULL;
  _previous              = NULL;
  _stage_class           = stage_class;
  _attendees             = NULL;
  _classification        = NULL;
  _classification_filter = NULL;

  _sensitivity_trigger    = new SensitivityTrigger ();
  _score_stuffing_trigger = NULL;

  _status_cbk_data = NULL;
  _status_cbk      = NULL;
}

// --------------------------------------------------------------------------------
Stage_c::~Stage_c ()
{
  FreeResult ();
  g_free (_name);

  _sensitivity_trigger->Release ();
  TryToRelease (_score_stuffing_trigger);

  TryToRelease (_classification);

  TryToRelease (_classification_filter);
}

// --------------------------------------------------------------------------------
void Stage_c::SetStatusCbk (StatusCbk  cbk,
                            void      *data)
{
  _status_cbk_data = data;
  _status_cbk      = cbk;
}

// --------------------------------------------------------------------------------
void Stage_c::SignalStatusUpdate ()
{
  if (_status_cbk)
  {
    _status_cbk (this,
                 _status_cbk_data);
  }
}

// --------------------------------------------------------------------------------
const gchar *Stage_c::GetClassName ()
{
  return _class->_name;
}

// --------------------------------------------------------------------------------
gchar *Stage_c::GetName ()
{
  return _name;
}

// --------------------------------------------------------------------------------
gchar *Stage_c::GetFullName ()
{
  return g_strdup_printf ("%s\n%s", _class->_name, _name);
}

// --------------------------------------------------------------------------------
void Stage_c::SetName (gchar *name)
{
  if (name)
  {
    g_free (_name);
    _name = NULL;

    _name = g_strdup (name);
  }
}

// --------------------------------------------------------------------------------
void Stage_c::FreeResult ()
{
  g_slist_free (_result);
  _result = NULL;
}

// --------------------------------------------------------------------------------
void Stage_c::Lock ()
{
  _locked = true;
  OnLocked ();
  SetResult ();
}

// --------------------------------------------------------------------------------
void Stage_c::UnLock ()
{
  FreeResult ();

  // Reset the players rank
  {
    GSList *classification = GetCurrentClassification ();

    for (guint i = 0; i < g_slist_length (classification); i ++)
    {
      Player_c *player;

      player = (Player_c *) g_slist_nth_data (classification,
                                              i);
      player->SetAttributeValue ("rank",
                                 i + 1);
    }
    g_slist_free (classification);
  }

  _locked = false;
  OnUnLocked ();
}

// --------------------------------------------------------------------------------
gboolean Stage_c::Locked ()
{
  return _locked;
}

// --------------------------------------------------------------------------------
gboolean Stage_c::IsOver ()
{
  return TRUE;
}

// --------------------------------------------------------------------------------
void Stage_c::ApplyConfig ()
{
}

// --------------------------------------------------------------------------------
void Stage_c::FillInConfig ()
{
}

// --------------------------------------------------------------------------------
void Stage_c::SetPrevious (Stage_c *previous)
{
  _previous = previous;
}

// --------------------------------------------------------------------------------
Stage_c *Stage_c::GetPreviousStage ()
{
  return _previous;
}

// --------------------------------------------------------------------------------
void Stage_c::RetrieveAttendees ()
{
  if (_attendees)
  {
    g_slist_free (_attendees);
    _attendees = NULL;
  }

  if (_previous)
  {
    _attendees = g_slist_copy (_previous->_result);
  }
}

// --------------------------------------------------------------------------------
Player_c *Stage_c::GetPlayerFromRef (guint ref)
{
  for (guint p = 0; p < g_slist_length (_attendees); p++)
  {
    Player_c *player;

    player = (Player_c *) g_slist_nth_data (_attendees,
                                            p);
    if (player->GetRef () == ref)
    {
      return player;
    }
  }

  return NULL;
}

// --------------------------------------------------------------------------------
void Stage_c::RegisterStageClass (const gchar *name,
                                  const gchar *xml_name,
                                  Creator      creator,
                                  guint        rights)
{
  StageClass *stage_class = new StageClass;

  stage_class->_name     = name;
  stage_class->_xml_name = xml_name;
  stage_class->_creator  = creator;
  stage_class->_rights   = (Rights) rights;

  _stage_base = g_slist_append (_stage_base,
                                (void *) stage_class);
}

// --------------------------------------------------------------------------------
guint Stage_c::GetNbStageClass ()
{
  return g_slist_length (_stage_base);
}

// --------------------------------------------------------------------------------
void Stage_c::GetStageClass (guint    index,
                             gchar   **xml_name,
                             Creator *creator,
                             Rights  *rights)
{
  StageClass *stage_class = (StageClass *) g_slist_nth_data (_stage_base,
                                                             index);
  if (stage_class)
  {
    *xml_name = (gchar *) stage_class->_xml_name;
    *creator  = stage_class->_creator;
    *rights   = stage_class->_rights;
  }
}

// --------------------------------------------------------------------------------
Stage_c::StageClass *Stage_c::GetClass ()
{
  return _class;
}

// --------------------------------------------------------------------------------
Stage_c::StageClass *Stage_c::GetClass (const gchar *name)
{
  if (name)
  {
    for (guint i = 0; i < g_slist_length (_stage_base); i++)
    {
      StageClass *stage_class;

      stage_class = (StageClass *) g_slist_nth_data (_stage_base,
                                                     i);
      if (strcmp (name, stage_class->_xml_name) == 0)
      {
        return stage_class;
      }
    }
  }
  return NULL;
}

// --------------------------------------------------------------------------------
Stage_c *Stage_c::CreateInstance (xmlNode *xml_node)
{
  if (xml_node)
  {
    return CreateInstance ((gchar *) xml_node->name);
  }

  return NULL;
}

// --------------------------------------------------------------------------------
Stage_c *Stage_c::CreateInstance (const gchar *name)
{
  StageClass *stage_class = GetClass (name);

  if (stage_class)
  {
    Stage_c *stage = stage_class->_creator (stage_class);

    if (stage)
    {
      stage->_class = stage_class;
    }
    return stage;
  }

  return NULL;
}

// --------------------------------------------------------------------------------
Stage_c *Stage_c::GetInputProvider ()
{
  return NULL;
}

// --------------------------------------------------------------------------------
Stage_c::Rights Stage_c::GetRights ()
{
  return _stage_class->_rights;
}

// --------------------------------------------------------------------------------
void Stage_c::ToggleClassification (gboolean classification_on)
{
  Module_c *module = dynamic_cast <Module_c *> (this);

  if (module)
  {
    GtkWidget *main_w           = module->GetWidget ("main_hook");
    GtkWidget *classification_w = module->GetWidget ("classification_hook");

    if (classification_on)
    {
      if (_result == NULL)
      {
        GSList *result = GetCurrentClassification ();

        UpdateClassification (result);
        g_slist_free (result);
      }

      gtk_widget_hide (main_w);
      gtk_widget_show (classification_w);

      _sensitivity_trigger->SwitchOff ();
    }
    else
    {
      gtk_widget_hide (classification_w);
      gtk_widget_show (main_w);

      _sensitivity_trigger->SwitchOn ();
    }
  }
}

// --------------------------------------------------------------------------------
Classification *Stage_c::GetClassification ()
{
  return _classification;
}

// --------------------------------------------------------------------------------
void Stage_c::SetClassificationFilter (Filter *filter)
{
  _classification_filter = filter;
  _classification_filter->Retain ();
}

// --------------------------------------------------------------------------------
void Stage_c::UpdateClassification (GSList *result)
{
  Module_c  *module           = dynamic_cast <Module_c *> (this);
  GtkWidget *classification_w = module->GetWidget ("classification_hook");

  if (classification_w)
  {
    if (_classification == NULL)
    {
      _classification = new Classification (_classification_filter);

      _classification->SetDataOwner (this);
      module->Plug (_classification,
                    classification_w);
    }

    _classification->Wipe ();

    for (guint i = 0; i < g_slist_length (result); i ++)
    {
      Player_c *player;

      player = (Player_c *) g_slist_nth_data (result,
                                              i);

      player->SetAttributeValue ("rank",
                                 i + 1);
      _classification->Add (player);
    }
  }
}

// --------------------------------------------------------------------------------
void Stage_c::SetResult ()
{
  GSList *result = GetCurrentClassification ();

  UpdateClassification (result);

  FreeResult ();
  _result = result;
}

// --------------------------------------------------------------------------------
void Stage_c::LockOnClassification (GtkWidget *w)
{
  _sensitivity_trigger->AddWidget (w);
}

// --------------------------------------------------------------------------------
void Stage_c::SetScoreStuffingPolicy (gboolean allowed)
{
  if (_score_stuffing_trigger == NULL)
  {
    Module_c *module = dynamic_cast <Module_c *> (this);

    if (module)
    {
      _score_stuffing_trigger = new SensitivityTrigger ();

      _score_stuffing_trigger->AddWidget (module->GetWidget ("stuff_toolbutton"));
    }
  }

  if (allowed)
  {
    _score_stuffing_trigger->SwitchOn ();
  }
  else
  {
    _score_stuffing_trigger->SwitchOff ();
  }
}

// --------------------------------------------------------------------------------
void Stage_c::LoadConfiguration (xmlNode *xml_node)
{
  gchar *attr = (gchar *) xmlGetProp (xml_node,
                                      BAD_CAST "name");
  SetName (attr);
}

// --------------------------------------------------------------------------------
void Stage_c::SaveConfiguration (xmlTextWriter *xml_writer)
{
  xmlTextWriterWriteFormatAttribute (xml_writer,
                                     BAD_CAST "name",
                                     "%s", GetName ());
}

// --------------------------------------------------------------------------------
void Stage_c::Dump ()
{
  if (_result)
  {
    for (guint i = 0; i < g_slist_length (_result); i++)
    {
      Player_c    *player;
      Attribute_c *attr;

      player = (Player_c *) g_slist_nth_data (_result, i);
      attr = player->GetAttribute ("name");

      g_print ("%d >>> %s\n", i, attr->GetUserImage ());
    }
  }
}
