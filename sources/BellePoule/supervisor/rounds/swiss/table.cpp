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

#include "quest.hpp"
#include "table.hpp"
#include "bonus.hpp"

namespace Swiss
{
  const gchar *Toto::_class_name     = N_ ("Table");
  const gchar *Toto::_xml_class_name = "TableauSuisse";

  // --------------------------------------------------------------------------------
  Toto::Toto (StageClass *stage_class)
    : Object ("Swiss::Table"),
    Table::Supervisor (stage_class)
  {
  }

  // --------------------------------------------------------------------------------
  Toto::~Toto ()
  {
  }

  // --------------------------------------------------------------------------------
  Generic::Bonus *Toto::GetBonus ()
  {
    return new Bonus (GetDataOwner ());
  }

  // --------------------------------------------------------------------------------
  const gchar *Toto::GetXmlClassName ()
  {
    return _xml_class_name;
  }

  // --------------------------------------------------------------------------------
  void Toto::Declare ()
  {
    RegisterStageClass (gettext (_class_name),
                        _xml_class_name,
                        CreateInstance,
                        EDITABLE|REMOVABLE);
  }

  // --------------------------------------------------------------------------------
  Stage *Toto::CreateInstance (StageClass *stage_class)
  {
    return new Toto (stage_class);
  }
}
