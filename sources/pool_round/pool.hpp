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

#ifndef pool_hpp
#define pool_hpp

#include <gtk/gtk.h>
#include <goocanvas.h>

#include "data.hpp"
#include "stage.hpp"
#include "canvas_module.hpp"
#include "score_collector.hpp"
#include "player.hpp"

class PoolMatchOrder;

class Pool : public CanvasModule
{
  public:
    typedef enum
    {
      WITH_POOL_NR  = 0x1,
      WITH_CALCULUS = 0x2,
      WITH_RANDOM   = 0x4
    } ComparisonPolicy;

  public:
    typedef void (*StatusCbk) (Pool *pool,
                               void *data);

    Pool (Data    *max_score,
          guint    number,
          gchar    weapon_code,
          guint32  rand_seed);

    void  AddFencer     (Player *player, Object *rank_owner);
    void  AddReferee    (Player *player);
    void  CreateMatchs  (AttributeDesc *affinity_criteria);
    void  RemoveFencer  (Player *player);
    void  RemoveReferee (Player *player);
    guint GetNbPlayers  ();
    guint GetNbMatchs   ();
    guint GetNumber     ();
    void  DropPlayer    (Player *player, gchar *reason);
    void  RestorePlayer (Player *player);
    void  CleanScores   ();
    void  DeleteMatchs  ();
    void  Lock          ();
    void  UnLock        ();
    void  SetDataOwner  (Object *single_owner,
                         Object *combined_owner,
                         Object *combined_source_owner);
    void  SetStatusCbk  (StatusCbk  cbk,
                         void      *data);
    void OnStatusChanged (GtkComboBox *combo_box);
    void CopyPlayersStatus (Object *from);

    gboolean IsOver ();
    gboolean HasError ();

    void RefreshScoreData ();

    gchar *GetName ();

    void Wipe ();

    void Save (xmlTextWriter *xml_writer);

    void Load (xmlNode *xml_node,
               GSList  *player_list);

    void Stuff ();

    void DrawPage (GtkPrintOperation *operation,
                   GtkPrintContext   *context,
                   gint               page_nr);

    GSList *GetFencerList ();

    GSList *GetRefereeList ();

    static gint ComparePlayer (Player   *A,
                               Player   *B,
                               Object   *data_owner,
                               guint32   rand_seed,
                               Object   *main_data_owner,
                               guint     comparison_policy);

  private:
    Object         *_single_owner;
    Object         *_combined_source_owner;
    Data           *_max_score;
    guint           _number;
    GSList         *_fencer_list;
    GSList         *_sorted_fencer_list;
    GSList         *_referee_list;
    ScoreCollector *_score_collector;
    GSList         *_match_list;
    gchar          *_name;
    gboolean        _is_over;
    gboolean        _has_error;
    GooCanvasItem  *_title_table;
    GooCanvasItem  *_status_item;
    gboolean        _locked;
    GSList         *_display_data;
    guint           _nb_drop;
    PoolMatchOrder *_match_order;

    void           *_status_cbk_data;
    StatusCbk       _status_cbk;

  private:
    typedef enum
    {
      AVERAGE,
      SUM
    } CombinedOperation;

    static gint _ComparePlayer (Player *A,
                                Player *B,
                                Pool   *pool);

    static gint _ComparePlayerWithFullRandom (Player *A,
                                              Player *B,
                                              Pool   *pool);

    void OnPlugged ();

    void OnUnPlugged ();

    void SortPlayers ();

    Player *GetPlayer (guint   i,
                       GSList *in_list);

    void SetDisplayData (Player      *player,
                         GooCanvas   *on_canvas,
                         const gchar *name,
                         void        *value);

    void Draw (GooCanvas *on_canvas,
               gboolean   print_for_referees);

    Match *GetMatch (Player *A,
                     Player *B);

    Match *GetMatch (guint i);

    static void OnNewScore (ScoreCollector *score_collector,
                            CanvasModule   *client,
                            Match          *match,
                            Player         *player);

    void RefreshDashBoard ();

    void RefreshAttribute (Player            *player,
                           const gchar       *name,
                           guint              value,
                           CombinedOperation  operation);

    static gint CompareMatch (Match *a,
                              Match *b,
                              Pool  *pool);

    static void on_status_changed (GtkComboBox *combo_box,
                                   Pool        *pool);

    static gboolean on_status_scrolled (GtkWidget *widget,
                                        GdkEvent  *event,
                                        gpointer   user_data);

    ~Pool ();
};

#endif