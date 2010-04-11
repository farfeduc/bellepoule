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
#include <goocanvas.h>

#include "canvas.hpp"
#include "attribute.hpp"
#include "schedule.hpp"
#include "player.hpp"

#include "players_list.hpp"

// --------------------------------------------------------------------------------
  PlayersList::PlayersList (gchar *glade_file,
                            guint  rights)
: Module (glade_file)
{
  _rights      = rights;
  _player_list = NULL;
  _store       = NULL;

  {
    _tree_view = _glade->GetWidget ("players_list");

    g_object_set (_tree_view,
                  "rules-hint",     TRUE,
                  "rubber-banding", TRUE,
                  NULL);

    gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (_tree_view)),
                                 GTK_SELECTION_MULTIPLE);
  }
}

// --------------------------------------------------------------------------------
PlayersList::~PlayersList ()
{
  Wipe ();
  g_object_unref (_store);
}

// --------------------------------------------------------------------------------
void PlayersList::SetFilter (Filter *filter)
{
  Module::SetFilter (filter);

  if (_filter)
  {
    GSList *attr_list    = _filter->GetAttrList ();
    guint  nb_attr       = g_slist_length (attr_list);
    GType  types[nb_attr + 1];

    for (guint i = 0; i < nb_attr; i++)
    {
      AttributeDesc *desc;

      desc = (AttributeDesc *) g_slist_nth_data (attr_list,
                                                 i);
      types[i] = desc->_type;
    }

    types[nb_attr] = G_TYPE_INT;

    _store = gtk_list_store_newv (nb_attr+1,
                                  types);

    gtk_tree_view_set_model (GTK_TREE_VIEW (_tree_view),
                             GTK_TREE_MODEL (_store));

    gtk_tree_view_set_search_column (GTK_TREE_VIEW (_tree_view),
                                     _filter->GetAttributeId ("name"));
  }
}

// --------------------------------------------------------------------------------
void PlayersList::Update (Player *player)
{
  GtkTreeRowReference *ref;
  GtkTreePath         *path;
  GtkTreeIter          iter;

  ref  = (GtkTreeRowReference *) player->GetData (this, "tree_row_ref");
  path = gtk_tree_row_reference_get_path (ref);
  gtk_tree_model_get_iter (GTK_TREE_MODEL (_store),
                           &iter,
                           path);
  gtk_tree_path_free (path);

  if (_filter)
  {
    GSList *attr_list = _filter->GetAttrList ();

    for (guint i = 0; i < g_slist_length (attr_list); i++)
    {
      AttributeDesc       *desc;
      Attribute           *attr;
      Player::AttributeId *attr_id;

      desc = (AttributeDesc *) g_slist_nth_data (attr_list,
                                                 i);
      if (desc->_scope == AttributeDesc::GLOBAL)
      {
        attr_id = new Player::AttributeId (desc->_xml_name);
      }
      else
      {
        attr_id = new Player::AttributeId (desc->_xml_name,
                                           GetDataOwner ());
      }

      attr = player->GetAttribute (attr_id);
      attr_id->Release ();

      if (attr)
      {
        gtk_list_store_set (_store, &iter,
                            i, attr->GetValue (), -1);
      }
      else
      {
        gtk_list_store_set (_store, &iter,
                            i, 0, -1);
      }
    }

    gtk_list_store_set (_store, &iter,
                        gtk_tree_model_get_n_columns (GTK_TREE_MODEL (_store)) - 1,
                        player, -1);
  }
}

// --------------------------------------------------------------------------------
GSList *PlayersList::CreateCustomList (CustomFilter filter)
{
  GSList *custom_list = NULL;

  for (guint i = 0; i < g_slist_length (_player_list); i++)
  {
    Player *p;

    p = (Player *) g_slist_nth_data (_player_list, i);

    if (filter (p) == TRUE)
    {
      custom_list = g_slist_append (custom_list,
                                    p);
    }
  }

  return custom_list;
}

// --------------------------------------------------------------------------------
void PlayersList::on_cell_edited (GtkCellRendererText *cell,
                                  gchar               *path_string,
                                  gchar               *new_text,
                                  gpointer             user_data)
{
  PlayersList   *p    = (PlayersList *) user_data;
  AttributeDesc *desc = (AttributeDesc *) g_object_get_data (G_OBJECT (cell),
                                                             "PlayersList::attribute_desc");

  p->OnCellEdited (path_string,
                   new_text,
                   desc);
}

// --------------------------------------------------------------------------------
void PlayersList::on_cell_toggled (GtkCellRendererToggle *cell,
                                   gchar                 *path_string,
                                   gpointer               user_data)
{
  PlayersList   *p    = (PlayersList *) user_data;
  AttributeDesc *desc = (AttributeDesc *) g_object_get_data (G_OBJECT (cell),
                                                             "PlayersList::attribute_desc");

  p->OnCellToggled (path_string,
                    gtk_cell_renderer_toggle_get_active (cell),
                    desc);
}

// --------------------------------------------------------------------------------
void PlayersList::OnCellEdited (gchar         *path_string,
                                gchar         *new_text,
                                AttributeDesc *desc)
{
  Player              *p       = GetPlayer (path_string);
  Player::AttributeId *attr_id = Player::AttributeId::CreateAttributeId (desc, this);

  p->SetAttributeValue (attr_id,
                        new_text);
  attr_id->Release ();

  Update (p);
  OnListChanged ();
}

// --------------------------------------------------------------------------------
GSList *PlayersList::GetSelectedPlayers ()
{
  GSList           *result         = NULL;
  GtkTreeSelection *selection      = gtk_tree_view_get_selection (GTK_TREE_VIEW (_tree_view));
  GList            *selection_list = gtk_tree_selection_get_selected_rows (selection,
                                                                           NULL);

  for (guint i = 0; i < g_list_length (selection_list); i++)
  {
    Player *p;
    gchar  *path;

    path = gtk_tree_path_to_string ((GtkTreePath *) g_list_nth_data (selection_list,
                                                                     i));
    p = GetPlayer (path);
    g_free (path);

    if (p)
    {
      result = g_slist_append (result,
                               p);
    }
  }

  g_list_foreach (selection_list, (GFunc) gtk_tree_path_free, NULL);
  g_list_free    (selection_list);

  return result;
}

// --------------------------------------------------------------------------------
void PlayersList::OnCellToggled (gchar         *path_string,
                                 gboolean       is_active,
                                 AttributeDesc *desc)
{
  GtkTreePath      *toggeled_path = gtk_tree_path_new_from_string (path_string);
  GtkTreeSelection *selection     = gtk_tree_view_get_selection (GTK_TREE_VIEW (_tree_view));

  if (gtk_tree_selection_path_is_selected (selection,
                                           toggeled_path))
  {
    GList *selection_list;

    selection_list = gtk_tree_selection_get_selected_rows (selection,
                                                           NULL);

    for (guint i = 0; i < g_list_length (selection_list); i++)
    {
      Player *p;

      {
        GtkTreePath *tree_path = (GtkTreePath *) g_list_nth_data (selection_list, i);
        gchar       *path = gtk_tree_path_to_string (tree_path);

        p = GetPlayer (path);

        g_free (path);
        gtk_tree_path_free (tree_path);
      }

      if (p)
      {
        Player::AttributeId *attr_id = Player::AttributeId::CreateAttributeId (desc, this);

        if (is_active)
        {
          p->SetAttributeValue (attr_id,
                                (guint) 0);
        }
        else
        {
          p->SetAttributeValue (attr_id,
                                1);
        }
        attr_id->Release ();

        Update (p);
      }
    }

    g_list_free (selection_list);
  }
  else
  {
    Player *p = GetPlayer (path_string);

    if (p)
    {
      Player::AttributeId *attr_id = Player::AttributeId::CreateAttributeId (desc, this);

      if (is_active)
      {
        p->SetAttributeValue (attr_id,
                              (guint) 0);
      }
      else
      {
        p->SetAttributeValue (attr_id,
                              1);
      }
      attr_id->Release ();

      Update (p);
    }
  }
  gtk_tree_path_free (toggeled_path);

  OnListChanged ();
}

// --------------------------------------------------------------------------------
void PlayersList::OnAttrListUpdated ()
{
  {
    GList *column_list = gtk_tree_view_get_columns (GTK_TREE_VIEW (_tree_view));
    guint  nb_col      = g_list_length (column_list);

    for (guint i = 0; i < nb_col; i++)
    {
      GtkTreeViewColumn *column;

      column = gtk_tree_view_get_column (GTK_TREE_VIEW (_tree_view),
                                         0);
      if (column)
      {
        gtk_tree_view_remove_column (GTK_TREE_VIEW (_tree_view),
                                     column);
      }
    }
  }

  if (_filter)
  {
    GSList *selected_attr = _filter->GetSelectedAttrList ();

    if (selected_attr)
    {
      for (guint i = 0; i < g_slist_length (selected_attr); i++)
      {
        AttributeDesc *desc;

        desc = (AttributeDesc *) g_slist_nth_data (selected_attr,
                                                   i);

        SetColumn (_filter->GetAttributeId (desc->_xml_name),
                   desc,
                   -1);
      }
    }
  }
}

// --------------------------------------------------------------------------------
gint PlayersList::CompareIterator (GtkTreeModel        *model,
                                   GtkTreeIter         *a,
                                   GtkTreeIter         *b,
                                   Player::AttributeId *attr_id)
{
  Player *player_a;
  Player *player_b;

  gtk_tree_model_get (model, a,
                      gtk_tree_model_get_n_columns (model) - 1,
                      &player_a, -1);
  gtk_tree_model_get (model, b,
                      gtk_tree_model_get_n_columns (model) - 1,
                      &player_b, -1);

  return Player::Compare (player_a,
                          player_b,
                          attr_id);
}

// --------------------------------------------------------------------------------
void PlayersList::SetColumn (guint          id,
                             AttributeDesc *desc,
                             gint           at)
{
  GtkTreeViewColumn *column   = NULL;
  GtkCellRenderer   *renderer = NULL;
  gboolean           attr_modifiable = (gboolean) desc->GetData (this,
                                                                 "modifiable");

  if (   (desc->_type == G_TYPE_STRING)
      || (desc->_type == G_TYPE_INT))
  {
    if (desc->HasDiscreteValue ())
    {
      renderer = gtk_cell_renderer_combo_new ();
      g_object_set (renderer,
                    "has-entry", desc->_free_value_allowed,
                    NULL);
      desc->BindDiscreteValues (G_OBJECT (renderer));
    }
    else
    {
      renderer = gtk_cell_renderer_text_new ();
    }

    if (   (desc->_rights == AttributeDesc::PUBLIC)
        && ((_rights & MODIFIABLE) || attr_modifiable))
    {
      g_object_set (renderer,
                    "editable", TRUE,
                    NULL);
      g_object_set_data (G_OBJECT (renderer),
                         "PlayersList::SensitiveAttribute",
                         (void *) "editable");
      g_signal_connect (renderer,
                        "edited", G_CALLBACK (on_cell_edited), this);
    }

    column = gtk_tree_view_column_new_with_attributes (desc->_user_name,
                                                       renderer,
                                                       "text", id,
                                                       0,      NULL);
  }
  else if (desc->_type == G_TYPE_BOOLEAN)
  {
    renderer = gtk_cell_renderer_toggle_new ();

    if (   (desc->_rights == AttributeDesc::PUBLIC)
        && ((_rights & MODIFIABLE) || attr_modifiable))
    {
      g_object_set (renderer,
                    "activatable", TRUE,
                    NULL);
      g_signal_connect (renderer,
                        "toggled", G_CALLBACK (on_cell_toggled), this);
      g_object_set_data (G_OBJECT (renderer),
                         "PlayersList::SensitiveAttribute",
                         (void *) "activatable");
    }

    column = gtk_tree_view_column_new_with_attributes (desc->_user_name,
                                                       renderer,
                                                       "active", id,
                                                       0,        NULL);
  }

  if (renderer && column)
  {
    g_object_set_data (G_OBJECT (renderer),
                       "PlayersList::attribute_desc", desc);

    if (_rights & SORTABLE)
    {
      Player::AttributeId *attr_id = Player::AttributeId::CreateAttributeId (desc, this);

      gtk_tree_view_column_set_sort_column_id (column,
                                               id);

      gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (_store),
                                       id,
                                       (GtkTreeIterCompareFunc) CompareIterator,
                                       attr_id,
                                       (GDestroyNotify) Object::TryToRelease);
    }

    gtk_tree_view_insert_column (GTK_TREE_VIEW (_tree_view),
                                 column,
                                 at);
  }
}

// --------------------------------------------------------------------------------
void PlayersList::SetAttributeRight (gchar    *name,
                                     gboolean  modifiable)
{
  AttributeDesc *desc = AttributeDesc::GetDesc (name);

  if (desc)
  {
    desc->SetData (this,
                   "modifiable",
                   (void *) modifiable);
  }
}

// --------------------------------------------------------------------------------
void PlayersList::SetSensitiveState (bool sensitive_value)
{
  {
    GList *columns = gtk_tree_view_get_columns (GTK_TREE_VIEW (_tree_view));

    for (guint c = 0; c < g_list_length (columns) ; c++)
    {
      GList             *renderers;
      GtkTreeViewColumn *column;

      column = GTK_TREE_VIEW_COLUMN (g_list_nth_data (columns, c));
      renderers = gtk_tree_view_column_get_cell_renderers (column);

      for (guint r = 0; r < g_list_length (renderers) ; r++)
      {
        GtkCellRenderer *renderer;
        gchar           *sensitive_attribute;

        renderer = GTK_CELL_RENDERER (g_list_nth_data (renderers, r));

        sensitive_attribute = (gchar *) g_object_get_data (G_OBJECT (renderer),
                                                           "PlayersList::SensitiveAttribute");
        g_object_set (renderer,
                      sensitive_attribute, sensitive_value,
                      NULL);
      }

      g_list_free (renderers);
    }

    g_list_free (columns);
  }
}

// --------------------------------------------------------------------------------
GtkTreeRowReference *PlayersList::GetPlayerRowRef (GtkTreeIter *iter)
{
  GtkTreeRowReference *ref;
  GtkTreePath         *path = gtk_tree_model_get_path (GTK_TREE_MODEL (_store),
                                                       iter);

  ref = gtk_tree_row_reference_new (GTK_TREE_MODEL (_store),
                                    path);

  gtk_tree_path_free (path);

  return ref;
}

// --------------------------------------------------------------------------------
void PlayersList::Add (Player *player)
{
  GtkTreeIter iter;

  player->Retain ();

  gtk_list_store_append (_store, &iter);

  {
    gchar               *str;
    Player::AttributeId  attr_id ("ref");

    str = g_strdup_printf ("%d", player->GetRef ());
    player->SetAttributeValue (&attr_id, str);
    g_free (str);
  }

  player->SetData (this, "tree_row_ref",
                   GetPlayerRowRef (&iter),
                   (GDestroyNotify) gtk_tree_row_reference_free);

  _player_list = g_slist_append (_player_list,
                                 player);

  Update (player);
}

// --------------------------------------------------------------------------------
void PlayersList::Wipe ()
{
  for (guint i = 0; i < g_slist_length (_player_list); i++)
  {
    Player *player;

    player = (Player *) g_slist_nth_data (_player_list,
                                          i);
    player->Release ();
  }

  g_slist_free (_player_list);
  _player_list = NULL;

  gtk_list_store_clear (_store);
}

// --------------------------------------------------------------------------------
void PlayersList::RemoveSelection ()
{
  GList            *ref_list       = NULL;
  GList            *selection_list = NULL;
  GtkTreeSelection *selection      = gtk_tree_view_get_selection (GTK_TREE_VIEW (_tree_view));


  selection_list = gtk_tree_selection_get_selected_rows (selection,
                                                         NULL);

  for (guint i = 0; i < g_list_length (selection_list); i++)
  {
    GtkTreeRowReference *ref;

    ref = gtk_tree_row_reference_new (GTK_TREE_MODEL (_store),
                                      (GtkTreePath *) g_list_nth_data (selection_list, i));
    ref_list = g_list_append (ref_list,
                              ref);
  }

  for (guint i = 0; i < g_list_length (ref_list); i++)
  {
    GtkTreeRowReference *ref;
    GtkTreePath         *selected_path;
    GtkTreePath         *current_path;

    ref = (GtkTreeRowReference *) g_list_nth_data (ref_list, i);
    selected_path = gtk_tree_row_reference_get_path (ref);
    gtk_tree_row_reference_free (ref);

    for (guint i = 0; i <  g_slist_length (_player_list); i++)
    {
      Player *p;

      p = (Player *) g_slist_nth_data (_player_list, i);
      current_path = gtk_tree_row_reference_get_path ((GtkTreeRowReference *) p->GetData (this, "tree_row_ref"));

      if (gtk_tree_path_compare (selected_path,
                                 current_path) == 0)
      {
        GtkTreeIter iter;

        gtk_tree_model_get_iter (GTK_TREE_MODEL (_store),
                                 &iter,
                                 selected_path);

        gtk_list_store_remove (_store,
                               &iter);

        _player_list = g_slist_remove (_player_list,
                                       p);
        OnPlayerRemoved (p);
        p->Release ();
        break;
      }
      gtk_tree_path_free (current_path);
    }
    gtk_tree_path_free (selected_path);
  }

  g_list_free (ref_list);

  g_list_foreach (selection_list, (GFunc) gtk_tree_path_free, NULL);
  g_list_free    (selection_list);

  OnListChanged ();
}

// --------------------------------------------------------------------------------
Player *PlayersList::GetPlayer (const gchar *path_string)
{
  GtkTreePath *path;
  Player      *result = NULL;

  path = gtk_tree_path_new_from_string (path_string);
  for (guint i = 0; i < g_slist_length (_player_list); i++)
  {
    GtkTreeRowReference *current_ref;
    GtkTreePath         *current_path;
    Player              *p;

    p = (Player *) g_slist_nth_data (_player_list, i);
    current_ref = (GtkTreeRowReference *) p->GetData (this, "tree_row_ref");
    current_path = gtk_tree_row_reference_get_path (current_ref);
    if (gtk_tree_path_compare (path,
                               current_path) == 0)
    {
      result = p;
      gtk_tree_path_free (current_path);
      break;
    }

    gtk_tree_path_free (current_path);
  }
  gtk_tree_path_free (path);

  return result;
}

// --------------------------------------------------------------------------------
guint PlayersList::GetNbPlayerPerPage (GtkPrintContext *context)
{
  gdouble paper_w  = gtk_print_context_get_width (context);
  gdouble paper_h  = gtk_print_context_get_height (context);
  gdouble player_h = PRINT_FONT_HEIGHT + PRINT_FONT_HEIGHT/3.0;

  return guint ((100.0*paper_h/paper_w - (PRINT_HEADER_HEIGHT+1.0+PRINT_FONT_HEIGHT))/player_h);
}

// --------------------------------------------------------------------------------
void PlayersList::OnBeginPrint (GtkPrintOperation *operation,
                                GtkPrintContext   *context)
{
  guint nb_players  = g_slist_length (_player_list);
  guint nb_per_page = GetNbPlayerPerPage (context);
  guint nb_pages    = nb_players / nb_per_page;

  if (nb_players % nb_per_page != 0)
  {
    nb_pages++;
  }

  gtk_print_operation_set_n_pages (operation,
                                   nb_pages);
}

// --------------------------------------------------------------------------------
void PlayersList::OnDrawPage (GtkPrintOperation *operation,
                              GtkPrintContext   *context,
                              gint               page_nr)
{
  cairo_t *cr = gtk_print_context_get_cairo_context (context);

  Module::OnDrawPage (operation,
                      context,
                      page_nr);

  cairo_save (cr);
  {
    guint          nb_per_page    = GetNbPlayerPerPage (context);
    GSList        *current_player = g_slist_nth (_player_list, page_nr*nb_per_page);
    GooCanvas     *canvas         = Canvas::CreatePrinterCanvas (context);
    GooCanvasItem *table          = goo_canvas_table_new (goo_canvas_get_root_item (canvas),
                                                          "column-spacing", PRINT_FONT_HEIGHT,
                                                          "row-spacing", PRINT_FONT_HEIGHT/3.0,
                                                          NULL);

    {
      cairo_matrix_t matrix;

      goo_canvas_item_get_transform (goo_canvas_get_root_item (canvas),
                                     &matrix);
      cairo_matrix_translate (&matrix,
                              1.0,
                              PRINT_HEADER_HEIGHT+1.0);
      goo_canvas_item_set_transform (goo_canvas_get_root_item (canvas),
                                     &matrix);
    }

    {
      GSList *current_attr = _filter->GetSelectedAttrList ();

      for (guint i = 0; current_attr != NULL; i++)
      {
        AttributeDesc *desc;

        desc = (AttributeDesc *) current_attr->data;
        if (desc)
        {
          GooCanvasItem *text;

          text = Canvas::PutTextInTable (table,
                                         desc->_user_name,
                                         0,
                                         i);
          g_object_set (G_OBJECT (text),
                        "font", "Sans Bold Italic 2px",
                        NULL);
          Canvas::SetTableItemAttribute (text, "x-align", 0.5);
        }
        current_attr = g_slist_next (current_attr);
      }
    }

    for (guint i = 0; (i < nb_per_page) && (current_player != NULL); i++)
    {
      Player *player;

      player = (Player *) current_player->data;
      {
        GSList *current_attr = _filter->GetSelectedAttrList ();

        for (guint j = 0; current_attr != NULL; j++)
        {
          AttributeDesc       *desc;
          Attribute           *attr;
          Player::AttributeId *attr_id;

          desc = (AttributeDesc *) current_attr->data;
          if (desc->_scope == AttributeDesc::GLOBAL)
          {
            attr_id = new Player::AttributeId (desc->_xml_name);
          }
          else
          {
            attr_id = new Player::AttributeId (desc->_xml_name,
                                               GetDataOwner ());
          }

          attr = player->GetAttribute (attr_id);
          attr_id->Release ();

          if (attr)
          {
            GooCanvasItem *item;

            if (desc->_type == G_TYPE_BOOLEAN)
            {
              if (attr->GetValue () != 0)
              {
                GooCanvasBounds bounds;
                gdouble         h;
                gdouble         paper_w = gtk_print_context_get_width (context);

                item = Canvas::PutStockIconInTable (table,
                                                    GTK_STOCK_APPLY,
                                                    i+1, j);
                goo_canvas_item_get_bounds (item,
                                            &bounds);
                h = bounds.y2-bounds.y1;

                goo_canvas_item_set_simple_transform (item,
                                                      0.0, 0.0,
                                                      paper_w*(PRINT_FONT_HEIGHT/100.0)/h,
                                                      0.0);
              }
            }
            else
            {
              gchar *font = g_strdup_printf ("Sans %dpx", guint (PRINT_FONT_HEIGHT));

              item = Canvas::PutTextInTable (table,
                                             attr->GetUserImage (),
                                             i+1,
                                             j);
              g_object_set (G_OBJECT (item),
                            "font", font,
                            NULL);
              g_free (font);
            }
          }
          current_attr = g_slist_next (current_attr);
        }
      }

      current_player = g_slist_next (current_player);
    }

    Canvas::FitToContext (goo_canvas_get_root_item (canvas),
                          context);

    goo_canvas_render (canvas,
                       cr,
                       NULL,
                       1.0);

    gtk_widget_destroy (GTK_WIDGET (canvas));
  }
  cairo_restore (cr);
}

// --------------------------------------------------------------------------------
gboolean PlayersList::OnPreview (GtkPrintOperation        *operation,
                                 GtkPrintOperationPreview *preview,
                                 GtkPrintContext          *context,
                                 GtkWindow                *parent)
{
  return TRUE;
}

// --------------------------------------------------------------------------------
void PlayersList::OnEndPrint (GtkPrintOperation *operation,
                              GtkPrintContext   *context)
{
}
