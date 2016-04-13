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

#ifndef hall_hpp
#define hall_hpp

#include "util/canvas_module.hpp"
#include "network/message.hpp"
#include "piste.hpp"
#include "batch.hpp"
#include "timeline.hpp"

class RefereePool;

class Hall :
  public CanvasModule,
  public Piste::Listener,
  public Batch::Listener,
  public Timeline::Listener
{
  public:
    Hall (RefereePool *referee_pool);

    void AddPiste ();

    void RemovePiste (Piste *piste);

    void TranslateSelected (gdouble tx,
                            gdouble ty);

    void RotateSelected ();

    void RemoveSelected ();

    void AlignSelectedOnGrid ();

    void ManageContest (Net::Message *message,
                        GtkNotebook  *notebook);

    void DropContest (Net::Message *message);

    void ManageJob (Net::Message *message);

    void DropJob (Net::Message *message);

  private:
    GooCanvasItem *_root;
    GList         *_piste_list;
    GList         *_selected_list;
    gboolean       _dragging;
    gdouble        _drag_x;
    gdouble        _drag_y;
    GList         *_batch_list;
    RefereePool   *_referee_pool;
    Timeline      *_timeline;

    ~Hall ();

    void OnPlugged ();

    void CancelSeletion ();

    Batch *GetBatch (const gchar *id);

    Batch *GetBatch (guint id);

    Object *GetDropObjectFromRef (guint32 ref,
                                  guint   key);

    void DropObject (Object   *object,
                     DropZone *source_zone,
                     DropZone *target_zone);

    static gboolean OnSelected (GooCanvasItem  *goo_rect,
                                GooCanvasItem  *target,
                                GdkEventButton *event,
                                Hall           *hall);

    static gboolean OnMotionNotify (GooCanvasItem  *item,
                                    GooCanvasItem  *target,
                                    GdkEventMotion *event,
                                    Hall           *hall);

    void OnPisteButtonEvent (Piste          *piste,
                             GdkEventButton *event);

    void OnPisteMotionEvent (Piste          *piste,
                             GdkEventMotion *event);

    void OnBatchAssignmentRequest (Batch *batch);

    void OnBatchAssignmentCancel (Batch *batch);

    void OnTimelineCursorMoved ();
};

#endif