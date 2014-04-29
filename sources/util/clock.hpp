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

#ifndef clock_hpp
#define clock_hpp

#include "object.hpp"

class Clock : public Object
{
  public:
    Clock ();

    void PrintElapsed (const gchar *tag);

  private:
    GTimeVal _last_timeval;

    virtual ~Clock ();

    void GetTimevalDiff (GTimeVal *result,
                         GTimeVal *x,
                         GTimeVal *y);
};

#endif