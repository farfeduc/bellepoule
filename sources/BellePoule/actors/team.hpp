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

#pragma once

#include "util/player.hpp"

class Data;

class Team : public Player
{
  public:
    static void RegisterPlayerClass ();

    static Player *CreateInstance ();

    const gchar *GetXmlTag ();

    void AddMember (Player *member);

    virtual void UpdateMembers ();

    void RemoveMember (Player *member);

    GSList *GetMemberList ();

    virtual void SetAttendingFromMembers ();

    virtual void SetAttributesFromMembers ();

    void SetDefaultClassification (Data *default_classification);

    void SetMinimumSize (Data *size);

    void SetManualClassification (Data *manual);

    void EnableMemberSaving (gboolean enable);

  protected:
    Team ();

    virtual ~Team ();

  private:
    static const gchar *_class_name;
    static const gchar *_xml_tag;

    GSList   *_member_list;
    Data     *_default_classification;
    Data     *_minimum_size;
    Data     *_manual_classification;
    gboolean  _enable_member_saving;

    Player *Clone ();

    void Load (xmlNode *xml_node);

    void Save (xmlTextWriter *xml_writer);

    void SaveMembers (xmlTextWriter *xml_writer);
};
