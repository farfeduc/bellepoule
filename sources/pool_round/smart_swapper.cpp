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

#include "object.hpp"
#include "pool.hpp"
#include "pool_zone.hpp"

#include "smart_swapper.hpp"

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
void SmartSwapper::Fencer::Dump (Object *owner)
{
  Player::AttributeId  previous_rank_attr_id ("previous_stage_rank", owner);
  Attribute *previous_stage_rank = _player->GetAttribute (&previous_rank_attr_id);

  g_print ("%d %20ss >> %s (pool #%d)\n",
           previous_stage_rank->GetUIntValue (),
           _player->GetName (),
           g_quark_to_string (_criteria_quark),
           _original_pool->_id);
}

// --------------------------------------------------------------------------------
gboolean SmartSwapper::Fencer::CanGoTo (PoolData   *pool_data,
                                        GHashTable *criteria_distribution)
{
  CriteriaData *criteria_data = (CriteriaData *) g_hash_table_lookup (criteria_distribution,
                                                                      (const void *) _criteria_quark);

  if (criteria_data == NULL)
  {
    return TRUE;
  }
  else
  {
    guint score = GPOINTER_TO_UINT (g_hash_table_lookup (pool_data->_criteria_score,
                                                         (const void *) _criteria_quark));

    if (score && _over_population_error)
    {
      if (criteria_data->_max_floating_fencers)
      {
        return score < criteria_data->_max_criteria_occurrence - 1;
      }
    }

    return score < criteria_data->_max_criteria_occurrence;
  }
}

// --------------------------------------------------------------------------------
void SmartSwapper::PoolData::ChangeCriteriaScore (GQuark criteria,
                                                  gint   delta_score)
{
  guint score = GPOINTER_TO_UINT (g_hash_table_lookup (_criteria_score,
                                                       (const void *) criteria));
  score += delta_score;

  g_hash_table_insert (_criteria_score,
                       (void *) criteria,
                       (void *) score);
}

// --------------------------------------------------------------------------------
void SmartSwapper::PoolData::AddFencer (Fencer *fencer)
{
  fencer->_new_pool = this;

  _fencer_list = g_list_append (_fencer_list,
                                fencer);
  _pool_sizes->NewSize (_size, _size+1);
  _size++;

  ChangeCriteriaScore (fencer->_criteria_quark,
                       1);
}

// --------------------------------------------------------------------------------
void SmartSwapper::PoolData::RemoveFencer (Fencer *fencer)
{
  fencer->_new_pool = NULL;

  _fencer_list = g_list_remove (_fencer_list,
                                fencer);
  _pool_sizes->NewSize (_size, _size-1);
  _size--;

  ChangeCriteriaScore (fencer->_criteria_quark,
                       -1);
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
void SmartSwapper::PoolSizes::Configure (guint nb_fencer,
                                         guint nb_pool)
{
  _min_size       = nb_fencer / nb_pool;
  _max_size       = 0;
  _nb_max_reached = 0;
  _nb_max         = nb_fencer % nb_pool;
  if (_nb_max)
  {
    _max_size = _min_size+1;
  }

  _available_sizes[MIN_SIZE] = _min_size;
  _available_sizes[MAX_SIZE] = _max_size;
  _available_sizes[END_MARK] = 0;
}

// --------------------------------------------------------------------------------
void SmartSwapper::PoolSizes::NewSize (guint old_size,
                                       guint new_size)
{
  if (old_size < new_size)
  {
    if (new_size == _max_size)
    {
      _nb_max_reached++;
    }
  }
  else if (old_size > new_size)
  {
    if (old_size == _max_size)
    {
      _nb_max_reached--;
    }
  }

  if (_nb_max_reached >= _nb_max)
  {
    _available_sizes[MAX_SIZE] = 0;
  }
  else
  {
    _available_sizes[MAX_SIZE] = _max_size;
  }
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
SmartSwapper::SmartSwapper (Object *owner)
: Object ("SmartSwapper")
{
  _owner            = owner;
  _remaining_errors = NULL;
}

// --------------------------------------------------------------------------------
SmartSwapper::~SmartSwapper ()
{
}

// --------------------------------------------------------------------------------
Swapper *SmartSwapper::Create (Object *owner)
{
  return new SmartSwapper (owner);
}

// --------------------------------------------------------------------------------
void SmartSwapper::Delete ()
{
  Release ();
}

// --------------------------------------------------------------------------------
void SmartSwapper::Swap (GSList *zones,
                         gchar  *criteria,
                         GSList *fencer_list)
{
  _criteria_id = new Player::AttributeId (criteria);

  _remaining_errors = NULL;

  _nb_pools   = 0;
  _pool_table = NULL;

  _error_list    = NULL;
  _floating_list = NULL;

  CreatePoolTable (zones);

  _pool_sizes.Configure (g_slist_length (fencer_list),
                         _nb_pools);

  LookUpDistribution (fencer_list);

  if (criteria)
  {
    FindLackOfPopulationErrors  ();
    ExtractOverPopulationErrors ();
    ExtractFloatings            ();

    DispatchErrors    ();
    DispatchFloatings ();
  }

  StoreSwapping ();

  _criteria_id->Release ();
  DeletePoolTable ();
  g_hash_table_destroy (_criteria_distribution);
}

// --------------------------------------------------------------------------------
void SmartSwapper::StoreSwapping ()
{
  for (guint i = 0; i < _nb_pools; i++)
  {
    PoolData *pool_data = &_pool_table[i];
    GList    *current   = pool_data->_fencer_list;

    while (current)
    {
      Fencer *fencer = (Fencer *) current->data;

      fencer->_player->SetData (_owner,
                               "original_pool",
                               (void *) (i+1));
      pool_data->_pool->AddFencer (fencer->_player,
                                   _owner);

      current = g_list_next (current);
    }
  }
}

// --------------------------------------------------------------------------------
void SmartSwapper::CreatePoolTable (GSList *zones)
{
  DeletePoolTable ();

  _nb_pools   = g_slist_length (zones);
  _pool_table = g_new (PoolData,
                       _nb_pools);

  {
    GSList *current = zones;

    for (guint i = 0; i < _nb_pools; i++)
    {
      PoolZone *zone = (PoolZone *) current->data;

      _pool_table[i]._pool           = zone->GetPool ();
      _pool_table[i]._fencer_list    = NULL;
      _pool_table[i]._size           = 0;
      _pool_table[i]._id             = i + 1;
      _pool_table[i]._pool_sizes     = &_pool_sizes;
      _pool_table[i]._criteria_score = g_hash_table_new (NULL,
                                                         NULL);
      current = g_slist_next (current);
    }
  }
}

// --------------------------------------------------------------------------------
void SmartSwapper::DeletePoolTable ()
{
  for (guint i = 0; i < _nb_pools; i++)
  {
    g_list_free (_pool_table[i]._fencer_list);
  }
  _nb_pools = 0;

  g_free (_pool_table);
  _pool_table = NULL;
}

// --------------------------------------------------------------------------------
void SmartSwapper::LookUpDistribution (GSList *fencer_list)
{
  _criteria_distribution = g_hash_table_new_full (NULL,
                                                  NULL,
                                                  NULL,
                                                  g_free);

  {
    GSList *current = fencer_list;

    for (guint i = 0; current != NULL; i++)
    {
      Player *player = (Player *) current->data;
      Fencer *fencer = new Fencer;
      GQuark  quark  = 0;

      // criteria
      {
        Attribute *criteria_attr = player->GetAttribute (_criteria_id);

        if (criteria_attr)
        {
          gchar *user_image = criteria_attr->GetUserImage ();

          quark = g_quark_from_string (user_image);

          InsertCriteria (_criteria_distribution,
                          quark);
          g_free (user_image);
        }
      }

      fencer->_player                = player;
      fencer->_rank                  = i;
      fencer->_criteria_quark        = quark;
      fencer->_over_population_error = FALSE;
      fencer->_new_pool              = NULL;
      fencer->_original_pool         = &_pool_table[GetPoolIndex (i)];

      //fencer->Dump (_owner);
      fencer->_original_pool->AddFencer (fencer);

      current = g_slist_next (current);
    }
  }

  g_hash_table_foreach (_criteria_distribution,
                        (GHFunc) SetExpectedData,
                        this);
}

// --------------------------------------------------------------------------------
void SmartSwapper::ExtractOverPopulationErrors ()
{
  //g_print ("\n\nExtract overpopulation ERRORS\n");

  for (guint i = 0; i < _nb_pools; i++)
  {
    PoolData       *pool_data = &_pool_table[i];
    GHashTableIter  iter;
    gpointer        key;
    gpointer        value;

    g_hash_table_iter_init (&iter,
                            pool_data->_criteria_score);

    //g_print ("   #%d\n", i+1);
    while (g_hash_table_iter_next (&iter,
                                   &key,
                                   &value))
    {
      GQuark quark = GPOINTER_TO_UINT (key);
      guint  count = GPOINTER_TO_UINT (value);
      CriteriaData *criteria_data = (CriteriaData *) g_hash_table_lookup (_criteria_distribution,
                                                                          (const void *) quark);

      if (criteria_data && (count > criteria_data->_max_criteria_occurrence))
      {
        for (guint error = 0; error < count - criteria_data->_max_criteria_occurrence; error++)
        {
          Fencer *fencer = ExtractFencer (pool_data,
                                          quark,
                                          &_error_list);

          if (fencer)
          {
            fencer->_over_population_error = TRUE;
          }
        }
      }
    }
  }

  _error_list = g_list_sort (_error_list,
                             (GCompareFunc) CompareFencerRank);
}

// --------------------------------------------------------------------------------
void SmartSwapper::FindLackOfPopulationErrors ()
{
  //g_print ("\n\nFind lack of population ERRORS\n");

  GHashTableIter iter;
  gpointer       key;
  gpointer       value;

  _lack_list = NULL;

  g_hash_table_iter_init (&iter,
                          _criteria_distribution);

  while (g_hash_table_iter_next (&iter,
                                 &key,
                                 &value))
  {
    CriteriaData *criteria_data = (CriteriaData *) value;

    if (   (criteria_data && criteria_data->_max_floating_fencers)
        && (criteria_data->_max_criteria_occurrence > 1))
    {
      GQuark quark = GPOINTER_TO_UINT (key);

      for (guint i = 0; i < _nb_pools; i++)
      {
        PoolData *pool_data = &_pool_table[i];
        guint     score;

        score = GPOINTER_TO_UINT (g_hash_table_lookup (pool_data->_criteria_score,
                                                       (const void *) quark));

        if (score < criteria_data->_max_criteria_occurrence-1)
        {

          _lack_list = g_slist_prepend (_lack_list,
                                        (void *) quark);
        }
      }
    }
  }
}

// --------------------------------------------------------------------------------
void SmartSwapper::ExtractFloatings ()
{
  //g_print ("\n\nExtract FLOATINGS\n");

  for (guint i = 0; i < _nb_pools; i++)
  {
    PoolData       *pool_data = &_pool_table[i];
    GHashTableIter  iter;
    gpointer        key;
    gpointer        value;

    g_hash_table_iter_init (&iter,
                            pool_data->_criteria_score);

    //g_print ("   #%d\n", i+1);
    while (g_hash_table_iter_next (&iter,
                                   &key,
                                   &value))
    {
      GQuark quark = GPOINTER_TO_UINT (key);
      guint  count = GPOINTER_TO_UINT (value);

      CriteriaData *criteria_data = (CriteriaData *) g_hash_table_lookup (_criteria_distribution,
                                                                          (const void *) quark);

      if (   (criteria_data == NULL)
          || (criteria_data->_max_floating_fencers && (count == criteria_data->_max_criteria_occurrence)))
      {
        ExtractFencer (pool_data,
                       quark,
                       &_floating_list);
      }
    }
  }

  _floating_list = g_list_sort (_floating_list,
                                (GCompareFunc) CompareFencerRank);

  // Transform lack of population into real errors
  {
    GSList *current_lack = _lack_list;

    while (current_lack)
    {
      GQuark  quark            = GPOINTER_TO_UINT (current_lack->data);
      GList  *current_floating = g_list_last (_floating_list);

      while (current_floating)
      {
        Fencer *fencer = (Fencer *) current_floating->data;

        if (fencer->_criteria_quark == quark)
        {
          fencer->_over_population_error = TRUE;
          break;
        }
        current_floating = g_list_previous (current_floating);
      }

      current_lack = g_slist_next (current_lack);
    }
  }
}

// --------------------------------------------------------------------------------
SmartSwapper::Fencer *SmartSwapper::ExtractFencer (PoolData  *from_pool,
                                                   GQuark     with_criteria,
                                                   GList    **to_list)
{
  GList  *current = g_list_last (from_pool->_fencer_list);
  Fencer *fencer  = (Fencer *) current->data;

  while (fencer->_criteria_quark != with_criteria)
  {
    current = g_list_previous (current);
    if (current == NULL)
    {
      return NULL;
    }
    fencer = (Fencer *) current->data;
  }

  *to_list = g_list_prepend (*to_list,
                             current->data);

  //fencer->Dump (_owner);
  from_pool->RemoveFencer ((Fencer *) current->data);

  return fencer;
}

// --------------------------------------------------------------------------------
void SmartSwapper::DispatchErrors ()
{
  //g_print ("\n\nDispatch ERRORS\n");

  DispatchFencers (_error_list);

  g_list_free (_error_list);
  _error_list = NULL;
}

// --------------------------------------------------------------------------------
void SmartSwapper::DispatchFloatings ()
{
  GSList *temp_list = NULL;

  //g_print ("\n\nDispatch FLOATINGS\n");

  DispatchFencers (_floating_list);

  // Remaining errors
  {
    GSList *current_error = _remaining_errors;

    while (current_error)
    {
      Fencer *error            = (Fencer *) current_error->data;
      GList  *current_floating = g_list_last (_floating_list);
      CriteriaData *criteria_data;

      criteria_data = (CriteriaData *) g_hash_table_lookup (_criteria_distribution,
                                                            (const void *) error->_criteria_quark);

      while (current_floating)
      {
        Fencer *floating = (Fencer *) current_floating->data;

        if (floating->_new_pool)
        {
          guint   score;

          score = GPOINTER_TO_UINT (g_hash_table_lookup (floating->_new_pool->_criteria_score,
                                                         (const void *) error->_criteria_quark));

          if (error->CanGoTo (floating->_new_pool,
                              _criteria_distribution))
          {
            floating->_new_pool->AddFencer    (error);
            floating->_new_pool->RemoveFencer (floating);

            for (guint size_type = 0; _pool_sizes._available_sizes[size_type] != 0; size_type++)
            {
              for (guint i = 0; i < _nb_pools; i++)
              {
                PoolData *pool_data = &_pool_table[i];

                if (MoveFencerTo (floating,
                                  pool_data,
                                  _pool_sizes._available_sizes[size_type]))
                {
                  goto next_error;
                }
              }
            }
          }
        }

        current_floating = g_list_previous (current_floating);
      }

      g_print ("****> %s\n", error->_player->GetName ());
      temp_list = g_slist_prepend (temp_list,
                                   error);

next_error:
      current_error = g_slist_next (current_error);
    }
  }

  g_slist_free (_remaining_errors);
  _remaining_errors = temp_list;

  g_list_free (_floating_list);
  _floating_list = NULL;
}

// --------------------------------------------------------------------------------
void SmartSwapper::DispatchFencers (GList *list)
{
  while (list)
  {
    Fencer *fencer = (Fencer *) list->data;

    //fencer->Dump (_owner);

    // Try original pool first
    if (fencer->_over_population_error == FALSE)
    {
      for (guint size_type = 0; _pool_sizes._available_sizes[size_type] != 0; size_type++)
      {
        if (MoveFencerTo (fencer,
                          fencer->_original_pool,
                          _pool_sizes._available_sizes[size_type]))
        {
          goto next_fencer;
        }
      }
    }

    for (guint size_type = 0; _pool_sizes._available_sizes[size_type] != 0; size_type++)
    {
      for (guint i = 0; i < _nb_pools; i++)
      {
        PoolData *pool_data = &_pool_table[i];

        if (MoveFencerTo (fencer,
                          pool_data,
                          _pool_sizes._available_sizes[size_type]))
        {
          goto next_fencer;
        }
      }
    }

    // Moving failed
    _remaining_errors = g_slist_prepend (_remaining_errors,
                                         fencer);
    fencer->_over_population_error = FALSE;

next_fencer:
    list = g_list_next (list);
  }
}

// --------------------------------------------------------------------------------
gboolean SmartSwapper::MoveFencerTo (Fencer   *fencer,
                                     PoolData *pool_data,
                                     guint     max_pool_size)
{
  if (pool_data->_size < max_pool_size)
  {
    if (fencer->CanGoTo (pool_data,
                         _criteria_distribution))
    {
      pool_data->AddFencer (fencer);
      //g_print ("          %d (%d)\n", pool_data->_pool->GetNumber (), g_list_length (pool_data->_fencer_list));
      return TRUE;
    }
  }

  return FALSE;
}

// --------------------------------------------------------------------------------
void SmartSwapper::InsertCriteria (GHashTable   *table,
                                   const GQuark  criteria)
{
  CriteriaData *criteria_data;

  if (g_hash_table_lookup_extended (table,
                                    (gconstpointer) criteria,
                                    NULL,
                                    (gpointer *) &criteria_data) == FALSE)
  {
    criteria_data = g_new (CriteriaData, 1);
    criteria_data->_count = 0;

    g_hash_table_insert (table,
                         (gpointer) criteria,
                         criteria_data);
  }
  criteria_data->_count++;
}

// --------------------------------------------------------------------------------
void SmartSwapper::SetExpectedData (GQuark        quark,
                                    CriteriaData *criteria_data,
                                    SmartSwapper *swapper)
{
  criteria_data->_max_criteria_occurrence = criteria_data->_count / swapper->_nb_pools;

  criteria_data->_max_floating_fencers = criteria_data->_count % swapper->_nb_pools;
  if (criteria_data->_max_floating_fencers)
  {
    criteria_data->_max_criteria_occurrence++;
  }

  //g_print ("%d x %20s >> %d (max) %d (floating)\n", criteria_data->_count, g_quark_to_string (quark),
           //criteria_data->_max_criteria_occurrence, criteria_data->_max_floating_fencers);
}

// --------------------------------------------------------------------------------
guint SmartSwapper::GetPoolIndex (guint fencer_index)
{
  if (((fencer_index / _nb_pools) % 2) == 0)
  {
    return (fencer_index % _nb_pools);
  }
  else
  {
    return (_nb_pools-1 - fencer_index % _nb_pools);
  }
}

// --------------------------------------------------------------------------------
gint SmartSwapper::CompareFencerRank (Fencer *a,
                                      Fencer *b)
{
  return a->_rank - b->_rank;
}

// --------------------------------------------------------------------------------
guint SmartSwapper::GetErrors ()
{
  return g_slist_length (_remaining_errors);
}
