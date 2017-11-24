"    <style type=\"text/css\">\n"
"      body\n"
"      {\n"
"        background:  #cccccc;\n"
"        color:       #117827;\n"
"        font-family: Geneva, Arial, Helvetica, sans-serif;\n"
"      }\n"
"      .Title h1\n"
"      {\n"
"        margin:       5;\n"
"      }\n"
"      .Round\n"
"      {\n"
"        display:     none;\n"
"      }\n"
"      .List\n"
"      {\n"
"        border-collapse:  collapse;\n"
"        white-space:      nowrap;\n"
"      }\n"
"      .List th\n"
"      {\n"
"        padding:    5px;\n"
"        text-align: left;\n"
"      }\n"
"      .List td\n"
"      {\n"
"        padding:    5px;\n"
"      }\n"
"      .ListHeader\n"
"      {\n"
"        background:  #117827;\n"
"        color:       #dddddd;\n"
"        font-weight: bold;\n"
"      }\n"
"      .EvenRow\n"
"      {\n"
"        background:  #f3f3f3;\n"
"        color:       Black;\n"
"      }\n"
"      .OddRow\n"
"      {\n"
"        background:  #dddddd;\n"
"        color:       Black;\n"
"      }\n"
"      .Separator\n"
"      {\n"
"        outline: 1px solid #117827;\n"
"      }\n"
"      .name\n"
"      {\n"
"        color:       Black;\n"
"        padding:     5px;\n"
"      }\n"
"      .first_name\n"
"      {\n"
"        color:       Black;\n"
"        font-weight: normal;\n"
"        font-style:  italic;\n"
"      }\n"
"      .country\n"
"      {\n"
"        color:       Black;\n"
"        font-weight: normal;\n"
"        padding:     5px;\n"
"      }\n"
"      .PoolName\n"
"      {\n"
"        background:  #117827;\n"
"        color:       #dddddd;\n"
"        font-weight: bold;\n"
"        padding:    5px;\n"
"      }\n"
"      .NoScoreCell\n"
"      {\n"
"        background: #117827;\n"
"        width:      25px;\n"
"        padding:    5px;\n"
"      }\n"
"      .PoolCell\n"
"      {\n"
"        text-align: center;\n"
"        width:      25px;\n"
"        border:     1px solid #117827;\n"
"        padding:    5px;\n"
"      }\n"
"      .DashBoardCell\n"
"      {\n"
"        text-align:     right;\n"
"        width:          25px;\n"
"        padding-left:   5px;\n"
"        padding-right:  5px;\n"
"        padding:    5px;\n"
"      }\n"
"      .GridSeparator\n"
"      {\n"
"        width:      20px;\n"
"        background: #cccccc;\n"
"        color:      #117827;\n"
"        text-align: right;\n"
"        padding:    5px;\n"
"        font-weight: bold;\n"
"      }\n"
"      .PoolSeparator\n"
"      {\n"
"        height:     35px;\n"
"      }\n"
"      .TableTable\n"
"      {\n"
"        border-collapse:  collapse;\n"
"      }\n"
"      .TableName\n"
"      {\n"
"        background:  #117827;\n"
"        color:       White;\n"
"        font-weight: bold;\n"
"      }\n"
"      .TableCell\n"
"      {\n"
"        background:   #dddddd;\n"
"        color:        Black;\n"
"        font-weight:  bold;\n"
"        white-space:  nowrap;\n"
"        border-left:  2px solid #117827;\n"
"        border-right: 2px solid #117827;\n"
"        padding:      5px;\n"
"      }\n"
"      .TableCellFirstCol\n"
"      {\n"
"        background:   #dddddd;\n"
"        color:        Black;\n"
"        font-weight:  bold;\n"
"        white-space:  nowrap;\n"
"        border-right: 2px solid #117827;\n"
"        padding:      5px;\n"
"      }\n"
"      .TableCellLastCol\n"
"      {\n"
"        background:   #dddddd;\n"
"        color:        Black;\n"
"        font-weight:  bold;\n"
"        border-left:  2px solid #117827;\n"
"        padding:      5px;\n"
"      }\n"
"      .TableConnector\n"
"      {\n"
"        background:  #cccccc;\n"
"        color:       #cccccc;\n"
"        white-space: nowrap;\n"
"        border-left: 2px solid #117827;\n"
"      }\n"
"      .EmptyTableCell\n"
"      {\n"
"        background:  #cccccc;\n"
"      }\n"
"      .TableScore\n"
"      {\n"
"        color:        #117827;\n"
"        margin-right: 5px;\n"
"      }\n"
"      .Footer\n"
"      {\n"
"        color:       White;\n"
"      }\n"
"      #menu_bar {\n"
"        list-style-type: none;\n"
"        padding-bottom:  28px;\n"
"        border-bottom:   1px solid #9EA0A1;\n"
"        margin-left:     0;\n"
"      }\n"
"      #menu_bar li {\n"
"        float:       left;\n"
"        height:      25px;\n"
"        margin:      2px 2px 0 4px;\n"
"        background:  #cccccc;\n"
"        border:      1px solid #9EA0A1;\n"
"      }\n"
"      #menu_bar li.active {\n"
"      }\n"
"      #menu_bar a {\n"
"        display:         block;\n"
"        text-decoration: none;\n"
"        padding:         4px;\n"
"      }\n"
"      #menu_bar a:hover {\n"
"        background:  #117827;\n"
"        color:       #dddddd;\n"
"      }\n"
"      A:link    {text-decoration: underline; color: #117827}\n"
"      A:visited {text-decoration: underline; color: #117827}\n"
"    </style>\n"
"\n"
"    <script language=\"JavaScript\">\n"
"      var current_selection;\n"
"\n"
"      // ----------------------------------------------\n"
"      function OnTabClicked (tab_id)\n"
"      {\n"
"        var element = document.getElementById (tab_id);\n"
"\n"
"        if (current_selection == null)\n"
"        {\n"
"          current_selection = document.getElementById (\'round_0\');\n"
"        }\n"
"        current_selection.style.display = \"none\";\n"
"\n"
"        current_selection = element;\n"
"        current_selection.style.display = \"inline\";\n"
"\n"
"        document.cookie = 'current_tab=' + tab_id;\n"
"      }\n"
"\n"
"      // ----------------------------------------------\n"
"      function OnLoad ()\n"
"      {\n"
"        window.OnTabClicked (ReadCookie ('current_tab'));\n"
"\n"
"        setInterval (function(){location.reload ();}, 60*1000);\n"
"      }\n"
"\n"
"      // ----------------------------------------------\n"
"      function ReadCookie (name)\n"
"      {\n"
"        return (document.cookie.match ('(^|; )' + name + '=([^;]*)')||0)[2];\n"
"      }\n"
"    </script>\n"
