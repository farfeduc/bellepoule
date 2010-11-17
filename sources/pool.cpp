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
#include <gdk/gdkkeysyms.h>
#include <goocanvas.h>
#include <cairo.h>
#include <cairo-pdf.h>

#include "match.hpp"
#include "pool_match_order.hpp"
#include "pool.hpp"

// --------------------------------------------------------------------------------
Pool::Pool (Data  *max_score,
            guint  number)
  : CanvasModule ("pool.glade",
                  "canvas_scrolled_window")
{
  _number      = number;
  _player_list = NULL;
  _match_list  = NULL;
  _is_over     = FALSE;
  _has_error   = FALSE;
  _title_table = NULL;
  _status_item = NULL;
  _locked      = FALSE;
  _max_score   = max_score;

  _status_cbk_data = NULL;
  _status_cbk      = NULL;

  _score_collector = NULL;

  _name = g_strdup_printf (gettext ("Pool #%02d"), _number);
}

// --------------------------------------------------------------------------------
Pool::~Pool ()
{
  Wipe ();

  g_free (_name);

  g_slist_free (_player_list);

  for (guint i = 0; i < g_slist_length (_match_list); i++)
  {
    Match *match;

    match = (Match *) g_slist_nth_data (_match_list, i);
    Object::TryToRelease (match);
  }
  g_slist_free (_match_list);

  Object::TryToRelease (_score_collector);
}

// --------------------------------------------------------------------------------
void Pool::Wipe ()
{
  _title_table = NULL;

  CanvasModule::Wipe ();
}

// --------------------------------------------------------------------------------
void Pool::Stuff ()
{
  for (guint i = 0; i < g_slist_length (_match_list); i++)
  {
    Match  *match;
    Player *A;
    Player *B;
    gint    score;

    match = (Match *) g_slist_nth_data (_match_list, i);
    A     = match->GetPlayerA ();
    B     = match->GetPlayerB ();
    score = g_random_int_range (0,
                                _max_score->_value);

    if (g_random_boolean ())
    {
      match->SetScore (A, _max_score->_value, TRUE);
      match->SetScore (B, score, FALSE);
    }
    else
    {
      match->SetScore (A, score, FALSE);
      match->SetScore (B, _max_score->_value, TRUE);
    }
  }

  RefreshScoreData ();
}

// --------------------------------------------------------------------------------
void Pool::SetStatusCbk (StatusCbk  cbk,
                         void      *data)
{
  _status_cbk_data = data;
  _status_cbk      = cbk;

  if (_status_cbk)
  {
    _status_cbk (this,
                 _status_cbk_data);
  }
}

// --------------------------------------------------------------------------------
guint Pool::GetNumber ()
{
  return _number;
}

// --------------------------------------------------------------------------------
gchar *Pool::GetName ()
{
  return _name;
}

// --------------------------------------------------------------------------------
void Pool::SetDataOwner (Object *single_owner,
                         Object *combined_owner,
                         Object *combined_source_owner)
{
  Module::SetDataOwner (combined_owner);

  _single_owner          = single_owner;
  _combined_source_owner = combined_source_owner;
}

// --------------------------------------------------------------------------------
void Pool::AddPlayer (Player *player,
                      Object *rank_owner)
{
  if (player == NULL)
  {
    return;
  }

  if (   (_player_list == NULL)
         || (g_slist_find (_player_list,
                           player) == NULL))
  {
    Player::AttributeId attr_id ("previous_stage_rank",
                                 rank_owner);

    for (guint i = 0; i < GetNbPlayers (); i++)
    {
      Player *current_player;
      Match  *match;

      current_player = GetPlayer (i);
      match = new Match (player,
                         current_player,
                         _max_score);

      _match_list = g_slist_append (_match_list,
                                    match);
    }

    attr_id.MakeRandomReady (_rand_seed);
    _player_list = g_slist_insert_sorted_with_data (_player_list,
                                                    player,
                                                    (GCompareDataFunc) Player::Compare,
                                                    &attr_id);
    player->SetData (rank_owner,
                     "Pool No",
                     (void *) GetNumber ());
  }
}

// --------------------------------------------------------------------------------
gint Pool::GetPosition (Player *player)
{
  return g_slist_index (_player_list,
                        player);
}

// --------------------------------------------------------------------------------
void Pool::RemovePlayer (Player *player)
{
  if (g_slist_find (_player_list,
                    player))
  {
    _player_list = g_slist_remove (_player_list,
                                   player);

    {
      GSList *node = _match_list;

      while (node)
      {
        GSList  *next_node;
        Match   *match;

        next_node = g_slist_next (node);

        match = (Match *) node->data;
        if (match->HasPlayer (player))
        {
          _match_list = g_slist_delete_link (_match_list,
                                             node);
          match->Release ();
        }

        node = next_node;
      }
    }
  }
}

// --------------------------------------------------------------------------------
guint Pool::GetNbPlayers ()
{
  return g_slist_length (_player_list);
}

// --------------------------------------------------------------------------------
Player *Pool::GetPlayer (guint i)
{
  return (Player *) g_slist_nth_data (_player_list,
                                      i);
}

// --------------------------------------------------------------------------------
void Pool::OnNewScore (ScoreCollector *score_collector,
                       CanvasModule   *client,
                       Match          *match,
                       Player         *player)
{
  Pool *pool = dynamic_cast <Pool *> (client);

  pool->RefreshScoreData ();
  pool->RefreshDashBoard ();
}

// --------------------------------------------------------------------------------
void Pool::Draw (GooCanvas *on_canvas,
                 gboolean   print_for_referees)
{
  if (_score_collector)
  {
    for (guint i = 0; i < g_slist_length (_match_list); i++)
    {
      Match *match;

      match = (Match *) g_slist_nth_data (_match_list, i);
      _score_collector->RemoveCollectingPoints (match);
    }

    _score_collector->Release ();
  }

  _score_collector = new ScoreCollector (this,
                                         (ScoreCollector::OnNewScore_cbk) &Pool::OnNewScore);

  {
    const guint    cell_w     = 40;
    const guint    cell_h     = 40;
    guint          nb_players = GetNbPlayers ();
    GooCanvasItem *root_item  = goo_canvas_get_root_item (on_canvas);
    GooCanvasItem *grid_group = goo_canvas_group_new (root_item, NULL);

    // Grid
    {
      // Border
      {
        goo_canvas_rect_new (grid_group,
                             0, 0,
                             nb_players * cell_w, nb_players * cell_h,
                             "line-width", 5.0,
                             NULL);
      }

      // Cells
      {
        GooCanvasItem *previous_goo_rect = NULL;

        for (guint i = 0; i < nb_players; i++)
        {
          Player *A;

          A = GetPlayer (i);

          for (guint j = 0; j < nb_players; j++)
          {
            GooCanvasItem *goo_rect;
            GooCanvasItem *score_text;
            gint           x, y;

            x = j * cell_w;
            y = i * cell_h;

            goo_rect = goo_canvas_rect_new (grid_group,
                                            x, y,
                                            cell_w, cell_h,
                                            "line-width", 2.0,
                                            "pointer-events", GOO_CANVAS_EVENTS_VISIBLE,
                                            NULL);

            if (i != j)
            {
              {
                Player *B = GetPlayer (j);
                Match  *match;

                match = GetMatch (A, B);

                // Text
                {
                  gchar *score_image;

                  if (print_for_referees)
                  {
                    score_image = g_strdup (match->GetName ());
                  }
                  else
                  {
                    Score *score = match->GetScore (A);

                    score_image = score->GetImage ();
                  }
                  score_text = goo_canvas_text_new (grid_group,
                                                    score_image,
                                                    x + cell_w / 2,
                                                    y + cell_h / 2,
                                                    -1,
                                                    GTK_ANCHOR_CENTER,
                                                    "font", "Sans bold 18px",
                                                    NULL);
                  if (print_for_referees)
                  {
                    g_object_set (score_text,
                                  "fill-color", "Grey",
                                  NULL);
                  }
                  g_free (score_image);
                }

                if ((print_for_referees == FALSE) && (_locked == FALSE))
                {
                  _score_collector->AddCollectingPoint (goo_rect,
                                                        score_text,
                                                        match,
                                                        A);

                  if (previous_goo_rect)
                  {
                    _score_collector->SetNextCollectingPoint (previous_goo_rect,
                                                              goo_rect);
                  }
                }

                previous_goo_rect = goo_rect;
              }
            }
            else
            {
              g_object_set (goo_rect, "fill-color", "grey", NULL);
            }
          }
        }
      }

      // Players (vertically)
      for (guint i = 0; i < nb_players; i++)
      {
        gint      x, y;
        GString  *image;

        x = - 5;
        y = cell_h / 2 + i * cell_h;
        image = GetPlayerImage (GetPlayer (i));

        {
          gchar *index = g_strdup_printf (" %d", i+1);

          image = g_string_append (image,
                                   index);
          g_free (index);
        }

        goo_canvas_text_new (grid_group,
                             image->str,
                             x, y, -1,
                             GTK_ANCHOR_EAST,
                             "font", "Sans 18px",
                             NULL);
        g_string_free (image,
                       TRUE);
      }

      // Players (horizontally)
      for (guint i = 0; i < nb_players; i++)
      {
        GooCanvasItem *goo_text;
        gint           x, y;
        gchar         *text;

        x = cell_w / 2 + i * cell_w;;
        y = - 13;
        text = g_strdup_printf ("%d", i+1);

        goo_text = goo_canvas_text_new (grid_group,
                                        text,
                                        x, y, -1,
                                        GTK_ANCHOR_WEST,
                                        "font", "Sans 18px",
                                        NULL);
        g_free (text);
      }

      if (print_for_referees)
      {
        GooCanvasBounds grid_bounds;

        goo_canvas_item_get_bounds (grid_group,
                                    &grid_bounds);

        for (guint i = 0; i < nb_players; i++)
        {
          goo_canvas_rect_new (grid_group,
                               grid_bounds.x2 + cell_w,
                               i*cell_h + 2,
                               cell_w*5,
                               cell_h-4,
                               "fill-color", "Grey85",
                               "line-width", 0.0,
                               NULL);
          goo_canvas_text_new (grid_group,
                               gettext ("Signature"),
                               grid_bounds.x2 + cell_w,
                               cell_h * (i + (1.0/2.0)),
                               -1,
                               GTK_ANCHOR_W,
                               "fill-color", "Grey95",
                               "font", "Sans bold 30.0px",
                               NULL);
        }
      }
    }

    // Dashboard
    if (print_for_referees == FALSE)
    {
      GooCanvasItem *dashboard_group = goo_canvas_group_new (root_item,
                                                             NULL);

      {
        GooCanvasItem *goo_text;
        gint           x, y;

        x = cell_w/2;
        y = - 10;

        goo_text = goo_canvas_text_new (dashboard_group,
                                        gettext ("Withdrawal"),
                                        0.0, y, -1,
                                        GTK_ANCHOR_WEST,
                                        "font", "Sans 18px",
                                        NULL);
        goo_canvas_item_rotate (goo_text, 315, 0.0, y);
        x += cell_w;

        goo_text = goo_canvas_text_new (dashboard_group,
                                        gettext ("Victories"),
                                        x, y, -1,
                                        GTK_ANCHOR_WEST,
                                        "font", "Sans 18px",
                                        NULL);
        goo_canvas_item_rotate (goo_text, 315, x, y);
        x += cell_w;

        goo_text = goo_canvas_text_new (dashboard_group,
                                        gettext ("Vict./Matchs"),
                                        x, y, -1,
                                        GTK_ANCHOR_WEST,
                                        "font", "Sans 18px",
                                        NULL);
        goo_canvas_item_rotate (goo_text, 315, x, y);
        x += cell_w;

        goo_text = goo_canvas_text_new (dashboard_group,
                                        gettext ("H. scored"),
                                        x, y, -1,
                                        GTK_ANCHOR_WEST,
                                        "font", "Sans 18px",
                                        NULL);
        goo_canvas_item_rotate (goo_text, 315, x, y);
        x += cell_w;

        goo_text = goo_canvas_text_new (dashboard_group,
                                        gettext ("H. recieved"),
                                        x, y, -1,
                                        GTK_ANCHOR_WEST,
                                        "font", "Sans 18px",
                                        NULL);
        goo_canvas_item_rotate (goo_text, 315, x, y);
        x += cell_w;

        goo_text = goo_canvas_text_new (dashboard_group,
                                        gettext ("Index"),
                                        x, y, -1,
                                        GTK_ANCHOR_WEST,
                                        "font", "Sans 18px",
                                        NULL);
        goo_canvas_item_rotate (goo_text, 315, x, y);
        x += cell_w;

        goo_text = goo_canvas_text_new (dashboard_group,
                                        gettext ("Place"),
                                        x, y, -1,
                                        GTK_ANCHOR_WEST,
                                        "font", "Sans 18px",
                                        NULL);
        goo_canvas_item_rotate (goo_text, 315, x, y);
        x += cell_w;
      }

      for (guint i = 0; i < nb_players; i++)
      {
        Player *player;

        player = GetPlayer (i);

        {
          GooCanvasItem *goo_item;
          gint           x, y;

          x = cell_w/2;
          y = cell_h/2 + i*cell_h;

          {
            GtkWidget *w = gtk_check_button_new ();

            g_object_set_data (G_OBJECT (w), "player",  player);
            g_signal_connect (w, "toggled",
                              G_CALLBACK (on_withdrawal_toggled), this);
            goo_item = goo_canvas_widget_new (dashboard_group,
                                              w,
                                              x-cell_w/2,
                                              y,
                                              cell_w,
                                              cell_h,
                                              "anchor", GTK_ANCHOR_CENTER,
                                              NULL);
            player->SetData (GetDataOwner (), "WithdrawalItem",  goo_item);
            x += cell_w;

            goo_item = goo_canvas_text_new (dashboard_group,
                                            ".",
                                            x, y, -1,
                                            GTK_ANCHOR_CENTER,
                                            "font", "Sans 18px",
                                            NULL);
            player->SetData (GetDataOwner (), "VictoriesItem",  goo_item);
            x += cell_w;

            goo_item = goo_canvas_text_new (dashboard_group,
                                            ".",
                                            x, y, -1,
                                            GTK_ANCHOR_CENTER,
                                            "font", "Sans 18px",
                                            NULL);
            player->SetData (GetDataOwner (), "VictoriesRatioItem",  goo_item);
            x += cell_w;

            goo_item = goo_canvas_text_new (dashboard_group,
                                            ".",
                                            x, y, -1,
                                            GTK_ANCHOR_CENTER,
                                            "font", "Sans 18px",
                                            NULL);
            player->SetData (GetDataOwner (), "HSItem",  goo_item);
            x += cell_w;

            goo_item = goo_canvas_text_new (dashboard_group,
                                            ".",
                                            x, y, -1,
                                            GTK_ANCHOR_CENTER,
                                            "font", "Sans 18px",
                                            NULL);
            player->SetData (GetDataOwner (), "HRItem",  goo_item);
            x += cell_w;

            goo_item = goo_canvas_text_new (dashboard_group,
                                            ".",
                                            x, y, -1,
                                            GTK_ANCHOR_CENTER,
                                            "font", "Sans 18px",
                                            NULL);
            player->SetData (GetDataOwner (), "IndiceItem",  goo_item);
            x += cell_w;

            goo_item = goo_canvas_text_new (dashboard_group,
                                            ".",
                                            x, y, -1,
                                            GTK_ANCHOR_CENTER,
                                            "font", "Sans 18px",
                                            NULL);
            player->SetData (GetDataOwner (), "RankItem",  goo_item);
            x += cell_w;
          }
        }
      }

      {
        GooCanvasBounds grid_bounds;
        GooCanvasBounds dashboard_bounds;

        goo_canvas_item_get_bounds (grid_group,
                                    &grid_bounds);
        goo_canvas_item_get_bounds (dashboard_group,
                                    &dashboard_bounds);

        goo_canvas_item_translate (GOO_CANVAS_ITEM (dashboard_group),
                                   grid_bounds.x2 - dashboard_bounds.x1 + cell_w/2,
                                   0);
      }
    }

    // Matches
    if (print_for_referees)
    {
      GooCanvasBounds  bounds;
      GooCanvasItem   *match_main_table;
      GooCanvasItem   *text_item;
      const guint      nb_column = 3;

      goo_canvas_item_get_bounds (root_item,
                                  &bounds);

      match_main_table = goo_canvas_table_new (root_item,
                                               "row-spacing", (gdouble) cell_h/2,
                                               "column-spacing", (gdouble) cell_w/2, NULL);

      for (guint m = 0; m < g_slist_length (_match_list); m++)
      {
        Match         *match;
        GString       *image;
        GooCanvasItem *match_table;

        match_table = goo_canvas_table_new (match_main_table, NULL);

        match = GetMatch (m);

        {
          text_item = Canvas::PutTextInTable (match_main_table,
                                              match->GetName (),
                                              m/nb_column,
                                              m%nb_column + 2*(m%nb_column));
          g_object_set (G_OBJECT (text_item),
                        "font", "Sans bold 18px",
                        NULL);
          Canvas::SetTableItemAttribute (text_item, "y-align", 0.5);
        }

        {
          image = GetPlayerImage (match->GetPlayerA ());

          text_item = Canvas::PutTextInTable (match_table,
                                              image->str,
                                              0,
                                              0);
          g_string_free (image,
                         TRUE);
          for (guint i = 0; i < _max_score->_value; i++)
          {
            GooCanvasItem *rect = goo_canvas_rect_new (match_table,
                                                       0.0, 0.0,
                                                       10.0, 10.0,
                                                       "stroke-color", "grey",
                                                       "line-width", 1.0,
                                                       NULL);
            Canvas::PutInTable (match_table,
                                rect,
                                0,
                                i+1);
            Canvas::SetTableItemAttribute (rect, "y-align", 0.5);
          }
        }

        {
          image = GetPlayerImage (match->GetPlayerB ());
          text_item = Canvas::PutTextInTable (match_table,
                                              image->str,
                                              1,
                                              0);
          g_string_free (image,
                         TRUE);

          for (guint i = 0; i < _max_score->_value; i++)
          {
            GooCanvasItem *rect = goo_canvas_rect_new (match_table,
                                                       0.0, 0.0,
                                                       10.0, 10.0,
                                                       "stroke-color", "grey",
                                                       "line-width", 1.0,
                                                       NULL);

            Canvas::PutInTable (match_table,
                                rect,
                                1,
                                i+1);
            Canvas::SetTableItemAttribute (rect, "y-align", 0.5);
          }
        }
        Canvas::PutInTable (match_main_table,
                            match_table,
                            m/nb_column,
                            m%nb_column + 2*(m%nb_column) + 1);
        Canvas::SetTableItemAttribute (match_table, "x-expand", 1U);
        Canvas::SetTableItemAttribute (match_table, "x-fill", 1U);
        Canvas::SetTableItemAttribute (match_table, "x-shrink", 1U);
      }
      goo_canvas_item_translate (match_main_table,
                                 bounds.x1,
                                 bounds.y2 + cell_h);
    }

    // Name
    {
      GooCanvasBounds  bounds;
      GooCanvasItem   *text_item;

      _title_table = goo_canvas_table_new (root_item, NULL);
      text_item = Canvas::PutTextInTable (_title_table,
                                          _name,
                                          0,
                                          1);
      g_object_set (G_OBJECT (text_item),
                    "font", "Sans bold 18px",
                    NULL);

      goo_canvas_item_get_bounds (root_item,
                                  &bounds);

      goo_canvas_item_translate (_title_table,
                                 bounds.x1,
                                 bounds.y1);
    }
  }

  if (print_for_referees == FALSE)
  {
    RefreshScoreData ();
    RefreshDashBoard ();
  }
}

// --------------------------------------------------------------------------------
void Pool::DrawPage (GtkPrintOperation *operation,
                     GtkPrintContext   *context,
                     gint               page_nr)
{
  GooCanvas *canvas             = CreateCanvas ();
  gboolean   print_for_referees = (gboolean) g_object_get_data (G_OBJECT (operation), "print_for_referees");

  {
    GooCanvasItem *title_table = _title_table;
    GooCanvasItem *status_item = _status_item;

    title_table = NULL;
    status_item = NULL;
    Draw (canvas,
          print_for_referees);

    // Rétablissement des attributs dans leur état original.
    // Pas terrible comme technique ! Trouver autre chose !
    _title_table = title_table;
    _status_item = status_item;
  }

  g_object_set_data (G_OBJECT (operation), "operation_canvas", (void *) canvas);
  CanvasModule::OnDrawPage (operation,
                            context,
                            page_nr);

  gtk_widget_destroy (GTK_WIDGET (canvas));
}

// --------------------------------------------------------------------------------
void Pool::OnPlugged ()
{
  CanvasModule::OnPlugged ();
  Draw (GetCanvas (),
        FALSE);
}

// --------------------------------------------------------------------------------
gint Pool::_ComparePlayer (Player *A,
                           Player *B,
                           Pool   *pool)
{
  return ComparePlayer (A,
                        B,
                        pool->_single_owner,
                        pool->_rand_seed,
                        WITH_CALCULUS | WITH_RANDOM);
}

// --------------------------------------------------------------------------------
gint Pool::ComparePlayer (Player   *A,
                          Player   *B,
                          Object   *data_owner,
                          guint32   rand_seed,
                          guint     comparison_policy)
{
  if (B == NULL)
  {
    return 1;
  }
  else if (A == NULL)
  {
    return -1;
  }
  else if (comparison_policy & WITH_CALCULUS)
  {
    guint  ratio_A;
    guint  ratio_B;
    gint   average_A;
    gint   average_B;
    guint  HS_A;
    guint  HS_B;
    Player::AttributeId attr_id ("", data_owner);

    attr_id._name = "victories_ratio";
    ratio_A = (guint) A->GetAttribute (&attr_id)->GetValue ();

    attr_id._name = "victories_ratio";
    ratio_B = (guint) B->GetAttribute (&attr_id)->GetValue ();

    attr_id._name = "indice";
    average_A = (gint)  A->GetAttribute (&attr_id)->GetValue ();

    attr_id._name = "indice";
    average_B = (gint)  B->GetAttribute (&attr_id)->GetValue ();

    attr_id._name = "HS";
    HS_A = (guint) A->GetAttribute (&attr_id)->GetValue ();

    attr_id._name = "HS";
    HS_B = (guint) B->GetAttribute (&attr_id)->GetValue ();

    if (ratio_B != ratio_A)
    {
      return ratio_B - ratio_A;
    }
    if (average_B != average_A)
    {
      return average_B - average_A;
    }
    else if (HS_B != HS_A)
    {
      return HS_B - HS_A;
    }
  }

  if (comparison_policy & WITH_RANDOM)
  {
    return Player::RandomCompare (A,
                                  B,
                                  rand_seed);
  }

  return 0;
}

// --------------------------------------------------------------------------------
void Pool::Lock ()
{
  _locked = TRUE;

  RefreshScoreData ();
  if (_score_collector)
  {
    _score_collector->Lock ();
  }
}

// --------------------------------------------------------------------------------
void Pool::UnLock ()
{
  _locked = FALSE;

  if (_score_collector)
  {
    _score_collector->UnLock ();
  }
}

// --------------------------------------------------------------------------------
void Pool::RefreshScoreData ()
{
  GSList *ranking    = NULL;
  guint   nb_players = GetNbPlayers ();

  _is_over   = TRUE;
  _has_error = FALSE;

  for (guint a = 0; a < nb_players; a++)
  {
    Player *player_a;
    guint   victories     = 0;
    guint   hits_scored   = 0;
    gint    hits_received = 0;

    player_a = GetPlayer (a);

    for (guint b = 0; b < nb_players; b++)
    {
      if (a != b)
      {
        Player *player_b = GetPlayer (b);
        Match  *match    = GetMatch (player_a, player_b);
        Score  *score_a  = match->GetScore (player_a);
        Score  *score_b  = match->GetScore (player_b);

        if (score_a->IsKnown ())
        {
          hits_scored += score_a->Get ();
        }

        if (score_b->IsKnown ())
        {
          hits_received -= score_b->Get ();
        }

        if (   score_a->IsKnown ()
            && score_b->IsKnown ())
        {
          if (score_a->IsTheBest ())
          {
            victories++;
          }
        }
        else
        {
          _is_over = FALSE;
        }

        if (   (score_a->IsValid () == false)
            || (score_b->IsValid () == false)
            || (score_a->IsConsistentWith (score_b) == false))
        {
          _is_over   = FALSE;
          _has_error = TRUE;
        }
      }
    }

    player_a->SetData (GetDataOwner (), "Victories", (void *) victories);
    player_a->SetData (GetDataOwner (), "HR", (void *) hits_received);

    RefreshAttribute (player_a,
                      "victories_ratio",
                      (victories*1000 / (GetNbPlayers ()-1)),
                      AVERAGE);

    RefreshAttribute (player_a,
                      "indice",
                      hits_scored+hits_received,
                      SUM);

    RefreshAttribute (player_a,
                      "HS",
                      hits_scored,
                      SUM);

    ranking = g_slist_append (ranking,
                              player_a);
  }

  ranking = g_slist_sort_with_data (ranking,
                                    (GCompareDataFunc) _ComparePlayer,
                                    (void *) this);

  {
    Player::AttributeId  victories_ratio_id ("victories_ratio", _single_owner);
    Player::AttributeId  indice_id          ("indice", _single_owner);
    Player::AttributeId  HS_id              ("HS", _single_owner);
    GSList *current_player  = ranking;
    Player *previous_player = NULL;
    guint   previous_rank   = 0;

    for (guint p = 1; current_player != NULL; p++)
    {
      Player *player;

      player = (Player *) current_player->data;
      if (   previous_player
          && (Player::Compare (previous_player, player, &victories_ratio_id) == 0)
          && (Player::Compare (previous_player, player, &indice_id) == 0)
          && (Player::Compare (previous_player, player, &HS_id) == 0))
      {
          player->SetData (this, "Rank", (void *) (previous_rank));
      }
      else
      {
          player->SetData (this, "Rank", (void *) (p));
          previous_rank = p;
      }

      previous_player = player;
      current_player  = g_slist_next (current_player);
    }
  }

  g_slist_free (ranking);

  if (_status_cbk)
  {
    _status_cbk (this,
                 _status_cbk_data);
  }

  if (_title_table)
  {
    if (_status_item)
    {
      goo_canvas_item_remove (_status_item);
      _status_item = NULL;
    }

    if (_is_over)
    {
      _status_item = Canvas::PutStockIconInTable (_title_table,
                                                  GTK_STOCK_APPLY,
                                                  0, 0);
    }
    else if (_has_error)
    {
      _status_item = Canvas::PutStockIconInTable (_title_table,
                                                  GTK_STOCK_DIALOG_WARNING,
                                                  0, 0);
    }
    else
    {
      _status_item = Canvas::PutStockIconInTable (_title_table,
                                                  GTK_STOCK_EXECUTE,
                                                  0, 0);
    }
  }
}

// --------------------------------------------------------------------------------
void Pool::RefreshAttribute (Player            *player,
                             gchar             *name,
                             guint              value,
                             CombinedOperation  operation)
{
  Player::AttributeId  single_attr_id  (name, _single_owner);
  Player::AttributeId  combined_attr_id (name, GetDataOwner ());

  player->SetAttributeValue (&single_attr_id,
                             value);

  if (_combined_source_owner == NULL)
  {
    player->SetAttributeValue (&combined_attr_id,
                               value);
  }
  else
  {
    Player::AttributeId  source_attr_id (name, _combined_source_owner);
    Attribute           *source_attr = player->GetAttribute (&source_attr_id);

    if (operation == AVERAGE)
    {
      player->SetAttributeValue (&combined_attr_id,
                                 ((guint) source_attr->GetValue () + value) / 2);
    }
    else
    {
      player->SetAttributeValue (&combined_attr_id,
                                 (guint) source_attr->GetValue () + value);
    }
  }
}

// --------------------------------------------------------------------------------
gboolean Pool::IsOver ()
{
  return _is_over;
}

// --------------------------------------------------------------------------------
gboolean Pool::HasError ()
{
  return _has_error;
}

// --------------------------------------------------------------------------------
void Pool::RefreshDashBoard ()
{
  guint               nb_players = GetNbPlayers ();
  Player::AttributeId attr_id ("", _single_owner);

  for (guint p = 0; p < nb_players; p++)
  {
    Player        *player;
    GooCanvasItem *goo_text;
    gchar         *text;
    Attribute     *attr;
    gint           value;

    player = GetPlayer (p);

    attr_id._name = "victories_ratio";
    attr = player->GetAttribute (&attr_id);
    goo_text = GOO_CANVAS_ITEM (player->GetData (GetDataOwner (), "VictoriesRatioItem"));
    text = g_strdup_printf ("%0.3f", (gdouble) ((guint) attr->GetValue ()) / 1000.0);
    g_object_set (goo_text,
                  "text",
                  text, NULL);
    g_free (text);

    value = (gint) player->GetData (GetDataOwner (), "Victories");
    goo_text = GOO_CANVAS_ITEM (player->GetData (GetDataOwner (), "VictoriesItem"));
    text = g_strdup_printf ("%d", value);
    g_object_set (goo_text,
                  "text",
                  text, NULL);
    g_free (text);

    attr_id._name = "HS";
    attr = player->GetAttribute (&attr_id);
    goo_text = GOO_CANVAS_ITEM (player->GetData (GetDataOwner (), "HSItem"));
    text = g_strdup_printf ("%d", (guint) attr->GetValue ());
    g_object_set (goo_text,
                  "text",
                  text, NULL);
    g_free (text);

    value = (gint) player->GetData (GetDataOwner (), "HR");
    goo_text = GOO_CANVAS_ITEM (player->GetData (GetDataOwner (), "HRItem"));
    text = g_strdup_printf ("%d", -value);
    g_object_set (goo_text,
                  "text",
                  text, NULL);
    g_free (text);

    attr_id._name = "indice";
    attr = player->GetAttribute (&attr_id);
    goo_text = GOO_CANVAS_ITEM (player->GetData (GetDataOwner (), "IndiceItem"));
    text = g_strdup_printf ("%+d", (gint) attr->GetValue ());
    g_object_set (goo_text,
                  "text",
                  text, NULL);
    g_free (text);

    value = (gint) player->GetData (this, "Rank");
    goo_text = GOO_CANVAS_ITEM (player->GetData (GetDataOwner (), "RankItem"));
    text = g_strdup_printf ("%d", value);
    g_object_set (goo_text,
                  "text",
                  text, NULL);
    g_free (text);
  }
}

// --------------------------------------------------------------------------------
Match *Pool::GetMatch (Player *A,
                       Player *B)
{
  for (guint i = 0; i < g_slist_length (_match_list); i++)
  {
    Match *match;

    match = (Match *) g_slist_nth_data (_match_list, i);
    if (   match->HasPlayer (A)
        && match->HasPlayer (B))
    {
      return match;
    }
  }

  return NULL;
}

// --------------------------------------------------------------------------------
Match *Pool::GetMatch (guint i)
{
  PoolMatchOrder::PlayerPair *pair = PoolMatchOrder::GetPlayerPair (GetNbPlayers ());

  if (pair)
  {
    Player *a = (Player *) g_slist_nth_data (_player_list, pair[i]._a-1);
    Player *b = (Player *) g_slist_nth_data (_player_list, pair[i]._b-1);

    return GetMatch (a,
                     b);
  }

  return NULL;
}

// --------------------------------------------------------------------------------
gint Pool::CompareMatch (Match *a,
                         Match *b,
                         Pool  *pool)
{
  return g_slist_index (pool->_player_list, a->GetPlayerA ()) - g_slist_index (pool->_player_list, b->GetPlayerA ());
}

// --------------------------------------------------------------------------------
void Pool::Save (xmlTextWriter *xml_writer)
{
  xmlTextWriterStartElement (xml_writer,
                             BAD_CAST "Poule");
  xmlTextWriterWriteFormatAttribute (xml_writer,
                                     BAD_CAST "ID",
                                     "%d", _number);

  {
    GSList              *current = _player_list;
    Player::AttributeId  attr_id ("", _single_owner);
    Attribute           *attr;

    for (guint i = 0; current != NULL; i++)
    {
      Player *player;
      guint   HS;
      gint    indice;

      player = (Player *) current->data;

      xmlTextWriterStartElement (xml_writer,
                                 BAD_CAST "Tireur");
      xmlTextWriterWriteFormatAttribute (xml_writer,
                                         BAD_CAST "REF",
                                         "%d", player->GetRef ());
      xmlTextWriterWriteFormatAttribute (xml_writer,
                                         BAD_CAST "NoDansLaPoule",
                                         "%d", i+1);
      xmlTextWriterWriteFormatAttribute (xml_writer,
                                         BAD_CAST "NbVictoires",
                                         "%d", (gint) player->GetData (GetDataOwner (),
                                                                       "Victories"));
      xmlTextWriterWriteFormatAttribute (xml_writer,
                                         BAD_CAST "NbMatches",
                                         "%d", g_slist_length (_player_list)-1);
      attr_id._name = "HS";
      attr = player->GetAttribute (&attr_id);
      if (attr)
      {
        HS = (guint) player->GetAttribute (&attr_id)->GetValue ();
        xmlTextWriterWriteFormatAttribute (xml_writer,
                                           BAD_CAST "TD",
                                           "%d", HS);
        attr_id._name = "indice";
        indice = (guint) player->GetAttribute (&attr_id)->GetValue ();
        xmlTextWriterWriteFormatAttribute (xml_writer,
                                           BAD_CAST "TR",
                                           "%d", HS - indice);
      }
      xmlTextWriterWriteFormatAttribute (xml_writer,
                                         BAD_CAST "RangPoule",
                                         "%d", (gint) player->GetData (this,
                                                                       "Rank"));
      xmlTextWriterEndElement (xml_writer);
      current = g_slist_next (current);
    }
  }

  {
    xmlTextWriterStartElement (xml_writer,
                               BAD_CAST "Arbitre");
    xmlTextWriterEndElement (xml_writer);
  }

  // To avoid the GREG pool display issue
  // the order of the saved matchs must be consistent
  // with the order of the players.
  for (guint p1 = 0; p1 < g_slist_length (_player_list); p1++)
  {
    Player *player_1;

    player_1 = (Player *) g_slist_nth_data (_player_list, p1);
    for (guint p2 = p1+1; p2 < g_slist_length (_player_list); p2++)
    {
      Match  *match;
      Player *player_2;

      player_2 = (Player *) g_slist_nth_data (_player_list, p2);
      match = GetMatch (player_1, player_2);
      if (match)
      {
        match->SaveInOrder (xml_writer,
                            player_1);
      }
    }
  }

  xmlTextWriterEndElement (xml_writer);
}

// --------------------------------------------------------------------------------
void Pool::Load (xmlNode *xml_node,
                 GSList  *player_list)
{
  if (xml_node == NULL)
  {
    return;
  }

  _is_over = TRUE;

  for (xmlNode *n = xml_node; n != NULL; n = n->next)
  {
    if (n->type == XML_ELEMENT_NODE)
    {
      static xmlNode *A = NULL;
      static xmlNode *B = NULL;

      if (strcmp ((char *) n->name, "Match") == 0)
      {
        A = NULL;
        B = NULL;
      }
      else if (strcmp ((char *) n->name, "Tireur") == 0)
      {
        if (A == NULL)
        {
          A = n;
        }
        else
        {
          gchar  *attr;
          Player *player_A = NULL;
          Player *player_B = NULL;
          Match  *match;

          B = n;
          {
            GSList *node;

            attr = (gchar *) xmlGetProp (A, BAD_CAST "REF");
            node = g_slist_find_custom (player_list,
                                        (void *) strtol (attr, NULL, 10),
                                        (GCompareFunc) Player::CompareWithRef);
            if (node)
            {
              player_A = (Player *) node->data;
            }

            attr = (gchar *) xmlGetProp (B, BAD_CAST "REF");
            node = g_slist_find_custom (player_list,
                                        (void *) strtol (attr, NULL, 10),
                                        (GCompareFunc) Player::CompareWithRef);
            if (node)
            {
              player_B = (Player *) node->data;
            }
          }

          match = GetMatch (player_A, player_B);

          if (match)
          {
            {
              gboolean is_the_best = FALSE;

              attr = (gchar *) xmlGetProp (A, BAD_CAST "Statut");
              if (attr && attr[0] == 'V')
              {
                is_the_best = TRUE;
              }

              attr = (gchar *) xmlGetProp (A, BAD_CAST "Score");

              if (attr)
              {
                match->SetScore (player_A, atoi (attr), is_the_best);
              }
            }

            {
              gboolean is_the_best = FALSE;

              attr = (gchar *) xmlGetProp (B, BAD_CAST "Statut");
              if (attr && attr[0] == 'V')
              {
                is_the_best = TRUE;
              }

              attr = (gchar *) xmlGetProp (B, BAD_CAST "Score");
              if (attr)
              {
                match->SetScore (player_B, atoi (attr), is_the_best);
              }
            }

            if (   (match->PlayerHasScore (player_A) == FALSE)
                   || (match->PlayerHasScore (player_B) == FALSE))
            {
              _is_over = FALSE;
            }

            {
              Score *score_A = match->GetScore (player_A);
              Score *score_B = match->GetScore (player_B);

              if (   (score_A->IsValid () == false)
                     || (score_B->IsValid () == false)
                     || (score_A->IsConsistentWith (score_B) == false))
              {
                _has_error = TRUE;
                _is_over   = FALSE;
              }
            }
          }
          return;
        }
      }

      Load (n->children,
            player_list);
    }
  }
}

// --------------------------------------------------------------------------------
void Pool::CleanScores ()
{
  for (guint i = 0; i < g_slist_length (_match_list); i++)
  {
    Match *match;

    match = (Match *) g_slist_nth_data (_match_list, i);
    match->CleanScore ();
  }
  _is_over = FALSE;

  RefreshScoreData ();
}

// --------------------------------------------------------------------------------
gint Pool::_ComparePlayerWithFullRandom (Player *A,
                                         Player *B,
                                         Pool   *pool)
{
  return ComparePlayer (A,
                        B,
                        pool->GetDataOwner (),
                        pool->_rand_seed,
                        WITH_RANDOM);
}

// --------------------------------------------------------------------------------
void Pool::SortPlayers ()
{
  _player_list = g_slist_sort_with_data (_player_list,
                                         (GCompareDataFunc) _ComparePlayerWithFullRandom,
                                         (void *) this);

  for (guint i = 0; i < g_slist_length (_match_list); i++)
  {
    Match *match;

    match = GetMatch (i);
    match->SetNumber (i+1);
  }
}

// --------------------------------------------------------------------------------
void Pool::ResetMatches (Object *rank_owner)
{
  {
    Player::AttributeId attr_id ("previous_stage_rank",
                                 rank_owner);

    attr_id.MakeRandomReady (_rand_seed);
    _player_list = g_slist_sort_with_data (_player_list,
                                           (GCompareDataFunc) Player::Compare,
                                           &attr_id);
  }

  for (guint i = 0; i < g_slist_length (_match_list); i++)
  {
    Match *match;

    match = GetMatch (i);
    match->SetNumber (0);
  }
}

// --------------------------------------------------------------------------------
void Pool::DropPlayer (Player *player)
{
}

// --------------------------------------------------------------------------------
void Pool::RestorePlayer (Player *player)
{
}

// --------------------------------------------------------------------------------
void Pool::on_withdrawal_toggled (GtkToggleButton *togglebutton,
                                  Pool            *pool)
{
  Player *player = (Player *) g_object_get_data (G_OBJECT (togglebutton), "player");

  if (gtk_toggle_button_get_active (togglebutton))
  {
    pool->DropPlayer (player);
  }
  else
  {
    pool->RestorePlayer (player);
  }
}
