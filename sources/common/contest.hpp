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

#ifndef contest_hpp
#define contest_hpp

#include <libxml/xmlwriter.h>
#include <gtk/gtk.h>

#include "util/data.hpp"
#include "util/module.hpp"
#include "util/glade.hpp"
#include "network/downloader.hpp"

#include "schedule.hpp"

namespace People
{
  class Checkin;
}

class Tournament;

class Contest : public Module
{
  public:
     Contest ();

    virtual ~Contest ();

    static void Init ();

    static void Cleanup ();

    static Contest *Create ();

    Contest *Duplicate ();

    void LoadXml (const gchar *filename);

    void LoadFencerFile (const gchar *filename);

    void LoadRemote (const gchar *address);

    void LoadMemory (const gchar *memory);

    void LatchPlayerList ();

    void AttachTo (GtkNotebook *to);

    void Save ();

    void SaveHeader (xmlTextWriter *xml_writer);

    void Publish ();

    gchar *GetFilename ();

    void AddFencer (Player *player,
                    guint   rank);

    void AddReferee (Player *referee);

    void ImportReferees (GSList *imported_list);

    Player *Share (Player *referee);

    Player *GetRefereeFromRef (guint ref);

    State GetState ();

    gboolean OnHttpPost (const gchar **url,
                         const gchar *data);

    gchar *GetId ();
    gchar *GetOrganizer ();
    gchar *GetDate ();
    gchar *GetWeapon ();
    gchar  GetWeaponCode ();
    gchar *GetName ();
    gchar *GetDefaultFileName ();
    gchar *GetGender ();
    gchar *GetCategory ();
    gboolean IsTeamEvent ();

  public:
    void ReadTeamProperty                 ();
    void on_save_toolbutton_clicked       ();
    void on_properties_toolbutton_clicked ();
    void on_contest_close_button_clicked  ();
    void on_calendar_button_clicked       ();
    void on_web_site_button_clicked       ();
    void on_ftp_changed                   (GtkComboBox *widget);
    void on_referees_toolbutton_toggled   (GtkToggleToolButton *w);

  private:
    struct Time : public Object
    {
      Time (const gchar *name);

      virtual ~Time ();

      void Load (gchar *attr);
      void Save (xmlTextWriter *xml_writer,
                 const gchar   *attr_name);
      void ReadProperties (Glade *glade);
      void HideProperties (Glade *glade);
      void FillInProperties (Glade *glade);
      void Copy (Time *to);
      gboolean IsEqualTo (Time *to);

      gchar *_name;
      guint  _hour;
      guint  _minute;
    };

    static const guint _nb_weapon = 4;
    static const gchar *weapon_image[_nb_weapon];
    static const gchar *weapon_xml_image[_nb_weapon];

    static const guint _nb_gender = 3;
    static const gchar *gender_image[_nb_gender];
    static const gchar *gender_xml_image[_nb_gender];

    static const guint _nb_category = 12;
    static const gchar *category_image[_nb_category];
    static const gchar *category_xml_image[_nb_category];
    static const gchar *category_xml_alias[_nb_category];

    static GList *_color_list;

    gchar           *_level;
    gchar           *_id;
    gchar           *_name;
    gchar           *_organizer;
    gchar           *_location;
    gchar           *_web_site;
    guint            _category;
    gchar           *_filename;
    guint            _weapon;
    guint            _gender;
    guint            _day;
    guint            _month;
    guint            _year;
    gboolean         _team_event;
    Data            *_manual_classification;
    Data            *_default_classification;
    Data            *_minimum_team_size;
    Time            *_checkin_time;
    Time            *_scratch_time;
    Time            *_start_time;
    Schedule        *_schedule;
    Tournament      *_tournament;
    gboolean         _derived;
    GdkColor        *_gdk_color;
    guint            _save_timeout_id;
    People::Checkin *_referees_list;
    gint             _referee_pane_position;
    GHashTable      *_ref_translation_table;
    State            _state;
    gboolean         _read_only;
    Net::Downloader *_downloader;

    GtkWidget   *_properties_dialog;
    GtkWidget   *_weapon_combo;
    GtkWidget   *_gender_combo;
    GtkWidget   *_category_combo;
    GtkNotebook *_notebook;

    void   ReadProperties       ();
    void   DisplayProperties    ();
    void   LoadXmlDoc           (xmlDoc *doc);
    void   GetUnknownAttributes (const gchar     *contest_keyword,
                                 xmlXPathContext *xml_context);
    void   LoadHeader           (xmlXPathContext *context);
    gchar *GetSaveFileName      (gchar       *title,
                                 const gchar *config_key);
    void   Save                 (gchar *filename);
    void   FillInProperties     ();
    void   FillInDate           (guint day,
                                 guint month,
                                 guint year);

    void OnDrawPage (GtkPrintOperation *operation,
                     GtkPrintContext   *context,
                     gint               page_nr);

    void DrawPage (GtkPrintOperation *operation,
                   GtkPrintContext   *context,
                   gint               page_nr);

    void ChooseColor ();

    void MakeDirty ();

    void OpenMemoryContest (Net::Downloader::CallbackData *cbk_data);

    static gboolean OnSaveTimeout (Contest *contest);

    static gboolean OnCompetitionReceived (Net::Downloader::CallbackData *cbk_data);

    void OnPlugged ();

  private:
    void OnBeginPrint (GtkPrintOperation *operation,
                       GtkPrintContext   *context);
    void OnEndPrint (GtkPrintOperation *operation,
                     GtkPrintContext   *context);
};

#endif
