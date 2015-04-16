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

#include <string.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#include "util/global.hpp"
#include "application/contest.hpp"

#include "smart_swapper/smart_swapper.hpp"
#include "dispatcher/dispatcher.hpp"

#include "pool_allocator.hpp"

namespace Pool
{
#define VALUE_INIT {0,{{0}}}

  typedef enum
  {
    POOL_SIZE_COL,
    NB_POOLS_COL,
    BEST_PIXMAP_COL
  } ComboboxColumn;

  extern "C" G_MODULE_EXPORT void on_nb_pools_combobox_changed (GtkWidget *widget,
                                                                Object    *owner);
  extern "C" G_MODULE_EXPORT void on_pool_size_combobox_changed (GtkWidget *widget,
                                                                 Object    *owner);

  const gchar *Allocator::_class_name     = N_ ("Pools arrangement");
  const gchar *Allocator::_xml_class_name = "";

  // --------------------------------------------------------------------------------
  Allocator::Allocator (StageClass *stage_class)
    : Stage (stage_class),
    CanvasModule ("pool_allocator.glade")
  {
    _config_list            = NULL;
    _selected_config        = NULL;
    _main_table             = NULL;
    _swapping_criteria_list = NULL;
    _loaded                 = FALSE;
    _nb_matchs              = 0;

    _max_score = new Data ("ScoreMax",
                           5);

    _seeding_balanced = new Data ("RepartitionEquilibre",
                                  TRUE);

    _swapping = new Data ("Decalage",
                          (gchar *) NULL);

    _combobox_store = GTK_LIST_STORE (_glade->GetGObject ("combo_liststore"));

    // Sensitive widgets
    {
      AddSensitiveWidget (_glade->GetWidget ("nb_pools_combobox"));
      AddSensitiveWidget (_glade->GetWidget ("pool_size_combobox"));
      AddSensitiveWidget (_glade->GetWidget ("swapping_hbox"));

      _swapping_sensitivity_trigger.AddWidget (_glade->GetWidget ("swapping_hbox"));
    }

    // Dnd
    {
      _dnd_target = _dnd_config->CreateTarget ("bellepoule/referee", GTK_TARGET_SAME_APP|GTK_TARGET_OTHER_WIDGET);
      _dnd_config->CreateTargetTable ();
    }

    {
      GSList *attr_list;
      Filter *filter;

      AttributeDesc::CreateExcludingList (&attr_list,
#ifndef DEBUG
                                          "ref",
#endif
                                          "IP",
                                          "HS",
                                          "attending",
                                          "availability",
                                          "exported",
                                          "final_rank",
                                          "global_status",
                                          "indice",
                                          "level",
                                          "participation_rate",
                                          "pool_nr",
                                          "promoted",
                                          "rank",
                                          "status",
                                          "team",
                                          "victories_count",
                                          "bouts_count",
                                          "victories_ratio",
                                          NULL);
      filter = new Filter (attr_list,
                           this);

      filter->ShowAttribute ("stage_start_rank");
      filter->ShowAttribute ("name");
      filter->ShowAttribute ("first_name");
      filter->ShowAttribute ("club");

      SetFilter (filter);
      filter->Release ();
    }

    _swapper = SmartSwapper::SmartSwapper::Create (this);

    {
      GtkContainer *swapping_hbox = GTK_CONTAINER (_glade->GetGObject ("swapping_criteria_hbox"));
      GSList       *attr          = AttributeDesc::GetSwappableList ();

      while (attr)
      {
        AttributeDesc *attr_desc = (AttributeDesc *) attr->data;

        if (attr_desc->_uniqueness == AttributeDesc::NOT_SINGULAR)
        {
          GtkWidget *check = gtk_check_button_new_with_label (attr_desc->_user_name);

          g_signal_connect (check, "toggled",
                            G_CALLBACK (OnSwappingToggled), this);
          g_object_set_data (G_OBJECT (check),
                             "criteria_attribute", attr_desc);
          gtk_container_add (swapping_hbox,
                             check);
        }

        attr = g_slist_next (attr);
      }
    }

    {
      _fencer_list = new People::PlayersList ("classification.glade",
                                              People::PlayersList::SORTABLE);
      {
        GSList *attr_list;
        Filter *filter;

        AttributeDesc::CreateExcludingList (&attr_list,
#ifndef DEBUG
                                            "ref",
#endif
                                            "IP",
                                            "HS",
                                            "attending",
                                            "availability",
                                            "exported",
                                            "final_rank",
                                            "global_status",
                                            "indice",
                                            "level",
                                            "participation_rate",
                                            "promoted",
                                            "rank",
                                            "status",
                                            "team",
                                            "victories_count",
                                            "bouts_count",
                                            "victories_ratio",
                                            NULL);
        filter = new Filter (attr_list,
                             _fencer_list);

        filter->ShowAttribute ("name");
        filter->ShowAttribute ("first_name");
        filter->ShowAttribute ("club");
        filter->ShowAttribute ("pool_nr");

        _fencer_list->SetFilter    (filter);
        _fencer_list->SetDataOwner (this);
        filter->Release ();
      }
    }
  }

  // --------------------------------------------------------------------------------
  Allocator::~Allocator ()
  {
    _max_score->Release        ();
    _swapping->Release         ();
    _seeding_balanced->Release ();
    _fencer_list->Release      ();
    _swapper->Delete           ();
  }

  // --------------------------------------------------------------------------------
  void Allocator::Declare ()
  {
    RegisterStageClass (gettext (_class_name),
                        _xml_class_name,
                        CreateInstance);
  }

  // --------------------------------------------------------------------------------
  Stage *Allocator::CreateInstance (StageClass *stage_class)
  {
    return new Allocator (stage_class);
  }

  // --------------------------------------------------------------------------------
  const gchar *Allocator::GetInputProviderClient ()
  {
    return "pool_stage";
  }

  // --------------------------------------------------------------------------------
  void Allocator::OnPlugged ()
  {
    CanvasModule::OnPlugged ();

    gtk_drag_dest_set (GTK_WIDGET (GetCanvas ()),
                       (GtkDestDefaults) 0,
                       _dnd_config->GetTargetTable (),
                       _dnd_config->GetTargetTableSize (),
                       GDK_ACTION_COPY);

    ConnectDndDest (GTK_WIDGET (GetCanvas ()));
    EnableDragAndDrop ();

    _owner->Plug (_fencer_list,
                  _glade->GetWidget ("fencer_list_hook"));
  }

  // --------------------------------------------------------------------------------
  void Allocator::OnUnPlugged ()
  {
    CanvasModule::OnUnPlugged ();

    _fencer_list->UnPlug ();
  }

  // --------------------------------------------------------------------------------
  Object *Allocator::GetDropObjectFromRef (guint32 ref)
  {
    return _contest->GetRefereeFromDndRef (ref);
  }

  // --------------------------------------------------------------------------------
  void Allocator::DropObject (Object   *object,
                              DropZone *source_zone,
                              DropZone *target_zone)
  {
    Player   *floating_object  = (Player *) object;
    PoolZone *source_pool_zone = NULL;
    PoolZone *target_pool_zone = NULL;

    if (source_zone)
    {
      source_pool_zone = (PoolZone *) source_zone;

      if (floating_object->Is ("Referee"))
      {
        source_pool_zone->RemoveObject (floating_object);
      }
    }

    if (target_zone)
    {
      target_pool_zone = (PoolZone *) target_zone;

      if (floating_object->Is ("Referee") == FALSE)
      {
        if (source_pool_zone)
        {
          Pool *source_pool = source_pool_zone->GetPool ();

          source_pool->RemoveFencer (floating_object);
        }

        {
          Pool *target_pool = target_pool_zone->GetPool ();

          target_pool->AddFencer (floating_object,
                                  this);
        }

        _fencer_list->Update (floating_object);
      }
      else
      {
        target_pool_zone->AddObject (floating_object);

        if (Locked ())
        {
          Module *next_stage = dynamic_cast <Module *> (GetNextStage ());

          if (next_stage)
          {
            next_stage->OnAttrListUpdated ();
          }
        }
      }
    }

    if (source_pool_zone || target_pool_zone)
    {
      FillPoolTable (source_pool_zone);
      FillPoolTable (target_pool_zone);
      SignalStatusUpdate ();
      FixUpTablesBounds ();
    }
  }

  // --------------------------------------------------------------------------------
  gboolean Allocator::DroppingIsForbidden (Object *object)
  {
    Player *player = (Player *) object;

    if (player && player->Is ("Referee"))
    {
      return FALSE;
    }

    return Locked ();
  }

  // --------------------------------------------------------------------------------
  GString *Allocator::GetFloatingImage (Object *floating_object)
  {
    GString *string = g_string_new ("");

    if (floating_object)
    {
      Player *player        = (Player *) floating_object;
      GSList *layout_list = NULL;

      if (_filter)
      {
        layout_list = _filter->GetLayoutList ();
      }

      while (layout_list)
      {
        Filter::Layout       *attr_layout = (Filter::Layout *) layout_list->data;
        Attribute            *attr;
        Player::AttributeId  *attr_id;

        attr_id = Player::AttributeId::Create (attr_layout->_desc, this);
        attr = player->GetAttribute (attr_id);
        attr_id->Release ();

        if (attr)
        {
          gchar *image = attr->GetUserImage (attr_layout->_look);

          string = g_string_append (string,
                                    image);
          string = g_string_append (string,
                                    "  ");
          g_free (image);
        }
        layout_list = g_slist_next (layout_list);
      }
    }

    return string;
  }

  // --------------------------------------------------------------------------------
  gboolean Allocator::ObjectIsDropable (Object   *floating_object,
                                        DropZone *in_zone)
  {
    if (floating_object && in_zone)
    {
      return TRUE;
    }

    return FALSE;
  }

  // --------------------------------------------------------------------------------
  void Allocator::RefreshDisplay ()
  {
    if (_main_table && (Locked () == FALSE))
    {
      SetCursor (GDK_WATCH);

      DeletePools ();
      CreatePools ();
      Display ();
      SignalStatusUpdate ();
      MakeDirty ();

      ResetCursor ();
    }
  }

  // --------------------------------------------------------------------------------
  void Allocator::Display ()
  {
    SetUpCombobox ();

    {
      OnFencerListToggled (gtk_toggle_tool_button_get_active (GTK_TOGGLE_TOOL_BUTTON (_glade->GetWidget ("fencer_list"))));

      PopulateFencerList ();
    }

    if (_main_table)
    {
      goo_canvas_item_remove (_main_table);
    }

    _max_w = 0;
    _max_h = 0;

    {
      GooCanvasItem *root = GetRootItem ();

      if (root)
      {
        _main_table = goo_canvas_group_new (root,
                                            NULL);

        {
          GSList *current = _drop_zones;

          while (current)
          {
            FillPoolTable ((PoolZone *) current->data);

            current = g_slist_next (current);
          }
        }
      }
    }
    FixUpTablesBounds ();

    SignalStatusUpdate ();
  }

  // --------------------------------------------------------------------------------
  void Allocator::Garnish ()
  {
    if (_drop_zones == NULL)
    {
      Setup ();

      if (_best_config)
      {
        _selected_config = _best_config;

        CreatePools ();
      }
    }
  }

  // --------------------------------------------------------------------------------
  void Allocator::ApplyConfig ()
  {
    Module *next_stage = dynamic_cast <Module *> (GetNextStage ());

    if (next_stage)
    {
      GtkToggleButton *toggle = GTK_TOGGLE_BUTTON (next_stage->GetGObject ("balanced_radiobutton"));

      if (toggle)
      {
        guint seeding_is_balanced = gtk_toggle_button_get_active (toggle);

        if (_seeding_balanced->_value != seeding_is_balanced)
        {
          _seeding_balanced->_value = seeding_is_balanced;

          if (seeding_is_balanced)
          {
            _swapping_sensitivity_trigger.SwitchOn ();
          }
          else
          {
            _swapping_sensitivity_trigger.SwitchOff ();

            g_slist_free (_swapping_criteria_list);
            _swapping_criteria_list = NULL;

            {
              GtkContainer *swapping_hbox = GTK_CONTAINER (_glade->GetGObject ("swapping_criteria_hbox"));
              GList        *siblings      = gtk_container_get_children (swapping_hbox);

              while (siblings)
              {
                GtkToggleButton *togglebutton = GTK_TOGGLE_BUTTON (siblings->data);

                g_signal_handlers_disconnect_by_func (G_OBJECT (togglebutton),
                                                      (void *) OnSwappingToggled,
                                                      (Object *) this);
                gtk_toggle_button_set_active (togglebutton,
                                              FALSE);
                g_signal_connect (G_OBJECT (togglebutton), "toggled",
                                  G_CALLBACK (OnSwappingToggled), this);

                siblings = g_list_next (siblings);
              }
            }
          }
        }
      }

      RefreshDisplay ();
    }
  }

  // --------------------------------------------------------------------------------
  void Allocator::FillInConfig ()
  {
    Module *next_stage = dynamic_cast <Module *> (GetNextStage ());

    if (next_stage)
    {
      GtkToggleButton *w;

      if (_seeding_balanced->_value)
      {
        w = GTK_TOGGLE_BUTTON (next_stage->GetGObject ("balanced_radiobutton"));
      }
      else
      {
        w = GTK_TOGGLE_BUTTON (next_stage->GetGObject ("strength_radiobutton"));
      }

      if (w)
      {
        gtk_toggle_button_set_active (w,
                                      TRUE);
      }
    }
  }

  // --------------------------------------------------------------------------------
  void Allocator::LoadConfiguration (xmlNode *xml_node)
  {
    Stage::LoadConfiguration (xml_node);

    if (_seeding_balanced)
    {
      _seeding_balanced->Load (xml_node);

      if (_seeding_balanced->_value == FALSE)
      {
        _swapping_sensitivity_trigger.SwitchOff ();
      }
    }

    if (_swapping)
    {
      _swapping->Load (xml_node);

      if (_swapping->GetString ())
      {
        gchar **tokens = g_strsplit_set (_swapping->GetString (),
                                         "/",
                                         0);

        if (tokens)
        {
          GtkContainer *swapping_hbox = GTK_CONTAINER (_glade->GetGObject ("swapping_criteria_hbox"));
          GList        *siblings      = gtk_container_get_children (swapping_hbox);

          for (guint i = 0; tokens[i] != NULL; i++)
          {
            if (*tokens[i] != '\0')
            {
              GList *current = siblings;

              while (current)
              {
                GtkToggleButton *togglebutton = GTK_TOGGLE_BUTTON (current->data);
                AttributeDesc   *attr_desc;

                attr_desc = (AttributeDesc *) g_object_get_data (G_OBJECT (togglebutton), "criteria_attribute");

                if (strcmp (tokens[i], attr_desc->_code_name) == 0)
                {
                  _swapping_criteria_list = g_slist_append (_swapping_criteria_list,
                                                            attr_desc);

                  gtk_toggle_button_set_active (togglebutton,
                                                TRUE);
                }

                current = g_list_next (current);
              }
            }
          }

          g_strfreev (tokens);
        }
      }
    }
  }

  // --------------------------------------------------------------------------------
  void Allocator::SaveConfiguration (xmlTextWriter *xml_writer)
  {
    Stage::SaveConfiguration (xml_writer);

    if (_seeding_balanced)
    {
      _seeding_balanced->Save (xml_writer);
    }

    if (_swapping)
    {
      _swapping->Save (xml_writer);
    }
  }

  // --------------------------------------------------------------------------------
  void Allocator::Load (xmlNode *xml_node)
  {
    static Pool     *current_pool     = NULL;
    static PoolZone *current_zone     = NULL;
    static guint     nb_pool          = 0;
    Stage           *next_stage       = GetNextStage ();
    StageClass      *next_stage_class = next_stage->GetClass ();

    for (xmlNode *n = xml_node; n != NULL; n = n->next)
    {
      if (n->type == XML_ELEMENT_NODE)
      {
        if (   (_loaded == FALSE)
            && strcmp ((char *) n->name, next_stage_class->_xml_name) == 0)
        {
          LoadConfiguration (xml_node);

          {
            gchar *string = (gchar *) xmlGetProp (xml_node,
                                                  BAD_CAST "NbDePoules");
            if (string)
            {
              nb_pool = (guint) atoi (string);
              xmlFree (string);
            }
          }
          _loaded = TRUE;
        }
        else if (strcmp ((char *) n->name, "Poule") == 0)
        {
          if (_config_list == NULL)
          {
            Setup ();

            // Get the configuration
            {
              GSList *current = _config_list;

              while (current)
              {
                _selected_config = (Configuration *) current->data;

                if (_selected_config->_nb_pool == nb_pool)
                {
                  break;
                }
                current = g_slist_next (current);
              }
            }

            // Assign players an original pool
            {
              CreatePools ();
              DeletePools ();
            }

            {
              _nb_matchs = GetNbMatchs ();
              RefreshMatchRate (_nb_matchs);
            }
          }

          {
            guint number = g_slist_length (_drop_zones);

            current_pool = new Pool (_max_score,
                                     number+1,
                                     GetXmlPlayerTag (),
                                     _rand_seed);
            SetPoolFlashRef (current_pool);

            {
              current_zone = new PoolZone (this,
                                           current_pool);

              _drop_zones = g_slist_append (_drop_zones,
                                            current_zone);
            }
          }
        }
        else if (strcmp ((char *) n->name, GetXmlPlayerTag ()) == 0)
        {
          if (current_pool == NULL)
          {
            LoadAttendees (n);
          }
          else
          {
            gchar *attr = (gchar *) xmlGetProp (n, BAD_CAST "REF");

            if (attr)
            {
              Player *player = GetFencerFromRef (atoi (attr));

              if (player)
              {
                current_pool->AddFencer (player,
                                         this);
              }
              xmlFree (attr);
            }
          }
        }
        else if (strcmp ((char *) n->name, "Arbitre") == 0)
        {
          gchar *attr = (gchar *) xmlGetProp (n, BAD_CAST "REF");

          if (attr)
          {
            Player *referee = _contest->GetRefereeFromDndRef (atoi (attr));

            if (referee)
            {
              current_zone->AddObject (referee);
            }
            xmlFree (attr);
          }
        }
        else if (strcmp ((char *) n->name, "Match") == 0)
        {
          current_pool->CreateMatchs (_swapping_criteria_list);
          current_pool->Load (n,
                              _attendees->GetShortList ());
          current_pool = NULL;
          return;
        }
        else
        {
          current_pool = NULL;
          return;
        }
      }

      Load (n->children);
    }
  }

  // --------------------------------------------------------------------------------
  void Allocator::SaveHeader (xmlTextWriter *xml_writer)
  {
    Stage      *next_stage       = GetNextStage ();
    StageClass *next_stage_class = next_stage->GetClass ();

    xmlTextWriterStartElement (xml_writer,
                               BAD_CAST next_stage_class->_xml_name);

    SaveConfiguration (xml_writer);

    if (_drop_zones)
    {
      xmlTextWriterWriteFormatAttribute (xml_writer,
                                         BAD_CAST "NbDePoules",
                                         "%d", g_slist_length (_drop_zones));
    }
    xmlTextWriterWriteFormatAttribute (xml_writer,
                                       BAD_CAST "PhaseSuivanteDesQualifies",
                                       "%d", GetId ()+2);
  }

  // --------------------------------------------------------------------------------
  void Allocator::Save (xmlTextWriter *xml_writer)
  {
    SaveHeader (xml_writer);

    Stage::SaveAttendees (xml_writer);

    {
      GSList *current = _drop_zones;

      while (current)
      {
        Pool *pool = GetPoolOf (current);

        pool->Save (xml_writer);
        current = g_slist_next (current);
      }
    }

    xmlTextWriterEndElement (xml_writer);
  }

  // --------------------------------------------------------------------------------
  void Allocator::RegisterConfig (Configuration *config)
  {
    if (config)
    {
      GtkTreeIter iter;

      _config_list = g_slist_append (_config_list,
                                     config);
      gtk_list_store_append (_combobox_store, &iter);

      {
        gchar *nb_pool_text = g_strdup_printf ("%d", config->_nb_pool);

        gtk_list_store_set (_combobox_store, &iter,
                            NB_POOLS_COL, nb_pool_text,
                            -1);
        g_free (nb_pool_text);
      }

      {
        gchar *pool_size_text;

        if (config->_nb_overloaded)
        {
          pool_size_text = g_strdup_printf ("%d %s %d", config->_size, gettext ("or"), config->_size+1);
        }
        else
        {
          pool_size_text = g_strdup_printf ("%d", config->_size);
        }

        gtk_list_store_set (_combobox_store, &iter,
                            POOL_SIZE_COL, pool_size_text,
                            -1);
        g_free (pool_size_text);
      }

      if (   (config->_size < 7)
          || ((config->_size == 7) && (config->_nb_overloaded == 0)))
      {
        _best_config = config;
      }

      config = NULL;
    }
  }

  // --------------------------------------------------------------------------------
  void Allocator::Setup ()
  {
    guint           nb_players    = g_slist_length (_attendees->GetShortList ());
    Configuration  *config        = NULL;
    guint           max_pool_size = Dispatcher::_MAX_POOL_SIZE;

    if (nb_players % Dispatcher::_MAX_POOL_SIZE)
    {
      max_pool_size = Dispatcher::_MAX_POOL_SIZE -1;
    }

    _best_config = NULL;
    for (guint size = 2; size <= MIN (max_pool_size, nb_players); size++)
    {
      if (nb_players%size == 0)
      {
        config = (Configuration *) g_malloc (sizeof (Configuration));

        config->_nb_pool       = nb_players/size;
        config->_size          = size;
        config->_nb_overloaded = 0;

        RegisterConfig (config);
      }

      if (   (nb_players%size != 0)
          || ((size == 6) && (nb_players%7 != 0)))
      {
        for (guint p = 1; p < nb_players/size; p++)
        {
          if ((nb_players - size*p) % (size + 1) == 0)
          {
            config = (Configuration *) g_malloc (sizeof (Configuration));

            config->_nb_pool       = p + (nb_players - size*p) / (size + 1);
            config->_size          = size;
            config->_nb_overloaded = (nb_players - size*p) / (size+1);

            RegisterConfig (config);
            break;
          }
        }
      }
    }

    if (_best_config)
    {
      GtkTreeIter iter;
      gint        best_index;

      best_index = g_slist_index (_config_list,
                                  _best_config);

      if (gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (_combobox_store),
                                         &iter,
                                         NULL,
                                         best_index))
      {
        gtk_list_store_set (_combobox_store, &iter,
                            BEST_PIXMAP_COL, GTK_STOCK_ABOUT,
                            -1);
      }
    }
  }

  // --------------------------------------------------------------------------------
  void Allocator::PopulateFencerList ()
  {
    if (_attendees)
    {
      GSList *current_player = _attendees->GetShortList ();

      _fencer_list->Wipe ();
      while (current_player)
      {
        _fencer_list->Add ((Player *) current_player->data);
        current_player = g_slist_next (current_player);
      }
      _fencer_list->OnAttrListUpdated ();
    }
  }

  // --------------------------------------------------------------------------------
  void Allocator::SetPoolFlashRef (Pool *pool)
  {
    gchar *ref = g_strdup_printf ("#%s/%d/%d",
                                  _contest->GetId (),
                                  GetId () + 1,
                                  pool->GetNumber ());

    pool->SetFlashRef (ref);
    g_free (ref);
  }

  // --------------------------------------------------------------------------------
  void Allocator::CreatePools ()
  {
    GtkWidget *swapping_hbox = _glade->GetWidget ("swapping_hbox");

    gtk_widget_set_tooltip_text (swapping_hbox,
                                 "");

    if (_selected_config)
    {
      Pool   **pool_table;
      GSList  *shortlist  = _attendees->GetShortList ();
      guint    nb_fencer  = g_slist_length (shortlist);
      guint    nb_pool    = _selected_config->_nb_pool;

      pool_table = g_new (Pool *, nb_pool);
      for (guint i = 0; i < nb_pool; i++)
      {
        pool_table[i] = new Pool (_max_score,
                                  i+1,
                                  GetXmlPlayerTag (),
                                  _rand_seed);
        SetPoolFlashRef (pool_table[i]);

        {
          PoolZone *zone = new PoolZone (this,
                                         pool_table[i]);

          _drop_zones = g_slist_append (_drop_zones,
                                        zone);
        }
      }

      if (_seeding_balanced->_value)
      {
        _swapper->Init (_drop_zones,
                        nb_fencer);
        if (_swapping_criteria_list)
        {
          _swapper->Swap (_swapping_criteria_list,
                          shortlist);
        }
        else
        {
          _swapper->Swap (NULL,
                          shortlist);
        }

        {
          gchar *figures = g_strdup_printf ("Moved : %d",
                                            _swapper->GetMoved ());

          gtk_widget_set_tooltip_text (swapping_hbox,
                                       figures);
          g_free (figures);
        }
      }
      else
      {
        Pool *pool = pool_table[0];

        for (guint i = 0; i < nb_fencer; i++)
        {
          Player *player = (Player *) g_slist_nth_data (shortlist,
                                                        i);

          if (_selected_config->_nb_overloaded)
          {
            if (pool->GetNumber () <= _selected_config->_nb_overloaded)
            {
              if (pool->GetNbPlayers () >= _selected_config->_size+1)
              {
                pool = pool_table[pool->GetNumber ()];
              }
            }
            else if (pool->GetNbPlayers () >= _selected_config->_size)
            {
              pool = pool_table[pool->GetNumber ()];
            }
          }
          else if (pool->GetNbPlayers () >= _selected_config->_size)
          {
            pool = pool_table[pool->GetNumber ()];
          }

          player->SetData (this,
                           "original_pool",
                           (void *) pool->GetNumber ());
          player->RemoveData (this,
                              "swapped_from");
          pool->AddFencer (player,
                           this);
        }
      }
      g_free (pool_table);

      {
        _nb_matchs = GetNbMatchs ();
        RefreshMatchRate (_nb_matchs);
      }
    }
  }

  // --------------------------------------------------------------------------------
  guint Allocator::GetBiggestPoolSize ()
  {
    if (_selected_config)
    {
      if (_selected_config->_nb_overloaded)
      {
        return _selected_config->_size + 1;
      }
      return _selected_config->_size;
    }

    return 0;
  }

  // --------------------------------------------------------------------------------
  gint Allocator::GetNbMatchs ()
  {
    gint nb_matchs = 0;

    if (_selected_config)
    {
      nb_matchs  = _selected_config->_size * (_selected_config->_size - 1) / 2;
      nb_matchs *= _selected_config->_nb_pool - _selected_config->_nb_overloaded;
      nb_matchs += _selected_config->_nb_overloaded * _selected_config->_size * (_selected_config->_size + 1) / 2;
    }

    return nb_matchs;
  }

  // --------------------------------------------------------------------------------
  void Allocator::SetUpCombobox ()
  {
    if (_best_config)
    {
      GtkWidget *w;
      gint       config_index;

      config_index = g_slist_index (_config_list,
                                    _selected_config);

      w = _glade->GetWidget ("nb_pools_combobox");
      g_signal_handlers_disconnect_by_func (G_OBJECT (w),
                                            (void *) on_nb_pools_combobox_changed,
                                            (Object *) this);
      gtk_combo_box_set_active (GTK_COMBO_BOX (w),
                                config_index);
      g_signal_connect (G_OBJECT (w), "changed",
                        G_CALLBACK (on_nb_pools_combobox_changed),
                        (Object *) this);

      w = _glade->GetWidget ("pool_size_combobox");
      g_signal_handlers_disconnect_by_func (G_OBJECT (w),
                                            (void *) on_pool_size_combobox_changed,
                                            (Object *) this);
      gtk_combo_box_set_active (GTK_COMBO_BOX (w),
                                config_index);
      g_signal_connect (G_OBJECT (w), "changed",
                        G_CALLBACK (on_pool_size_combobox_changed),
                        (Object *) this);
    }
  }

  // --------------------------------------------------------------------------------
  void Allocator::FixUpTablesBounds ()
  {
    if (_selected_config)
    {
      GSList *current = _drop_zones;

      while (current)
      {
        PoolZone        *zone  = (PoolZone *) current->data;
        Pool            *pool  = GetPoolOf (current);
        GooCanvasItem   *table = (GooCanvasItem *) zone->GetPtrData (this, "table");
        GooCanvasBounds  bounds;

        {
          guint nb_columns = 2;
          guint nb_rows    = _selected_config->_nb_pool / nb_columns;

          if (_selected_config->_nb_pool % nb_columns != 0)
          {
            nb_rows++;
          }

          {
            guint row    = (pool->GetNumber () - 1) % nb_rows;
            guint column = (pool->GetNumber () - 1) / nb_rows;

            goo_canvas_item_get_bounds (table,
                                        &bounds);
            goo_canvas_item_translate (table,
                                       -bounds.x1,
                                       -bounds.y1);
            goo_canvas_item_translate (table,
                                       column * (_max_w + 40.0),
                                       row * (_max_h + 20.0));
          }
        }

        goo_canvas_item_get_bounds (table,
                                    &bounds);

        zone->Redraw (bounds.x1,
                      bounds.y1,
                      bounds.x2 - bounds.x1,
                      bounds.y2 - bounds.y1);

        current = g_slist_next (current);
      }
    }
  }

  // --------------------------------------------------------------------------------
  void Allocator::FillPoolTable (PoolZone *zone)
  {
    if ((zone == NULL) || (_main_table == NULL))
    {
      return;
    }

    GooCanvasItem *item;
    GooCanvasItem *table       = (GooCanvasItem *) zone->GetPtrData (this, "table");
    GSList        *layout_list = NULL;
    Pool          *pool        = zone->GetPool ();

    if (_filter)
    {
      layout_list = _filter->GetLayoutList ();
    }

    if (table)
    {
      goo_canvas_item_remove (table);
    }

    {
      table = goo_canvas_table_new (_main_table,
                                    "row-spacing",    2.0,
                                    "column-spacing", 4.0,
                                    NULL);
      zone->SetData (this, "table",
                     table);
    }

    {
      guint pool_size           = pool->GetNbPlayers ();
      GooCanvasItem *name_table = goo_canvas_table_new (table, NULL);

      // Status icon
      if (_selected_config)
      {
        gchar *icon_name;

        if (   (pool_size == _selected_config->_size)
            || (   _selected_config->_nb_overloaded
                && ((pool_size == _selected_config->_size) || (pool_size == _selected_config->_size + 1))))
        {
          icon_name = g_strdup (GTK_STOCK_APPLY);
          pool->SetData (this, "is_balanced", (void *) 1);
        }
        else
        {
          icon_name = g_strdup (GTK_STOCK_DIALOG_WARNING);
          pool->SetData (this, "is_balanced", 0);
        }

        Canvas::PutStockIconInTable (name_table,
                                     icon_name,
                                     0, 0);
        g_free (icon_name);
      }

      // Name
      {
        item = Canvas::PutTextInTable (name_table,
                                       pool->GetName (),
                                       0, 1);
        g_object_set (G_OBJECT (item),
                      "font", BP_FONT "bold 18px",
                      NULL);
      }

      Canvas::PutInTable (table,
                          name_table,
                          0, 0);
      Canvas::SetTableItemAttribute (name_table, "columns", g_slist_length (layout_list));
      //Canvas::SetTableItemAttribute (name_table, "x-expand", 1U);
      Canvas::SetTableItemAttribute (name_table, "x-fill", 1U);
    }

    if (layout_list)
    {
      guint p = 0;

      // Referees
      {
        GSList *current = pool->GetRefereeList ();

        while (current)
        {
          Player *player = (Player *) current->data;

          DisplayPlayer (player,
                         p,
                         table,
                         zone,
                         layout_list);
          p++;

          current = g_slist_next (current);
        }
      }

      // Fencers
      {
        GSList *current = pool->GetFencerList ();

        while (current)
        {
          Player *player = (Player *) current->data;

          DisplayPlayer (player,
                         p,
                         table,
                         zone,
                         layout_list);
          p++;

          current = g_slist_next (current);
        }
      }
    }

    {
      GooCanvasBounds bounds;

      goo_canvas_item_get_bounds (table,
                                  &bounds);

      if ((bounds.x2 - bounds.x1) > _max_w)
      {
        _max_w = bounds.x2 - bounds.x1;
      }
      if ((bounds.y2 - bounds.y1) > _max_h)
      {
        _max_h = bounds.y2 - bounds.y1;
      }
    }

    zone->Draw (GetRootItem ());
  }

  // --------------------------------------------------------------------------------
  void Allocator::DisplayPlayer (Player        *player,
                                 guint          indice,
                                 GooCanvasItem *table,
                                 PoolZone      *zone,
                                 GSList        *layout_list)
  {
    if (player)
    {
      Pool *pool = zone->GetPool ();

      if (player->Is ("Referee"))
      {
        static gchar  *referee_icon = NULL;

        if (referee_icon == NULL)
        {
          referee_icon = g_build_filename (Global::_share_dir, "resources/glade/referee.png", NULL);
        }
        Canvas::PutIconInTable (table,
                                referee_icon,
                                indice+1, 0);
      }
      else
      {
        if (player->GetUIntData (this, "original_pool") != pool->GetNumber ())
        {
          Canvas::PutStockIconInTable (table,
                                       GTK_STOCK_REFRESH,
                                       indice+1, 0);
        }
#ifdef DEBUG
        {
          guint swapped_from = player->GetUIntData (this, "swapped_from");

          if (swapped_from)
          {
            GooCanvasItem *item;
            gchar         *swapped_from_text = g_strdup_printf ("%02d", swapped_from);

            item = Canvas::PutTextInTable (table,
                                           swapped_from_text,
                                           indice+1, 1);
            g_object_set (G_OBJECT (item),
                          "font", BP_FONT "Bold Italic 14px",
                          "fill_color", "blue",
                          NULL);
            g_free (swapped_from_text);
          }
        }
#endif
      }

      for (guint i = 0; layout_list != NULL; i++)
      {
        GooCanvasItem        *item;
        Filter::Layout       *attr_layout = (Filter::Layout *) layout_list->data;
        Attribute            *attr;
        Player::AttributeId  *attr_id;

        attr_id = Player::AttributeId::Create (attr_layout->_desc, this);
        attr = player->GetAttribute (attr_id);
        attr_id->Release ();

        item = NULL;
        if (attr)
        {
          GdkPixbuf *pixbuf = NULL;

          if (attr_layout->_look == AttributeDesc::GRAPHICAL)
          {
            pixbuf = attr->GetPixbuf ();
          }

          if (pixbuf)
          {
            item = Canvas::PutPixbufInTable (table,
                                             pixbuf,
                                             indice+1, i+2);
          }
          else
          {
            gchar *image = attr->GetUserImage (attr_layout->_look);

            if (image)
            {
              item = Canvas::PutTextInTable (table,
                                             image,
                                             indice+1, i+2);
              g_free (image);
            }
          }
        }

        if (item == NULL)
        {
          item = Canvas::PutTextInTable (table,
                                         " ",
                                         indice+1, i+2);
        }

        if (item)
        {
          if (player->Is ("Referee"))
          {
            g_object_set (G_OBJECT (item),
                          "font", BP_FONT "Bold Italic 14px",
                          "fill_color", "black",
                          NULL);
          }
          else
          {
            g_object_set (G_OBJECT (item),
                          "font", BP_FONT "14px",
                          "fill_color", "black",
                          NULL);
          }

          SetObjectDropZone (player,
                             item,
                             zone);
        }

        layout_list = g_slist_next (layout_list);
      }
    }
  }

  // --------------------------------------------------------------------------------
  void Allocator::DeletePools ()
  {
    if (_drop_zones)
    {
      GSList *current = _drop_zones;

      while (current)
      {
        Pool *pool = GetPoolOf (current);

        Object::TryToRelease ((PoolZone *) current->data);
        Object::TryToRelease (pool);
        current = g_slist_next (current);
      }

      g_slist_free (_drop_zones);
      _drop_zones = NULL;
    }

    CanvasModule::Wipe ();
    _main_table = NULL;

    RefreshMatchRate (-_nb_matchs);
    _nb_matchs = 0;
  }

  // --------------------------------------------------------------------------------
  void Allocator::Reset ()
  {
    DeletePools ();

    {
      g_signal_handlers_disconnect_by_func (_glade->GetWidget ("pool_size_combobox"),
                                            (void *) on_pool_size_combobox_changed,
                                            (Object *) this);
      g_signal_handlers_disconnect_by_func (_glade->GetWidget ("nb_pools_combobox"),
                                            (void *) on_nb_pools_combobox_changed,
                                            (Object *) this);

      gtk_list_store_clear (_combobox_store);

      g_signal_connect (_glade->GetWidget ("pool_size_combobox"), "changed",
                        G_CALLBACK (on_pool_size_combobox_changed),
                        (Object *) this);
      g_signal_connect (_glade->GetWidget ("nb_pools_combobox"), "changed",
                        G_CALLBACK (on_nb_pools_combobox_changed),
                        (Object *) this);
    }

    if (_config_list)
    {
      g_slist_free_full (_config_list,
                         (GDestroyNotify) g_free);
      _config_list     = NULL;
      _selected_config = NULL;
    }
  }

  // --------------------------------------------------------------------------------
  gboolean Allocator::IsOver ()
  {
    if (_swapper->HasErrors ())
    {
      return FALSE;
    }
    else
    {
      GSList *current = _drop_zones;

      while (current)
      {
        Pool *pool = GetPoolOf (current);

        if (pool->GetUIntData (this, "is_balanced") == 0)
        {
          return FALSE;
        }
        current = g_slist_next (current);
      }
    }
    return TRUE;
  }

  // --------------------------------------------------------------------------------
  gchar *Allocator::GetError ()
  {
    if (_swapper->HasErrors ())
    {
      return g_strdup_printf ("<span foreground=\"black\" style=\"italic\" weight=\"400\">\"%s\" </span>",
                              gettext ("Swapping failed"));
    }
    else
    {
      GSList *current = _drop_zones;

      while (current)
      {
        Pool *pool = GetPoolOf (current);

        if (pool->GetUIntData (this, "is_balanced") == 0)
        {
          return g_strdup_printf (" <span foreground=\"black\" weight=\"800\">%s:</span> \n "
                                  " <span foreground=\"black\" style=\"italic\" weight=\"400\">\"%s\" </span>",
                                  pool->GetName (), gettext ("Wrong fencers count!"));
        }
        current = g_slist_next (current);
      }
    }

    return NULL;
  }

  // --------------------------------------------------------------------------------
  gchar *Allocator::GetPrintName ()
  {
    return g_strdup_printf ("%s %s", gettext ("Allocation"), GetName ());
  }

  // --------------------------------------------------------------------------------
  guint Allocator::PreparePrint (GtkPrintOperation *operation,
                                 GtkPrintContext   *context)
  {
    if (GetStageView (operation) == STAGE_VIEW_CLASSIFICATION)
    {
      return 0;
    }
    else
    {
      gdouble canvas_w;
      gdouble paper_w  = gtk_print_context_get_width (context);
      gdouble paper_h  = gtk_print_context_get_height (context);
      gdouble header_h = (PRINT_HEADER_HEIGHT+2) * paper_w  / 100;

      {
        GooCanvasBounds bounds;

        goo_canvas_item_get_bounds (GetRootItem (),
                                    &bounds);
        canvas_w = bounds.x2 - bounds.x1;
      }

      {
        gdouble canvas_dpi;
        gdouble printer_dpi;

        g_object_get (G_OBJECT (GetCanvas ()),
                      "resolution-x", &canvas_dpi,
                      NULL);
        printer_dpi = gtk_print_context_get_dpi_x (context);

        _print_scale = printer_dpi/canvas_dpi;
      }

      if (canvas_w * _print_scale > paper_w)
      {
        _print_scale = paper_w / canvas_w;
      }

      {
        guint nb_row_by_page = (guint) ((paper_h - header_h) / ((_max_h+20)*_print_scale));
        guint nb_row         = g_slist_length (_drop_zones)/2;

        if (g_slist_length (_drop_zones) % 2 > 0)
        {
          nb_row++;
        }

        _nb_page = nb_row/nb_row_by_page;
        _page_h  = nb_row_by_page * (_max_h+20)*_print_scale;

        if (nb_row % nb_row_by_page != 0)
        {
          _nb_page++;
        }
      }
    }

    return _nb_page;
  }

  // --------------------------------------------------------------------------------
  void Allocator::DrawPage (GtkPrintOperation *operation,
                            GtkPrintContext   *context,
                            gint               page_nr)
  {
    gdouble          paper_w = gtk_print_context_get_width  (context);
    cairo_t         *cr      = gtk_print_context_get_cairo_context (context);
    GooCanvasBounds  bounds;

    DrawContainerPage (operation,
                       context,
                       page_nr);

    {
      GooCanvas *canvas = Canvas::CreatePrinterCanvas (context);
      char      *text   = g_strdup_printf ("Page %d/%d", page_nr+1, _nb_page);

      goo_canvas_text_new (goo_canvas_get_root_item (canvas),
                           text,
                           98.0, -2.0,
                           -1.0,
                           GTK_ANCHOR_SE,
                           "fill-color", "black",
                           "font", BP_FONT "Bold 2px", NULL);
      g_free (text);

      goo_canvas_render (canvas,
                         gtk_print_context_get_cairo_context (context),
                         NULL,
                         1.0);

      gtk_widget_destroy (GTK_WIDGET (canvas));
    }

    cairo_save (cr);

    cairo_scale (cr,
                 _print_scale,
                 _print_scale);

    bounds.x1 = 0.0;
    bounds.y1 = (page_nr*_page_h)/_print_scale;
    bounds.x2 = paper_w/_print_scale;
    bounds.y2 = ((page_nr+1)*_page_h)/_print_scale;

    cairo_translate (cr,
                     0.0,
                     -(_page_h*page_nr)/_print_scale);

    goo_canvas_render (GetCanvas (),
                       cr,
                       &bounds,
                       1.0);

    cairo_restore (cr);
  }

  // --------------------------------------------------------------------------------
  void Allocator::DrawConfig (GtkPrintOperation *operation,
                              GtkPrintContext   *context,
                              gint               page_nr)
  {
    {
      gchar *text;

      if (_seeding_balanced->_value)
      {
        text = g_strdup_printf ("%s : %s",
                                gettext ("Seeding"),
                                gettext ("Balanced"));
      }
      else
      {
        text = g_strdup_printf ("%s : %s",
                                gettext ("Seeding"),
                                gettext ("Grouped by strength"));
      }

      DrawConfigLine (operation,
                      context,
                      text);

      g_free (text);
    }

    {
      gchar *text = g_strdup_printf ("%s : %d",
                                     gettext ("Max score"),
                                     _max_score->_value);

      DrawConfigLine (operation,
                      context,
                      text);

      g_free (text);
    }

    {
      gchar *text;

      if (_nb_qualified->IsValid ())
      {
        text = g_strdup_printf ("%s : %d",
                                gettext ("Qualified"),
                                _nb_qualified->_value);
      }
      else
      {
        text = g_strdup_printf ("%s : %s",
                                gettext ("Qualified"),
                                "100%");
      }

      DrawConfigLine (operation,
                      context,
                      text);

      g_free (text);
    }
  }

  // --------------------------------------------------------------------------------
  Data *Allocator::GetMaxScore ()
  {
    return _max_score;
  }

  // --------------------------------------------------------------------------------
  gboolean Allocator::SeedingIsBalanced ()
  {
    return (_seeding_balanced->_value == 1);
  }

  // --------------------------------------------------------------------------------
  guint Allocator::GetNbPools ()
  {
    return g_slist_length (_drop_zones);
  }

  // --------------------------------------------------------------------------------
  Pool *Allocator::GetPool (guint index)
  {
    return GetPoolOf (g_slist_nth (_drop_zones,
                                   index));
  }

  // --------------------------------------------------------------------------------
  Pool *Allocator::GetPoolOf (GSList *drop_zone)
  {
    if (drop_zone)
    {
      PoolZone *zone = (PoolZone *) drop_zone->data;

      return zone->GetPool ();
    }

    return NULL;
  }

  // --------------------------------------------------------------------------------
  PoolZone *Allocator::GetZone (guint index)
  {
    GSList *zone = g_slist_nth (_drop_zones,
                                index);

    return (PoolZone *) zone->data;
  }

  // --------------------------------------------------------------------------------
  extern "C" G_MODULE_EXPORT void on_pool_size_combobox_changed (GtkWidget *widget,
                                                                 Object    *owner)
  {
    Allocator *pl = dynamic_cast <Allocator *> (owner);

    pl->OnComboboxChanged (GTK_COMBO_BOX (widget));
  }

  // --------------------------------------------------------------------------------
  void Allocator::OnComboboxChanged (GtkComboBox *cb)
  {
    gint config_index     = gtk_combo_box_get_active (cb);
    Configuration *config = (Configuration *) g_slist_nth_data (_config_list,
                                                                config_index);

    if (config)
    {
      {
        GtkWidget *w;

        w = _glade->GetWidget ("nb_pools_combobox");
        g_signal_handlers_disconnect_by_func (G_OBJECT (w),
                                              (void *) on_nb_pools_combobox_changed,
                                              (Object *) this);
        gtk_combo_box_set_active (GTK_COMBO_BOX (w),
                                  config_index);
        g_signal_connect (G_OBJECT (w), "changed",
                          G_CALLBACK (on_nb_pools_combobox_changed),
                          (Object *) this);

        w = _glade->GetWidget ("pool_size_combobox");
        g_signal_handlers_disconnect_by_func (G_OBJECT (w),
                                              (void *) on_pool_size_combobox_changed,
                                              (Object *) this);
        gtk_combo_box_set_active (GTK_COMBO_BOX (w),
                                  config_index);
        g_signal_connect (G_OBJECT (w), "changed",
                          G_CALLBACK (on_pool_size_combobox_changed),
                          (Object *) this);
      }

      _selected_config = config;

      RefreshDisplay ();
    }
  }

  // --------------------------------------------------------------------------------
  void Allocator::OnPrintClicked ()
  {
    gchar *title = GetPrintName ();

    if (gtk_toggle_tool_button_get_active (GTK_TOGGLE_TOOL_BUTTON (_glade->GetWidget ("fencer_list"))))
    {
      _fencer_list->Print (title);
    }
    else
    {
      Print (title);
    }
    g_free (title);
  }

  // --------------------------------------------------------------------------------
  extern "C" G_MODULE_EXPORT void on_nb_pools_combobox_changed (GtkWidget *widget,
                                                                Object    *owner)
  {
    Allocator *c = dynamic_cast <Allocator *> (owner);

    c->OnComboboxChanged (GTK_COMBO_BOX (widget));
  }

  // --------------------------------------------------------------------------------
  extern "C" G_MODULE_EXPORT void on_print_toolbutton_clicked (GtkWidget *widget,
                                                               Object    *owner)
  {
    Allocator *pa = dynamic_cast <Allocator *> (owner);

    pa->OnPrintClicked ();
  }

  // --------------------------------------------------------------------------------
  void Allocator::OnLocked ()
  {
    DisableSensitiveWidgets ();

    {
      GSList *current = _drop_zones;

      while (current)
      {
        Pool *pool = GetPoolOf (current);

        pool->CreateMatchs (_swapping_criteria_list);
        current = g_slist_next (current);
      }
    }
  }

  // --------------------------------------------------------------------------------
  GSList *Allocator::GetCurrentClassification ()
  {
    return g_slist_copy (_attendees->GetShortList ());
  }

  // --------------------------------------------------------------------------------
  void Allocator::OnUnLocked ()
  {
    EnableSensitiveWidgets ();
  }

  // --------------------------------------------------------------------------------
  void Allocator::OnAttrListUpdated ()
  {
    GSList *current = _drop_zones;

    _max_w = 0;
    _max_h = 0;

    while (current)
    {
      FillPoolTable ((PoolZone *) current->data);

      current = g_slist_next (current);
    }
    FixUpTablesBounds ();
  }

  // --------------------------------------------------------------------------------
  void Allocator::OnFilterClicked ()
  {
    if (gtk_toggle_tool_button_get_active (GTK_TOGGLE_TOOL_BUTTON (_glade->GetWidget ("fencer_list"))))
    {
      _fencer_list->SelectAttributes ();
    }
    else
    {
      SelectAttributes ();
    }
  }

  // --------------------------------------------------------------------------------
  void Allocator::OnSwappingToggled (GtkToggleButton *togglebutton,
                                     Allocator       *allocator)
  {
    if (allocator->_contest->GetState () == OPERATIONAL)
    {
      GtkWidget *parent   = gtk_widget_get_parent (GTK_WIDGET (togglebutton));
      GList     *siblings = gtk_container_get_children (GTK_CONTAINER (parent));
      GString   *string   = g_string_new ("/");

      g_slist_free (allocator->_swapping_criteria_list);
      allocator->_swapping_criteria_list = NULL;

      while (siblings)
      {
        GtkToggleButton *togglebutton = GTK_TOGGLE_BUTTON (siblings->data);

        if (gtk_toggle_button_get_active (togglebutton))
        {
          AttributeDesc *desc;

          desc = (AttributeDesc *) g_object_get_data (G_OBJECT (togglebutton), "criteria_attribute");

          allocator->_swapping_criteria_list = g_slist_append (allocator->_swapping_criteria_list,
                                                               desc);

          g_string_append_printf (string,
                                  "%s/",
                                  desc->_code_name);
        }

        siblings = g_list_next (siblings);
      }

      allocator->_swapping->SetString (string->str);
      g_string_free (string,
                     TRUE);

      allocator->RefreshDisplay ();
    }
  }

  // --------------------------------------------------------------------------------
  void Allocator::DumpToHTML (FILE *file)
  {
    _fencer_list->DumpToHTML (file);
  }

  // --------------------------------------------------------------------------------
  extern "C" G_MODULE_EXPORT void on_filter_button_clicked (GtkWidget *widget,
                                                            Object    *owner)
  {
    Allocator *p = dynamic_cast <Allocator *> (owner);

    p->OnFilterClicked ();
  }

  // --------------------------------------------------------------------------------
  void Allocator::OnFencerListToggled (gboolean toggled)
  {
    GtkWidget *main_w        = _glade->GetWidget ("main_hook");
    GtkWidget *fencer_list_w = _glade->GetWidget ("fencer_list_hook");

    if (toggled)
    {
      if (main_w)
      {
        gtk_widget_hide (main_w);
      }
      gtk_widget_show (fencer_list_w);
    }
    else
    {
      gtk_widget_hide (fencer_list_w);
      if (main_w)
      {
        gtk_widget_show_all (main_w);
      }
    }
  }

  // --------------------------------------------------------------------------------
  extern "C" G_MODULE_EXPORT void on_fencer_list_toggled (GtkToggleToolButton *widget,
                                                          Object              *owner)
  {
    Allocator *p = dynamic_cast <Allocator *> (owner);

    p->OnFencerListToggled (gtk_toggle_tool_button_get_active (widget));
  }
}
