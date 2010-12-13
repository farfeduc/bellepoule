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
#include <goocanvas.h>

#include "player.hpp"
#include "match.hpp"

// --------------------------------------------------------------------------------
Match::Match (Data *max_score)
: Object ("Match")
{
  _max_score = max_score;

  _A = NULL;
  _B = NULL;

  _A_is_known = FALSE;
  _B_is_known = FALSE;

  _A_is_dropped = FALSE;
  _B_is_dropped = FALSE;

  _name = g_strdup ("");

  _A_score = new Score (max_score);
  _B_score = new Score (max_score);
}

// --------------------------------------------------------------------------------
Match::Match  (Player *A,
               Player *B,
               Data   *max_score)
: Object ("Match")
{
  _max_score = max_score;

  _A = A;
  _B = B;

  _A_is_known = TRUE;
  _B_is_known = TRUE;

  _name = g_strdup ("");

  _A_score = new Score (max_score);
  _B_score = new Score (max_score);
}

// --------------------------------------------------------------------------------
Match::~Match ()
{
  Object::TryToRelease (_A_score);
  Object::TryToRelease (_B_score);

  g_free (_name);
}

// --------------------------------------------------------------------------------
gboolean Match::IsDropped ()
{
  return ((_A_is_dropped == TRUE) || (_B_is_dropped == TRUE));
}

// --------------------------------------------------------------------------------
void Match::SetPlayerA (Player *player)
{
  _A          = player;
  _A_is_known = TRUE;
}

// --------------------------------------------------------------------------------
void Match::SetPlayerB (Player *player)
{
  _B          = player;
  _B_is_known = TRUE;
}

// --------------------------------------------------------------------------------
Player *Match::GetPlayerA ()
{
  return _A;
}

// --------------------------------------------------------------------------------
Player *Match::GetPlayerB ()
{
  return _B;
}

// --------------------------------------------------------------------------------
void Match::DropPlayer (Player *player)
{
  if (_A == player)
  {
    _A_is_dropped = TRUE;
  }
  else
  {
    _B_is_dropped = TRUE;
  }

  if (IsDropped ())
  {
    _A_score->Drop ();
    _B_score->Drop ();
  }
}

// --------------------------------------------------------------------------------
void Match::RestorePlayer (Player *player)
{
  if (_A == player)
  {
    _A_is_dropped = FALSE;
  }
  else if (_B == player)
  {
    _B_is_dropped = FALSE;
  }

  if (IsDropped () == FALSE)
  {
    _A_score->Restore ();
    _B_score->Restore ();
  }
}

// --------------------------------------------------------------------------------
Player *Match::GetWinner ()
{
  if (   (_B == NULL)
      && _B_is_known)
  {
    return _A;
  }
  else if (   (_A == NULL)
           && _A_is_known)
  {
    return _B;
  }
  else if (PlayerHasScore (_A) && PlayerHasScore (_B))
  {
    Score *score_A = GetScore (_A);
    Score *score_B = GetScore (_B);

    if (IsDropped ())
    {
      if (_A_is_dropped)
      {
        return _B;
      }
      else
      {
        return _A;
      }
    }
    else if (   score_A->IsValid ()
             && score_B->IsValid ()
             && score_A->IsConsistentWith (score_B))
    {
      if (_A_score->Get () > _B_score->Get ())
      {
        return _A;
      }
      else if (_A_score->Get () < _B_score->Get ())
      {
        return _B;
      }
      else if (_A_score->IsTheBest ())
      {
        return _A;
      }
      else
      {
        return _B;
      }
    }
  }

  return NULL;
}

// --------------------------------------------------------------------------------
gboolean Match::HasPlayer (Player *player)
{
  return ((_A == player) || (_B == player));
}

// --------------------------------------------------------------------------------
gboolean Match::PlayerHasScore (Player *player)
{
  if (_A && (player == _A))
  {
    return (_A_score->IsKnown ());
  }
  else if (_B && (player == _B))
  {
    return (_B_score->IsKnown ());
  }
  else
  {
    return FALSE;
  }
}

// --------------------------------------------------------------------------------
void Match::SetScore (Player   *player,
                      gint      score,
                      gboolean  is_the_best)
{
  if (_A == player)
  {
    _A_score->Set (score, is_the_best);
  }
  else if (_B == player)
  {
    _B_score->Set (score, is_the_best);
  }
}

// --------------------------------------------------------------------------------
gboolean Match::ScoreIsNumber (gchar *score)
{
  guint len = strlen (score);

  if (len == 0)
  {
    return FALSE;
  }

  for (guint i = 0; i < len; i++)
  {
    if (g_ascii_isalpha (score[i]))
    {
      return FALSE;
    }
  }

  return TRUE;
}

// --------------------------------------------------------------------------------
gboolean Match::SetScore (Player *player,
                          gchar  *score)
{
  gboolean result = FALSE;

  if (score)
  {
    if (   (strlen (score) == 1)
        && (g_ascii_toupper (score[0]) == 'V'))
    {
      SetScore (player,
                _max_score->_value,
                TRUE);
      result = TRUE;
    }
    else
    {
      gchar    *score_value = score;
      gboolean  is_the_best = FALSE;

      if (   (strlen (score) > 1)
          && (g_ascii_toupper (score[0]) == 'W'))
      {
        score_value = &score[1];
        is_the_best = TRUE;
      }

      if (ScoreIsNumber (score_value))
      {
        gchar *max_str        = g_strdup_printf ("%d", _max_score->_value);
        gchar *one_digit_more = g_strdup_printf ("%s0", score_value);

        SetScore (player,
                  atoi (score_value),
                  is_the_best);
        if (strlen (score_value) >= strlen (max_str))
        {
          result = TRUE;
        }
        else if ((guint) atoi (one_digit_more) > _max_score->_value)
        {
          result = TRUE;
        }
        else
        {
          result = FALSE;
        }

        g_free (one_digit_more);
        g_free (max_str);
      }
    }
  }

  return result;
}

// --------------------------------------------------------------------------------
Score *Match::GetScore (Player *player)
{
  if (_A == player)
  {
    return _A_score;
  }
  else if (_B == player)
  {
    return _B_score;
  }

  return NULL;
}

// --------------------------------------------------------------------------------
void Match::Save (xmlTextWriter *xml_writer)
{
  if (_A && _B && _number)
  {
    xmlTextWriterStartElement (xml_writer,
                               BAD_CAST "Match");
    xmlTextWriterWriteFormatAttribute (xml_writer,
                                       BAD_CAST "ID",
                                       "%d", _number);

    Save (xml_writer,
          _A);
    Save (xml_writer,
          _B);

    xmlTextWriterEndElement (xml_writer);
  }
}

// --------------------------------------------------------------------------------
void Match::SaveInOrder (xmlTextWriter *xml_writer,
                         Player        *first_player)
{
  if (_A && _B && _number)
  {
    xmlTextWriterStartElement (xml_writer,
                               BAD_CAST "Match");
    xmlTextWriterWriteFormatAttribute (xml_writer,
                                       BAD_CAST "ID",
                                       "%d", _number);

    if (_A == first_player)
    {
      Save (xml_writer,
            _A);
      Save (xml_writer,
            _B);
    }
    else
    {
      Save (xml_writer,
            _B);
      Save (xml_writer,
            _A);
    }

    xmlTextWriterEndElement (xml_writer);
  }
}

// --------------------------------------------------------------------------------
void Match::Save (xmlTextWriter *xml_writer,
                  Player        *player)
{
  if (player)
  {
    Score *score = GetScore (player);

    xmlTextWriterStartElement (xml_writer,
                               BAD_CAST "Tireur");
    xmlTextWriterWriteFormatAttribute (xml_writer,
                                       BAD_CAST "REF",
                                       "%d", player->GetRef ());
    if (score->IsKnown ())
    {
      xmlTextWriterWriteFormatAttribute (xml_writer,
                                         BAD_CAST "Score",
                                         "%d", score->Get ());
      if (GetWinner () == player)
      {
        xmlTextWriterWriteAttribute (xml_writer,
                                     BAD_CAST "Statut",
                                     BAD_CAST "V");
      }
      else
      {
        xmlTextWriterWriteAttribute (xml_writer,
                                     BAD_CAST "Statut",
                                     BAD_CAST "D");
      }
    }
    xmlTextWriterEndElement (xml_writer);
  }
}

// --------------------------------------------------------------------------------
void Match::CleanScore ()
{
  _A_is_dropped = FALSE;
  _B_is_dropped = FALSE;
  _A_score->Clean ();
  _B_score->Clean ();
}

// --------------------------------------------------------------------------------
void Match::SetNumber (gint number)
{
  g_free (_name);
  _name = g_strdup_printf ("M%d", number);

  _number = number;
}

// --------------------------------------------------------------------------------
gchar *Match::GetName ()
{
  return _name;
}

// --------------------------------------------------------------------------------
gint Match::GetNumber ()
{
  return _number;
}
