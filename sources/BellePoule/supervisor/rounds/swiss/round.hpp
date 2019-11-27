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

#include <gtk/gtk.h>

#include "../../stage.hpp"

class Data;

namespace Swiss
{
  class Round : public Stage,
                public Module
  {
    public:
      static void Declare ();

      Round (StageClass *stage_class);

    public:
      static const gchar *_class_name;
      static const gchar *_xml_class_name;

    private:
      Data *_matches_per_fencer;

      ~Round () override;

      void Load (xmlNode *xml_node) override;

      void SaveConfiguration (XmlScheme *xml_scheme) override;

      void LoadConfiguration (xmlNode *xml_node) override;

      void FillInConfig () override;

      void ApplyConfig () override;

      static Stage *CreateInstance (StageClass *stage_class);
  };
}
