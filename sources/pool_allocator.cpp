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

#include "swapper.hpp"
#include "pool_match_order.hpp"

#include "pool_allocator.hpp"

#define VALUE_INIT {0,{{0}}}

typedef enum
{
  POOL_SIZE_COL,
  NB_POOLS_COL,
  BEST_PIXMAP_COL
} ComboboxColumn;

typedef enum
{
  SWAPPING_IMAGE,
  SWAPPING_CRITERIA
} SwappingColumn;

extern "C" G_MODULE_EXPORT void on_nb_pools_combobox_changed (GtkWidget *widget,
                                                              Object    *owner);
extern "C" G_MODULE_EXPORT void on_pool_size_combobox_changed (GtkWidget *widget,
                                                               Object    *owner);

const gchar *PoolAllocator::_class_name     = N_ ("Pools arrangement");
const gchar *PoolAllocator::_xml_class_name = "TourDePoules";

// --------------------------------------------------------------------------------
PoolAllocator::PoolAllocator (StageClass *stage_class)
: Stage (stage_class),
  CanvasModule ("pool_allocator.glade")
{
  _pools_list        = NULL;
  _config_list       = NULL;
  _selected_config   = NULL;
  _dragging          = FALSE;
  _target_pool       = NULL;
  _floating_player   = NULL;
  _drag_text         = NULL;
  _main_table        = NULL;
  _swapping_criteria = NULL;

  _max_score = new Data ("ScoreMax",
                         5);

  _swapping = new Data ("Decalage",
                        (gchar *) NULL);

  _combobox_store = GTK_LIST_STORE (_glade->GetObject ("combo_liststore"));

  // Sensitive widgets
  {
    AddSensitiveWidget (_glade->GetWidget ("nb_pools_combobox"));
    AddSensitiveWidget (_glade->GetWidget ("pool_size_combobox"));
    AddSensitiveWidget (_glade->GetWidget ("swapping_combobox"));
  }

  {
    GSList *attr_list;
    Filter *filter;

    AttributeDesc::CreateList (&attr_list,
#ifndef DEBUG
                               "ref",
#endif
                               "status",
                               "global_status",
                               "start_rank",
                               "final_rank",
                               "attending",
                               "exported",
                               "victories_ratio",
                               "indice",
                               "HS",
                               "rank",
                               NULL);
    filter = new Filter (attr_list,
                         this);

    filter->ShowAttribute ("previous_stage_rank");
    filter->ShowAttribute ("name");
    filter->ShowAttribute ("first_name");
    filter->ShowAttribute ("club");

    SetFilter (filter);
    filter->Release ();
  }

  {
    GtkListStore *swapping_store = GTK_LIST_STORE (_glade->GetObject ("swapping_liststore"));
    GtkTreeIter   iter;
    GSList       *attr           = _filter->GetAttrList ();

    gtk_list_store_append (swapping_store, &iter);
    gtk_list_store_set (swapping_store, &iter,
                        SWAPPING_IMAGE, "Aucun",
                        SWAPPING_CRITERIA, NULL,
                        -1);

    while (attr)
    {
      AttributeDesc *attr_desc;

      attr_desc = (AttributeDesc *) attr->data;
      if (attr_desc->_uniqueness == AttributeDesc::NOT_SINGULAR)
      {
        gtk_list_store_append (swapping_store, &iter);

        gtk_list_store_set (swapping_store, &iter,
                            SWAPPING_IMAGE, attr_desc->_user_name,
                            SWAPPING_CRITERIA, attr_desc,
                            -1);
      }

      attr = g_slist_next (attr);
    }

    gtk_combo_box_set_active (GTK_COMBO_BOX (_glade->GetObject ("swapping_combobox")),
                              0);
  }
}

// --------------------------------------------------------------------------------
PoolAllocator::~PoolAllocator ()
{
  _max_score->Release     ();
  _swapping->Release      ();
}

// --------------------------------------------------------------------------------
void PoolAllocator::Init ()
{
  RegisterStageClass (gettext (_class_name),
                      _xml_class_name,
                      CreateInstance);
}

// --------------------------------------------------------------------------------
Stage *PoolAllocator::CreateInstance (StageClass *stage_class)
{
  return new PoolAllocator (stage_class);
}

// --------------------------------------------------------------------------------
const gchar *PoolAllocator::GetInputProviderClient ()
{
  return "pool_stage";
}

// --------------------------------------------------------------------------------
void PoolAllocator::Display ()
{
  SetUpCombobox ();

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

      g_object_set (G_OBJECT (root),
                    "pointer-events", GOO_CANVAS_EVENTS_VISIBLE,
                    NULL);
      g_signal_connect (root, "motion_notify_event",
                        G_CALLBACK (on_motion_notify), this);
      g_signal_connect (root, "button_release_event",
                        G_CALLBACK (on_button_release), this);

      for (guint i = 0; i < g_slist_length (_pools_list); i++)
      {
        Pool *pool;

        pool = (Pool *) g_slist_nth_data (_pools_list, i);
        FillPoolTable (pool);
      }
    }
  }
  FixUpTablesBounds ();

  SignalStatusUpdate ();
}

// --------------------------------------------------------------------------------
void PoolAllocator::Garnish ()
{
  if (_pools_list == NULL)
  {
    FillCombobox ();

    if (_best_config)
    {
      _selected_config = _best_config;

      CreatePools ();
    }
  }
}

// --------------------------------------------------------------------------------
void PoolAllocator::LoadConfiguration (xmlNode *xml_node)
{
  Stage::LoadConfiguration (xml_node);

  if (_swapping)
  {
    GtkTreeModel *model          = GTK_TREE_MODEL (_glade->GetObject ("swapping_liststore"));
    guint         criteria_index = 0;

    _swapping->Load (xml_node);

    if (_swapping->_string)
    {
      GtkTreeIter iter;
      gboolean    iter_is_valid;

      iter_is_valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model),
                                                     &iter);
      for (guint i = 0; iter_is_valid; i++)
      {
        gchar         *image;
        AttributeDesc *attr_desc;

        gtk_tree_model_get (model, &iter,
                            SWAPPING_IMAGE,    &image,
                            SWAPPING_CRITERIA, &attr_desc,
                            -1);
        if (strcmp (image, _swapping->_string) == 0)
        {
          criteria_index = i;
          break;
        }
        iter_is_valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (model),
                                                  &iter);
      }
    }
    gtk_combo_box_set_active (GTK_COMBO_BOX (_glade->GetObject ("swapping_combobox")),
                              criteria_index);
  }
}

// --------------------------------------------------------------------------------
void PoolAllocator::SaveConfiguration (xmlTextWriter *xml_writer)
{
  Stage::SaveConfiguration (xml_writer);

  if (_swapping)
  {
    _swapping->Save (xml_writer);
  }
}

// --------------------------------------------------------------------------------
void PoolAllocator::Load (xmlNode *xml_node)
{
  static Pool  *current_pool = NULL;
  static guint  nb_pool      = 0;

  for (xmlNode *n = xml_node; n != NULL; n = n->next)
  {
    if (n->type == XML_ELEMENT_NODE)
    {
      if (   (current_pool == NULL)
          && strcmp ((char *) n->name, _xml_class_name) == 0)
      {
        LoadConfiguration (xml_node);

        {
          gchar *string = (gchar *) xmlGetProp (xml_node,
                                                BAD_CAST "NbDePoules");
          if (string)
          {
            nb_pool = (guint) atoi (string);
          }
        }
      }
      else if (strcmp ((char *) n->name, "Poule") == 0)
      {
        if (_config_list == NULL)
        {
          // Get the configuration
          {
            FillCombobox ();

            for (guint i = 0; i < g_slist_length (_config_list); i++)
            {
              _selected_config = (Configuration *) g_slist_nth_data (_config_list,
                                                                     i);
              if (_selected_config->nb_pool == nb_pool)
              {
                break;
              }
            }
          }

          // Assign players an original pool
          {
            CreatePools ();
            DeletePools ();
          }
        }

        {
          guint number = g_slist_length (_pools_list);

          current_pool = new Pool (_max_score,
                                   number+1);

          _pools_list = g_slist_append (_pools_list,
                                        current_pool);
        }
      }
      else if (strcmp ((char *) n->name, "Tireur") == 0)
      {
        if (current_pool == NULL)
        {
          LoadAttendees (n);
        }
        else
        {
          gchar *attr;

          attr = (gchar *) xmlGetProp (n, BAD_CAST "REF");
          if (attr)
          {
            Player *player = GetPlayerFromRef (atoi (attr));

            if (player)
            {
              current_pool->AddPlayer (player,
                                       this);
            }
          }
        }
      }
      else if (strcmp ((char *) n->name, "Arbitre") == 0)
      {
      }
      else if (strcmp ((char *) n->name, "Match") == 0)
      {
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
void PoolAllocator::Save (xmlTextWriter *xml_writer)
{
  xmlTextWriterStartElement (xml_writer,
                             BAD_CAST _xml_class_name);

  SaveConfiguration (xml_writer);

  if (_pools_list)
  {
    xmlTextWriterWriteFormatAttribute (xml_writer,
                                       BAD_CAST "NbDePoules",
                                       "%d", g_slist_length (_pools_list));
  }
  xmlTextWriterWriteFormatAttribute (xml_writer,
                                     BAD_CAST "PhaseSuivanteDesQualifies",
                                     "%d", GetId ()+2);
  if (_selected_config)
  {
    xmlTextWriterWriteFormatAttribute (xml_writer,
                                       BAD_CAST "NbQualifiesParIndice",
                                       "%d", _selected_config->size);
  }

  Stage::SaveAttendees (xml_writer);

  {
    GSList *current = _pools_list;

    while (current)
    {
      Pool *pool;

      pool = (Pool *) current->data;
      pool->Save (xml_writer);
      current = g_slist_next (current);
    }
  }

  xmlTextWriterEndElement (xml_writer);
}

// --------------------------------------------------------------------------------
void PoolAllocator::RegisterConfig (Configuration *config)
{
  if (config)
  {
    GtkTreeIter iter;

    _config_list = g_slist_append (_config_list,
                                   config);
    gtk_list_store_append (_combobox_store, &iter);

    {
      gchar *nb_pool_text = g_strdup_printf ("%d", config->nb_pool);

      gtk_list_store_set (_combobox_store, &iter,
                          NB_POOLS_COL, nb_pool_text,
                          -1);
      g_free (nb_pool_text);
    }

    {
      gchar *pool_size_text;

      if (config->has_two_size)
      {
        pool_size_text = g_strdup_printf ("%d ou %d", config->size, config->size+1);
      }
      else
      {
        pool_size_text = g_strdup_printf ("%d", config->size);
      }

      gtk_list_store_set (_combobox_store, &iter,
                          POOL_SIZE_COL, pool_size_text,
                          -1);
      g_free (pool_size_text);
    }

    if (   (config->size < 7)
        || (config->size == 7) && (config->has_two_size == FALSE))
    {
      _best_config = config;
    }

    config = NULL;
  }
}

// --------------------------------------------------------------------------------
void PoolAllocator::FillCombobox ()
{
  guint          nb_players    = g_slist_length (_attendees->GetShortList ());
  Configuration *config        = NULL;
  guint          max_pool_size;

  if (nb_players%PoolMatchOrder::MAX_POOL_SIZE == 0)
  {
    max_pool_size = PoolMatchOrder::MAX_POOL_SIZE;
  }
  else
  {
    max_pool_size = PoolMatchOrder::MAX_POOL_SIZE-1;
  }

  _best_config = NULL;
  for (unsigned int size = 3; size <= MIN (max_pool_size, nb_players); size++)
  {
    if (nb_players%size == 0)
    {
      config = (Configuration *) g_malloc (sizeof (Configuration));

      config->has_two_size = FALSE;
      config->nb_pool      = nb_players/size;
      config->size         = size;

      RegisterConfig (config);
    }

    if (   (nb_players%size != 0)
        || ((size == 6) && (nb_players%7 != 0)))
    {
      for (unsigned int p = 1; p < nb_players/size; p++)
      {
        if ((nb_players - size*p) % (size + 1) == 0)
        {
          config = (Configuration *) g_malloc (sizeof (Configuration));

          config->has_two_size = TRUE;
          config->nb_pool      = p + (nb_players - size*p) / (size + 1);
          config->size         = size;

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
void PoolAllocator::CreatePools ()
{
  if (_selected_config)
  {
    Pool   **pool_table;
    GSList  *shortlist = _attendees->GetShortList ();
    guint    nb_pool   = _selected_config->nb_pool;

    pool_table = (Pool **) g_malloc (nb_pool * sizeof (Pool *));
    for (guint i = 0; i < nb_pool; i++)
    {
      pool_table[i] = new Pool (_max_score,
                                i+1);
      _pools_list = g_slist_append (_pools_list,
                                    pool_table[i]);
    }

    {
      Swapper *swapper;

      if (_swapping_criteria)
      {
        swapper = new Swapper (_pools_list,
                               _swapping_criteria->_code_name,
                               shortlist);
      }
      else
      {
        swapper = new Swapper (_pools_list,
                               NULL,
                               shortlist);
      }

      for (guint i = 0; i < g_slist_length (shortlist); i++)
      {
        Player *player;
        Pool   *pool;

        if (((i / nb_pool) % 2) == 0)
        {
          pool = pool_table[i%nb_pool];
        }
        else
        {
          pool = pool_table[nb_pool-1 - i%nb_pool];
        }

        player = swapper->GetNextPlayer (pool);
        player->SetData (this,
                         "original_pool",
                         (void *) pool->GetNumber ());
        pool->AddPlayer (player,
                         this);
      }

      swapper->Release ();
    }

    SetOriginalPools ();
  }
}

// --------------------------------------------------------------------------------
void PoolAllocator::SetOriginalPools ()
{
  for (guint i = 0; i < g_slist_length (_pools_list); i++)
  {
    Pool *pool;

    pool = (Pool *) g_slist_nth_data (_pools_list, i);
    for (guint j = 0; j < pool->GetNbPlayers (); j++)
    {
      Player *player;

      player = pool->GetPlayer (j);
      player->SetData (this,
                       "original_pool",
                       (void *) (i+1));
    }
  }
}

// --------------------------------------------------------------------------------
void PoolAllocator::SetUpCombobox ()
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
gboolean PoolAllocator::on_enter_player (GooCanvasItem  *item,
                                         GooCanvasItem  *target,
                                         GdkEventButton *event,
                                         Pool           *pool)
{
  PoolAllocator *pl = (PoolAllocator*) pool->GetPtrData (pool, "pool_allocator");

  if (pl)
  {
    pl->SetCursor (GDK_FLEUR);
  }

  return FALSE;
}

// --------------------------------------------------------------------------------
gboolean PoolAllocator::on_leave_player (GooCanvasItem  *item,
                                         GooCanvasItem  *target,
                                         GdkEventButton *event,
                                         Pool           *pool)
{
  PoolAllocator *pl = (PoolAllocator*) pool->GetPtrData (pool, "pool_allocator");

  if (pl)
  {
    pl->ResetCursor ();
  }

  return FALSE;
}

// --------------------------------------------------------------------------------
gboolean PoolAllocator::on_button_press (GooCanvasItem  *item,
                                         GooCanvasItem  *target,
                                         GdkEventButton *event,
                                         Pool           *pool)
{
  PoolAllocator *pl = (PoolAllocator*) pool->GetPtrData (pool, "pool_allocator");

  if (pl)
  {
    return pl->OnButtonPress (item,
                              target,
                              event,
                              pool);
  }

  return FALSE;
}

// --------------------------------------------------------------------------------
gboolean PoolAllocator::OnButtonPress (GooCanvasItem  *item,
                                       GooCanvasItem  *target,
                                       GdkEventButton *event,
                                       Pool           *pool)
{
  if (Locked ())
  {
    return FALSE;
  }

  if (   (event->button == 1)
         && (event->type == GDK_BUTTON_PRESS))
  {
    _floating_player = (Player *) g_object_get_data (G_OBJECT (item),
                                                     "PoolAllocator::player");

    _drag_x = event->x;
    _drag_y = event->y;

    goo_canvas_convert_from_item_space  (GetCanvas (),
                                         item,
                                         &_drag_x,
                                         &_drag_y);

    {
      GooCanvasBounds  bounds;
      GString         *string = g_string_new ("");
      GSList          *selected_attr = NULL;

      if (_filter)
      {
        selected_attr = _filter->GetSelectedAttrList ();
      }

      if (_floating_player && selected_attr)
      {
        for (guint i = 0; i < g_slist_length (selected_attr); i++)
        {
          AttributeDesc       *attr_desc;
          Attribute           *attr;
          Player::AttributeId *attr_id;

          attr_desc = (AttributeDesc *) g_slist_nth_data (selected_attr,
                                                          i);
          attr_id = Player::AttributeId::CreateAttributeId (attr_desc, this);
          attr = _floating_player->GetAttribute (attr_id);
          attr_id->Release ();

          if (attr)
          {
            gchar *image = attr->GetUserImage ();

            string = g_string_append (string,
                                      image);
            string = g_string_append (string,
                                      "  ");
            g_free (image);
          }
        }
      }

      goo_canvas_item_get_bounds (item,
                                  &bounds);

      _drag_text = goo_canvas_text_new (GetRootItem (),
                                        string->str,
                                        //bounds.x1,
                                        //bounds.y1,
                                        _drag_x,
                                        _drag_y,
                                        -1,
                                        GTK_ANCHOR_NW,
                                        "font", "Sans Bold 14px", NULL);
      g_string_free (string,
                     TRUE);

    }

    SetCursor (GDK_FLEUR);

    _dragging = TRUE;
    _source_pool = pool;

    {
      pool->RemovePlayer (_floating_player);
      FillPoolTable (pool);
      SignalStatusUpdate ();
      FixUpTablesBounds ();
    }

    return TRUE;
  }

  return FALSE;
}

// --------------------------------------------------------------------------------
gboolean PoolAllocator::on_button_release (GooCanvasItem  *item,
                                           GooCanvasItem  *target,
                                           GdkEventButton *event,
                                           PoolAllocator    *pl)
{
  if (pl)
  {
    return pl->OnButtonRelease (item,
                                target,
                                event);
  }

  return FALSE;
}

// --------------------------------------------------------------------------------
gboolean PoolAllocator::OnButtonRelease (GooCanvasItem  *item,
                                         GooCanvasItem  *target,
                                         GdkEventButton *event)
{
  if (_dragging)
  {
    _dragging = FALSE;

    goo_canvas_item_remove (_drag_text);
    _drag_text = NULL;

    if (_target_pool)
    {
      _target_pool->AddPlayer (_floating_player,
                               this);
      FillPoolTable (_target_pool);
      _target_pool = NULL;
    }
    else
    {
      _source_pool->AddPlayer (_floating_player,
                               this);
      FillPoolTable (_source_pool);
    }
    //OnAttrListUpdated ();
    SignalStatusUpdate ();
    FixUpTablesBounds ();

    _floating_player = NULL;

    ResetCursor ();
  }

  return TRUE;
}

// --------------------------------------------------------------------------------
gboolean PoolAllocator::on_motion_notify (GooCanvasItem  *item,
                                          GooCanvasItem  *target,
                                          GdkEventButton *event,
                                          PoolAllocator  *pl)
{
  if (pl)
  {
    pl->OnMotionNotify (item,
                        target,
                        event);
  }

  return FALSE;
}

// --------------------------------------------------------------------------------
gboolean PoolAllocator::OnMotionNotify (GooCanvasItem  *item,
                                        GooCanvasItem  *target,
                                        GdkEventButton *event)
{
  if (_dragging && (event->state & GDK_BUTTON1_MASK))
  {
    gdouble new_x = event->x_root;
    gdouble new_y = event->y_root;

    goo_canvas_item_translate (_drag_text,
                               new_x - _drag_x,
                               new_y - _drag_y);

    _drag_x = new_x;
    _drag_y = new_y;

    SetCursor (GDK_FLEUR);
  }

  return TRUE;
}

// --------------------------------------------------------------------------------
gboolean PoolAllocator::on_enter_notify (GooCanvasItem  *item,
                                         GooCanvasItem  *target,
                                         GdkEventButton *event,
                                         Pool           *pool)
{
  PoolAllocator *pl = (PoolAllocator*) pool->GetPtrData (pool, "pool_allocator");

  if (pl)
  {
    return pl->OnEnterNotify (item,
                              target,
                              event,
                              pool);
  }

  return FALSE;
}

// --------------------------------------------------------------------------------
gboolean PoolAllocator::OnEnterNotify (GooCanvasItem  *item,
                                       GooCanvasItem  *target,
                                       GdkEventButton *event,
                                       Pool           *pool)
{
  if (_dragging)
  {
    GooCanvasItem *rect = (GooCanvasItem*) pool->GetPtrData (this, "focus_rectangle");

    g_object_set (G_OBJECT (rect),
                  "stroke-color", "black",
                  NULL);

    _target_pool = pool;

    return TRUE;
  }
  return FALSE;
}

// --------------------------------------------------------------------------------
gboolean PoolAllocator::on_leave_notify (GooCanvasItem  *item,
                                         GooCanvasItem  *target,
                                         GdkEventButton *event,
                                         Pool           *pool)
{
  PoolAllocator *pl = (PoolAllocator*) pool->GetPtrData (pool, "pool_allocator");

  if (pl)
  {
    return pl->OnLeaveNotify (item,
                              target,
                              event,
                              pool);
  }

  return FALSE;
}

// --------------------------------------------------------------------------------
gboolean PoolAllocator::OnLeaveNotify (GooCanvasItem  *item,
                                       GooCanvasItem  *target,
                                       GdkEventButton *event,
                                       Pool           *pool)
{
  if (_dragging)
  {
    GooCanvasItem *rect = (GooCanvasItem*) pool->GetPtrData (this, "focus_rectangle");

    g_object_set (G_OBJECT (rect),
                  "stroke-pattern", NULL,
                  NULL);

    _target_pool = NULL;

    return TRUE;
  }
  return FALSE;
}

// --------------------------------------------------------------------------------
void PoolAllocator::FixUpTablesBounds ()
{
  if (_selected_config == NULL)
  {
    return;
  }

  for (guint p = 0; p < g_slist_length (_pools_list); p++)
  {
    Pool            *pool;
    GooCanvasItem   *focus_rect;
    GooCanvasBounds  bounds;
    GooCanvasItem   *table;

    pool = (Pool *) g_slist_nth_data (_pools_list, p);
    table = (GooCanvasItem *) pool->GetPtrData (this, "table");

    focus_rect = (GooCanvasItem *) pool->GetPtrData (this, "focus_rectangle");

    {
      guint nb_columns = 2;
      guint nb_rows    = _selected_config->nb_pool / nb_columns;

      if (_selected_config->nb_pool % nb_columns != 0)
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

    g_object_set (G_OBJECT (focus_rect),
                  "x",      bounds.x1,
                  "y",      bounds.y1,
                  "width",  bounds.x2 - bounds.x1,
                  "height", bounds.y2 - bounds.y1,
                  NULL);

    if (_dragging)
    {
      goo_canvas_item_raise (focus_rect,
                             NULL);
    }
    else
    {
      goo_canvas_item_lower (focus_rect,
                             NULL);

      g_object_set (G_OBJECT (focus_rect),
                    "stroke-pattern", NULL,
                    NULL);
    }
  }
}

// --------------------------------------------------------------------------------
void PoolAllocator::FillPoolTable (Pool *pool)
{
  if (_main_table == NULL)
  {
    return;
  }

  GooCanvasItem *item;
  GooCanvasItem *table      = (GooCanvasItem *) pool->GetPtrData (this, "table");
  GooCanvasItem *focus_rect = (GooCanvasItem *) pool->GetPtrData (this, "focus_rectangle");
  GSList *selected_attr     = NULL;

  if (_filter)
  {
    selected_attr = _filter->GetSelectedAttrList ();
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
    pool->SetData (this, "table",
                   table);
  }

  {
    guint pool_size           = pool->GetNbPlayers ();
    GooCanvasItem *name_table = goo_canvas_table_new (table, NULL);

    // Status icon
    if (_selected_config)
    {
      gchar *icon_name;

      if (   (pool_size == _selected_config->size)
          || (   _selected_config->has_two_size
              && ((pool_size == _selected_config->size)) || (pool_size == _selected_config->size + 1)))
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
                    "font", "Sans bold 18px",
                    NULL);
    }

    Canvas::PutInTable (table,
                        name_table,
                        0, 0);
    Canvas::SetTableItemAttribute (name_table, "columns", g_slist_length (selected_attr));
    //Canvas::SetTableItemAttribute (name_table, "x-expand", 1U);
    Canvas::SetTableItemAttribute (name_table, "x-fill", 1U);
  }

  for (guint p = 0; p < pool->GetNbPlayers (); p++)
  {
    Player *player;

    player = pool->GetPlayer (p);

    if (player && selected_attr)
    {
      if (player->GetUIntData (this, "original_pool") != pool->GetNumber ())
      {
        Canvas::PutStockIconInTable (table,
                                     GTK_STOCK_REFRESH,
                                     p+1, 0);
      }
      for (guint i = 0; i < g_slist_length (selected_attr); i++)
      {
        GooCanvasItem       *item;
        AttributeDesc       *attr_desc;
        Attribute           *attr;
        Player::AttributeId *attr_id;

        attr_desc = (AttributeDesc *) g_slist_nth_data (selected_attr,
                                                        i);
        attr_id = Player::AttributeId::CreateAttributeId (attr_desc, this);
        attr = player->GetAttribute (attr_id);
        attr_id->Release ();

        if (attr)
        {
          gchar *image = attr->GetUserImage ();

          item = Canvas::PutTextInTable (table,
                                         image,
                                         p+1, i+1);
          g_free (image);
        }
        else
        {
          item = Canvas::PutTextInTable (table,
                                         "",
                                         p+1, i+1);
        }
        g_object_set (G_OBJECT (item),
                      "font", "Sans 14px",
                      "fill_color", "black",
                      NULL);
        g_object_set_data (G_OBJECT (item),
                           "PoolAllocator::player",
                           player);

        g_signal_connect (item, "button_press_event",
                          G_CALLBACK (on_button_press), pool);
        g_signal_connect (item, "enter_notify_event",
                          G_CALLBACK (on_enter_player), pool);
        g_signal_connect (item, "leave_notify_event",
                          G_CALLBACK (on_leave_player), pool);
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

  if (focus_rect == NULL)
  {
    focus_rect = goo_canvas_rect_new (GetRootItem (),
                                      0, 0,
                                      0, 0,
                                      "pointer-events", GOO_CANVAS_EVENTS_VISIBLE,
                                      "stroke-pattern", NULL,
                                      NULL);

    goo_canvas_item_lower (focus_rect,
                           NULL);

    g_signal_connect (focus_rect, "enter_notify_event",
                      G_CALLBACK (on_enter_notify), pool);
    g_signal_connect (focus_rect, "leave_notify_event",
                      G_CALLBACK (on_leave_notify), pool);

    pool->SetData (pool, "pool_allocator",
                   this);
    pool->SetData (this, "focus_rectangle",
                   focus_rect);
  }
}

// --------------------------------------------------------------------------------
void PoolAllocator::DeletePools ()
{
  if (_pools_list)
  {
    for (guint i = 0; i < g_slist_length (_pools_list); i++)
    {
      Pool *pool;

      pool = (Pool *) g_slist_nth_data (_pools_list, i);
      Object::TryToRelease (pool);
    }

    g_slist_free (_pools_list);
    _pools_list = NULL;
  }

  CanvasModule::Wipe ();
  _main_table = NULL;
}

// --------------------------------------------------------------------------------
void PoolAllocator::Wipe ()
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
    for (guint i = 0; i < g_slist_length (_config_list); i++)
    {
      Configuration *config;

      config = (Configuration *) g_slist_nth_data (_config_list, i);
      g_free (config);
    }
    g_slist_free (_config_list);
    _config_list     = NULL;
    _selected_config = NULL;
  }
}

// --------------------------------------------------------------------------------
gboolean PoolAllocator::IsOver ()
{
  for (guint i = 0; i < g_slist_length (_pools_list); i++)
  {
    Pool *pool;

    pool = (Pool *) g_slist_nth_data (_pools_list, i);

    if (pool->GetUIntData (this, "is_balanced") == 0)
    {
      return FALSE;
    }
  }
  return TRUE;
}

// --------------------------------------------------------------------------------
void PoolAllocator::OnBeginPrint (GtkPrintOperation *operation,
                                  GtkPrintContext   *context)
{
  gdouble canvas_x;
  gdouble canvas_y;
  gdouble canvas_w;
  gdouble canvas_h;
  gdouble paper_w  = gtk_print_context_get_width (context);
  gdouble paper_h  = gtk_print_context_get_height (context);
  gdouble header_h = (PRINT_HEADER_HEIGHT+2) * paper_w  / 100;

  {
    GooCanvasBounds bounds;

    goo_canvas_item_get_bounds (GetRootItem (),
                                &bounds);
    canvas_x = bounds.x1;
    canvas_y = bounds.y1;
    canvas_w = bounds.x2 - bounds.x1;
    canvas_h = bounds.y2 - bounds.y1;
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
    guint nb_row         = g_slist_length (_pools_list)/2;

    if (g_slist_length (_pools_list) % 2 > 0)
    {
      nb_row++;
    }

    _nb_page = nb_row/nb_row_by_page;
    _page_h  = nb_row_by_page * (_max_h+20)*_print_scale;

    if (nb_row % nb_row_by_page != 0)
    {
      _nb_page++;
    }

    gtk_print_operation_set_n_pages (operation,
                                     _nb_page);
  }
}

// --------------------------------------------------------------------------------
void PoolAllocator::OnDrawPage (GtkPrintOperation *operation,
                                GtkPrintContext   *context,
                                gint               page_nr)
{
  gdouble paper_w = gtk_print_context_get_width  (context);
  cairo_t         *cr       = gtk_print_context_get_cairo_context (context);
  gdouble          header_h = (PRINT_HEADER_HEIGHT+2) * paper_w  / 100;
  GooCanvasBounds  bounds;

  DrawContainerPage (operation,
                     context,
                     page_nr);

  {
    GooCanvas *canvas = Canvas::CreatePrinterCanvas (context);
    char      *text   = g_strdup_printf ("Page %d/%d", page_nr+1, _nb_page);

    goo_canvas_text_new (goo_canvas_get_root_item (canvas),
                         text,
                         98.0, 9.0,
                         -1.0,
                         GTK_ANCHOR_SE,
                         "fill-color", "black",
                         "font", "Sans Bold 2px", NULL);
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
                   (header_h - (_page_h*page_nr))/_print_scale);

  goo_canvas_render (GetCanvas (),
                     cr,
                     &bounds,
                     1.0);

  cairo_restore (cr);
}

// --------------------------------------------------------------------------------
Data *PoolAllocator::GetMaxScore ()
{
  return _max_score;
}

// --------------------------------------------------------------------------------
guint PoolAllocator::GetNbPools ()
{
  return g_slist_length (_pools_list);
}

// --------------------------------------------------------------------------------
Pool *PoolAllocator::GetPool (guint index)
{
  return (Pool *) g_slist_nth_data (_pools_list,
                                    index);
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_swapping_combobox_changed (GtkWidget *widget,
                                                              Object    *owner)
{
  PoolAllocator *pl = dynamic_cast <PoolAllocator *> (owner);

  pl->OnSwappingComboboxChanged (GTK_COMBO_BOX (widget));
}

// --------------------------------------------------------------------------------
void PoolAllocator::OnSwappingComboboxChanged (GtkComboBox *cb)
{
  GtkTreeIter iter;

  gtk_combo_box_get_active_iter (cb,
                                 &iter);
  gtk_tree_model_get (GTK_TREE_MODEL (_glade->GetObject ("swapping_liststore")),
                      &iter,
                      SWAPPING_IMAGE, &_swapping->_string,
                      SWAPPING_CRITERIA, &_swapping_criteria,
                      -1);

  if (_pools_list)
  {
    DeletePools ();
    CreatePools ();

    Display ();
    SignalStatusUpdate ();
  }
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_pool_size_combobox_changed (GtkWidget *widget,
                                                               Object    *owner)
{
  PoolAllocator *pl = dynamic_cast <PoolAllocator *> (owner);

  pl->OnComboboxChanged (GTK_COMBO_BOX (widget));
}

// --------------------------------------------------------------------------------
void PoolAllocator::OnComboboxChanged (GtkComboBox *cb)
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

    DeletePools ();
    CreatePools ();
    Display ();
    SignalStatusUpdate ();
  }
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_nb_pools_combobox_changed (GtkWidget *widget,
                                                              Object    *owner)
{
  PoolAllocator *c = dynamic_cast <PoolAllocator *> (owner);

  c->OnComboboxChanged (GTK_COMBO_BOX (widget));
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_print_toolbutton_clicked (GtkWidget *widget,
                                                             Object    *owner)
{
  PoolAllocator *c = dynamic_cast <PoolAllocator *> (owner);

  c->Print (gettext ("Pools allocation"));
}

// --------------------------------------------------------------------------------
void PoolAllocator::OnLocked (Reason reason)
{
  DisableSensitiveWidgets ();
}

// --------------------------------------------------------------------------------
GSList *PoolAllocator::GetCurrentClassification ()
{
  return g_slist_copy (_attendees->GetShortList ());
}

// --------------------------------------------------------------------------------
void PoolAllocator::OnUnLocked ()
{
  EnableSensitiveWidgets ();
}

// --------------------------------------------------------------------------------
void PoolAllocator::OnAttrListUpdated ()
{
  _max_w = 0;
  _max_h = 0;

  for (guint i = 0; i < g_slist_length (_pools_list); i++)
  {
    Pool *pool;

    pool = (Pool *) g_slist_nth_data (_pools_list, i);
    FillPoolTable (pool);
  }
  FixUpTablesBounds ();
}

// --------------------------------------------------------------------------------
extern "C" G_MODULE_EXPORT void on_filter_button_clicked (GtkWidget *widget,
                                                          Object    *owner)
{
  PoolAllocator *p = dynamic_cast <PoolAllocator *> (owner);

  p->SelectAttributes ();
}
