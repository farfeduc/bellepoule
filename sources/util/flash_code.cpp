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

#include <qrencode.h>

#include "flash_code.hpp"

// --------------------------------------------------------------------------------
FlashCode::FlashCode (const gchar *text)
  : Object ("FlashCode")
{
  _text = g_strdup_printf ("%s", text);
}

// --------------------------------------------------------------------------------
FlashCode::~FlashCode ()
{
  g_free (_text);
}

// --------------------------------------------------------------------------------
void FlashCode::DestroyPixbuf (guchar   *pixels,
                               gpointer  data)
{
  g_free (pixels);
}

// --------------------------------------------------------------------------------
gchar *FlashCode::GetText ()
{
  return g_strdup (_text);
}

// --------------------------------------------------------------------------------
GdkPixbuf *FlashCode::GetPixbuf (guint pixel_size)
{
  GdkPixbuf *pixbuf = NULL;

  {
    QRcode *qr_code;

    {
      gchar  *text = GetText ();

      qr_code = QRcode_encodeString (text,
                                     0,
                                     QR_ECLEVEL_L,
                                     QR_MODE_8,
                                     1);
      g_free (text);
    }

    {
      GdkPixbuf *small_pixbuf;
      guint      stride = qr_code->width * 3;
      guchar    *data   = g_new (guchar, qr_code->width * stride);

      for (gint y = 0; y < qr_code->width; y++)
      {
        guchar *row = qr_code->data + (y * qr_code->width);
        guchar *p   = data + (y * stride);

        for (gint x = 0; x < qr_code->width; x++)
        {
          if (row[x] & 0x1)
          {
            p[0] = 0x0;
            p[1] = 0x0;
            p[2] = 0x0;
          }
          else
          {
            p[0] = 0xFF;
            p[1] = 0xFF;
            p[2] = 0xFF;
          }
          p += 3;
        }
      }

      small_pixbuf = gdk_pixbuf_new_from_data (data,
                                               GDK_COLORSPACE_RGB,
                                               FALSE,
                                               8,
                                               qr_code->width,
                                               qr_code->width,
                                               stride,
                                               DestroyPixbuf,
                                               NULL);

      pixbuf = gdk_pixbuf_scale_simple (small_pixbuf,
                                         qr_code->width * pixel_size,
                                         qr_code->width * pixel_size,
                                         GDK_INTERP_NEAREST);
      g_object_unref (small_pixbuf);
    }
  }

  return pixbuf;
}