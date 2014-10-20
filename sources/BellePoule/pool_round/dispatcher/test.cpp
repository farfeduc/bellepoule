#include <stdlib.h>
#include "dispatcher.hpp"

void TestDispatcher (guint size)
{
  Pool::Dispatcher **fie = g_new0 (Pool::Dispatcher *, 18);

  Pool::Dispatcher *old_fie = new Pool::Dispatcher (6,
                                                    "old 6",
                                                    1, 2,
                                                    4, 5,
                                                    2, 3,
                                                    5, 6,
                                                    3, 1,
                                                    6, 4,
                                                    2, 5,
                                                    1, 4,
                                                    5, 3,
                                                    1, 6,
                                                    4, 2,
                                                    3, 6,
                                                    5, 1,
                                                    3, 4,
                                                    6, 2);
  fie[6] = new Pool::Dispatcher (6,
                                 "new 6",
                                 1, 2,
                                 4, 3,
                                 6, 5,
                                 3, 1,
                                 2, 6,
                                 5, 4,
                                 1, 6,
                                 3, 5,
                                 4, 2,
                                 5, 1,
                                 6, 4,
                                 2, 3,
                                 1, 4,
                                 5, 2,
                                 3, 6);
  fie[7] = new Pool::Dispatcher (7,
                                 "FIE",
                                 1, 4,
                                 2, 5,
                                 3, 6,
                                 7, 1,
                                 5, 4,
                                 2, 3,
                                 6, 7,
                                 5, 1,
                                 4, 3,
                                 6, 2,
                                 5, 7,
                                 3, 1,
                                 4, 6,
                                 7, 2,
                                 3, 5,
                                 1, 6,
                                 2, 4,
                                 7, 3,
                                 6, 5,
                                 1, 2,
                                 4, 7);
  fie[8] = new Pool::Dispatcher (8,
                                 "FIE",
                                 2, 3,
                                 1, 5,
                                 7, 4,
                                 6, 8,
                                 1, 2,
                                 3, 4,
                                 5, 6,
                                 8, 7,
                                 4, 1,
                                 5, 2,
                                 8, 3,
                                 6, 7,
                                 4, 2,
                                 8, 1,
                                 7, 5,
                                 3, 6,
                                 2, 8,
                                 5, 4,
                                 6, 1,
                                 3, 7,
                                 4, 8,
                                 2, 6,
                                 3, 5,
                                 1, 7,
                                 4, 6,
                                 8, 5,
                                 7, 2,
                                 1, 3);

  fie[9] = new Pool::Dispatcher (9,
                                 "FIE",
                                 1, 9,
                                 2, 8,
                                 3, 7,
                                 4, 6,
                                 1, 5,
                                 2, 9,
                                 8, 3,
                                 7, 4,
                                 6, 5,
                                 1, 2,
                                 9, 3,
                                 8, 4,
                                 7, 2,
                                 6, 1,
                                 3, 2,
                                 9, 4,
                                 5, 8,
                                 7, 6,
                                 3, 1,
                                 2, 4,
                                 5, 9,
                                 8, 6,
                                 7, 1,
                                 4, 3,
                                 5, 2,
                                 6, 9,
                                 8, 7,
                                 4, 1,
                                 5, 3,
                                 6, 2,
                                 9, 7,
                                 1, 8,
                                 4, 5,
                                 3, 6,
                                 5, 7,
                                 9, 8);

  fie[10] = new Pool::Dispatcher (10,
                                  "FIE",
                                  1, 4,
                                  6, 9,
                                  2, 5,
                                  7, 10,
                                  3, 1,
                                  8, 6,
                                  4, 5,
                                  9, 10,
                                  2, 3,
                                  7, 8,
                                  5, 1,
                                  10, 6,
                                  4, 2,
                                  9, 7,
                                  5, 3,
                                  10, 8,
                                  1, 2,
                                  6, 7,
                                  3, 4,
                                  8, 9,
                                  5, 10,
                                  1, 6,
                                  2, 7,
                                  3, 8,
                                  4, 9,
                                  6, 5,
                                  10, 2,
                                  8, 1,
                                  7, 4,
                                  9, 3,
                                  2, 6,
                                  5, 8,
                                  4, 10,
                                  1, 9,
                                  3, 7,
                                  8, 2,
                                  6, 4,
                                  9, 5,
                                  10, 3,
                                  7, 1,
                                  4, 8,
                                  2, 9,
                                  3, 6,
                                  5, 7,
                                  1, 10);

  fie[11] = new Pool::Dispatcher (11,
                                  "FIE",
                                  1, 2,
                                  7, 8,
                                  4, 5,
                                  10, 11,
                                  2, 3,
                                  8, 9,
                                  5, 6,
                                  3, 1,
                                  9, 7,
                                  6, 4,
                                  2, 5,
                                  8, 11,
                                  1, 4,
                                  7, 10,
                                  5, 3,
                                  11, 9,
                                  1, 6,
                                  4, 2,
                                  10, 8,
                                  3, 6,
                                  5, 1,
                                  11, 7,
                                  3, 4,
                                  9, 10,
                                  6, 2,
                                  1, 7,
                                  3, 9,
                                  10, 4,
                                  8, 2,
                                  5, 11,
                                  1, 8,
                                  9, 2,
                                  3, 10,
                                  4, 11,
                                  6, 7,
                                  9, 1,
                                  2, 10,
                                  11, 3,
                                  7, 5,
                                  6, 8,
                                  10, 1,
                                  11, 2,
                                  4, 7,
                                  8, 5,
                                  6, 9,
                                  11, 1,
                                  7, 3,
                                  4, 8,
                                  9, 5,
                                  6, 10,
                                  2, 7,
                                  8, 3,
                                  4, 9,
                                  10, 5,
                                  6, 11);

  fie[12] = new Pool::Dispatcher (12,
                                  "FIE",
                                  1, 2,
                                  3, 4,
                                  5, 6,
                                  7, 8,
                                  9, 10,
                                  11, 12,
                                  3, 1,
                                  2, 4,
                                  7, 5,
                                  6, 8,
                                  11, 9,
                                  10, 12,
                                  4, 1,
                                  2, 3,
                                  8, 5,
                                  6, 7,
                                  12, 9,
                                  10, 11,
                                  1, 5,
                                  4, 8,
                                  6, 2,
                                  7, 3,
                                  9, 1,
                                  12, 5,
                                  4, 10,
                                  8, 11,
                                  2, 7,
                                  3, 6,
                                  5, 9,
                                  1, 12,
                                  8, 10,
                                  11, 4,
                                  5, 2,
                                  9, 7,
                                  12, 3,
                                  1, 6,
                                  10, 2,
                                  5, 11,
                                  8, 9,
                                  4, 7,
                                  3, 10,
                                  12, 6,
                                  11, 1,
                                  2, 8,
                                  9, 4,
                                  7, 10,
                                  5, 3,
                                  6, 11,
                                  2, 12,
                                  1, 8,
                                  4, 5,
                                  9, 3,
                                  7, 11,
                                  10, 6,
                                  8, 12,
                                  9, 2,
                                  7, 1,
                                  6, 4,
                                  3, 11,
                                  10, 5,
                                  12, 7,
                                  6, 9,
                                  8, 3,
                                  1, 10,
                                  4, 12,
                                  11, 2);

  fie[13] = new Pool::Dispatcher (13,
                                  "FIE",
                                  1, 2,
                                  3, 4,
                                  5, 6,
                                  7, 8,
                                  9, 10,
                                  11, 12,
                                  13, 1,
                                  2, 3,
                                  4, 5,
                                  6, 7,
                                  8, 9,
                                  10, 11,
                                  12, 1,
                                  2, 13,
                                  3, 5,
                                  4, 6,
                                  7, 9,
                                  8, 10,
                                  1, 11,
                                  12, 2,
                                  13, 3,
                                  5, 7,
                                  9, 4,
                                  6, 8,
                                  10, 1,
                                  11, 2,
                                  3, 12,
                                  5, 13,
                                  7, 4,
                                  9, 6,
                                  1, 8,
                                  2, 10,
                                  11, 3,
                                  12, 5,
                                  4, 13,
                                  1, 7,
                                  6, 2,
                                  3, 9,
                                  8, 11,
                                  10, 5,
                                  4, 12,
                                  13, 7,
                                  6, 1,
                                  2, 9,
                                  8, 3,
                                  5, 11,
                                  10, 4,
                                  7, 12,
                                  13, 6,
                                  9, 1,
                                  2, 8,
                                  11, 4,
                                  3, 10,
                                  12, 6,
                                  9, 13,
                                  1, 5,
                                  7, 2,
                                  4, 8,
                                  6, 11,
                                  10, 12,
                                  1, 3,
                                  8, 13,
                                  5, 9,
                                  11, 7,
                                  6, 10,
                                  12, 8,
                                  2, 4,
                                  13, 11,
                                  3, 7,
                                  8, 5,
                                  9, 12,
                                  10, 13,
                                  4, 1,
                                  3, 6,
                                  5, 2,
                                  11, 9,
                                  7, 10,
                                  12, 13);

  fie[14] = new Pool::Dispatcher (14,
                                  "FIE",
                                  1, 2,
                                  3, 4,
                                  5, 6,
                                  7, 8,
                                  9, 10,
                                  11, 12,
                                  13, 14,
                                  3, 1,
                                  2, 4,
                                  7, 5,
                                  6, 8,
                                  11, 9,
                                  10, 12,
                                  1, 13,
                                  14, 3,
                                  2, 5,
                                  4, 7,
                                  6, 9,
                                  8, 11,
                                  10, 1,
                                  12, 13,
                                  3, 2,
                                  5, 14,
                                  4, 6,
                                  9, 7,
                                  8, 1,
                                  11, 10,
                                  12, 2,
                                  13, 3,
                                  4, 5,
                                  14, 6,
                                  1, 7,
                                  8, 9,
                                  2, 10,
                                  11, 3,
                                  12, 4,
                                  5, 13,
                                  6, 1,
                                  7, 14,
                                  2, 8,
                                  9, 3,
                                  10, 4,
                                  12, 14,
                                  5, 11,
                                  13, 6,
                                  7, 2,
                                  1, 12,
                                  14, 8,
                                  3, 10,
                                  4, 9,
                                  1, 5,
                                  6, 11,
                                  12, 7,
                                  2, 13,
                                  8, 3,
                                  10, 14,
                                  4, 1,
                                  9, 5,
                                  6, 7,
                                  11, 2,
                                  3, 12,
                                  13, 8,
                                  14, 1,
                                  5, 10,
                                  11, 4,
                                  2, 9,
                                  3, 6,
                                  7, 13,
                                  8, 12,
                                  1, 11,
                                  4, 14,
                                  5, 3,
                                  10, 6,
                                  9, 13,
                                  14, 2,
                                  7, 11,
                                  8, 4,
                                  12, 5,
                                  1, 9,
                                  13, 10,
                                  6, 2,
                                  3, 7,
                                  9, 12,
                                  14, 11,
                                  13, 4,
                                  5, 8,
                                  10, 7,
                                  12, 6,
                                  11, 13,
                                  9, 14,
                                  8, 10);

  fie[15] = new Pool::Dispatcher (15,
                                  "FIE",
                                  1, 2,
                                  3, 4,
                                  5, 6,
                                  7, 8,
                                  9, 10,
                                  11, 12,
                                  13, 14,
                                  15, 1,
                                  2, 3,
                                  4, 5,
                                  6, 7,
                                  8, 9,
                                  10, 11,
                                  12, 13,
                                  14, 1,
                                  2, 15,
                                  3, 5,
                                  4, 6,
                                  7, 9,
                                  8, 10,
                                  11, 13,
                                  1, 12,
                                  14, 2,
                                  15, 3,
                                  5, 7,
                                  9, 4,
                                  6, 8,
                                  10, 13,
                                  1, 11,
                                  12, 2,
                                  3, 14,
                                  5, 15,
                                  4, 7,
                                  6, 9,
                                  13, 8,
                                  10, 1,
                                  2, 11,
                                  12, 3,
                                  14, 5,
                                  15, 4,
                                  7, 13,
                                  1, 6,
                                  9, 2,
                                  8, 11,
                                  3, 10,
                                  5, 12,
                                  4, 14,
                                  7, 15,
                                  13, 1,
                                  2, 6,
                                  11, 9,
                                  8, 3,
                                  10, 5,
                                  12, 4,
                                  14, 7,
                                  13, 15,
                                  9, 1,
                                  6, 11,
                                  2, 8,
                                  3, 7,
                                  4, 10,
                                  13, 5,
                                  14, 12,
                                  15, 9,
                                  1, 8,
                                  6, 3,
                                  7, 11,
                                  4, 2,
                                  10, 12,
                                  9, 5,
                                  15, 14,
                                  13, 3,
                                  1, 7,
                                  8, 4,
                                  10, 6,
                                  11, 5,
                                  12, 9,
                                  2, 13,
                                  3, 1,
                                  8, 14,
                                  6, 15,
                                  7, 10,
                                  11, 4,
                                  5, 2,
                                  9, 13,
                                  12, 8,
                                  14, 6,
                                  1, 4,
                                  11, 3,
                                  15, 10,
                                  2, 7,
                                  5, 8,
                                  13, 6,
                                  9, 14,
                                  12, 15,
                                  5, 1,
                                  10, 2,
                                  11, 14,
                                  7, 12,
                                  4, 13,
                                  8, 15,
                                  3, 9,
                                  6, 12,
                                  14, 10,
                                  15, 11);

  fie[16] = new Pool::Dispatcher (16,
                                  "FIE",
                                  1, 2,
                                  3, 4,
                                  5, 6,
                                  7, 8,
                                  9, 10,
                                  11, 12,
                                  13, 14,
                                  15, 16,
                                  3, 1,
                                  2, 4,
                                  7, 5,
                                  6, 8,
                                  11, 9,
                                  10, 12,
                                  15, 13,
                                  14, 16,
                                  4, 1,
                                  2, 3,
                                  8, 5,
                                  6, 7,
                                  12, 9,
                                  10, 11,
                                  16, 13,
                                  14, 15,
                                  1, 5,
                                  4, 8,
                                  6, 2,
                                  7, 3,
                                  9, 13,
                                  12, 16,
                                  14, 10,
                                  15, 11,
                                  8, 1,
                                  5, 4,
                                  2, 7,
                                  3, 6,
                                  16, 9,
                                  13, 12,
                                  10, 15,
                                  11, 14,
                                  1, 7,
                                  8, 2,
                                  5, 3,
                                  4, 6,
                                  9, 15,
                                  16, 10,
                                  12, 14,
                                  13, 11,
                                  1, 6,
                                  2, 5,
                                  3, 8,
                                  4, 7,
                                  9, 14,
                                  10, 13,
                                  11, 16,
                                  12, 15,
                                  9, 1,
                                  6, 14,
                                  10, 2,
                                  5, 13,
                                  11, 3,
                                  8, 16,
                                  12, 4,
                                  7, 15,
                                  14, 1,
                                  6, 9,
                                  13, 2,
                                  5, 10,
                                  16, 3,
                                  8, 11,
                                  15, 4,
                                  7, 12,
                                  1, 13,
                                  2, 14,
                                  9, 5,
                                  10, 6,
                                  3, 15,
                                  4, 16,
                                  12, 8,
                                  11, 7,
                                  1, 10,
                                  2, 9,
                                  14, 5,
                                  3, 12,
                                  13, 6,
                                  4, 11,
                                  16, 7,
                                  15, 8,
                                  12, 1,
                                  10, 3,
                                  11, 2,
                                  9, 4,
                                  5, 16,
                                  7, 14,
                                  6, 15,
                                  8, 13,
                                  1, 11,
                                  2, 12,
                                  3, 9,
                                  4, 10,
                                  15, 5,
                                  16, 6,
                                  13, 7,
                                  14, 8,
                                  15, 1,
                                  5, 11,
                                  16, 2,
                                  6, 12,
                                  13, 3,
                                  7, 9,
                                  14, 4,
                                  8, 10,
                                  1, 16,
                                  2, 15,
                                  5, 12,
                                  6, 11,
                                  3, 14,
                                  4, 13,
                                  7, 10,
                                  8, 9);

  fie[17] = new Pool::Dispatcher (17,
                                  "FIE",
                                  1, 2,
                                  3, 4,
                                  5, 6,
                                  7, 8,
                                  9, 10,
                                  11, 12,
                                  13, 14,
                                  15, 16,
                                  17, 1,
                                  2, 3,
                                  4, 5,
                                  6, 7,
                                  8, 9,
                                  10, 11,
                                  12, 13,
                                  14, 15,
                                  16, 1,
                                  2, 17,
                                  3, 5,
                                  4, 6,
                                  7, 9,
                                  8, 10,
                                  11, 13,
                                  12, 14,
                                  1, 15,
                                  16, 2,
                                  17, 3,
                                  5, 7,
                                  9, 4,
                                  6, 8,
                                  10, 13,
                                  14, 11,
                                  1, 12,
                                  15, 2,
                                  3, 16,
                                  5, 17,
                                  4, 7,
                                  6, 9,
                                  13, 8,
                                  10, 14,
                                  11, 1,
                                  2, 12,
                                  15, 3,
                                  16, 5,
                                  17, 4,
                                  7, 13,
                                  10, 6,
                                  9, 14,
                                  8, 1,
                                  2, 11,
                                  12, 3,
                                  5, 15,
                                  4, 16,
                                  7, 17,
                                  13, 6,
                                  1, 10,
                                  14, 8,
                                  9, 2,
                                  3, 11,
                                  12, 5,
                                  15, 4,
                                  16, 7,
                                  6, 17,
                                  13, 1,
                                  2, 10,
                                  8, 3,
                                  14, 5,
                                  11, 9,
                                  4, 12,
                                  7, 15,
                                  6, 16,
                                  17, 13,
                                  1, 3,
                                  8, 2,
                                  5, 10,
                                  14, 4,
                                  16, 17,
                                  9, 12,
                                  11, 7,
                                  17, 8,
                                  15, 6,
                                  13, 16,
                                  1, 5,
                                  3, 10,
                                  2, 4,
                                  7, 14,
                                  12, 6,
                                  15, 9,
                                  16, 11,
                                  5, 13,
                                  10, 17,
                                  4, 8,
                                  7, 1,
                                  3, 14,
                                  6, 2,
                                  12, 15,
                                  9, 16,
                                  5, 11,
                                  13, 4,
                                  10, 7,
                                  14, 17,
                                  1, 6,
                                  8, 12,
                                  3, 9,
                                  2, 5,
                                  11, 15,
                                  16, 10,
                                  4, 1,
                                  13, 3,
                                  6, 14,
                                  12, 7,
                                  17, 9,
                                  5, 8,
                                  10, 15,
                                  13, 2,
                                  4, 11,
                                  3, 6,
                                  14, 16,
                                  9, 1,
                                  17, 12,
                                  2, 7,
                                  6, 11,
                                  8, 15,
                                  10, 4,
                                  9, 5,
                                  16, 12,
                                  1, 14,
                                  15, 13,
                                  11, 17,
                                  7, 3,
                                  8, 16,
                                  14, 2,
                                  12, 10,
                                  15, 17,
                                  13, 9,
                                  11, 8);

  if (size)
  {
    if (size == 6)
    {
      old_fie->Dump ();
    }

    if (fie[size])
    {
      Pool::Dispatcher *sequence = fie[size];

      sequence->Dump ();
    }
  }
}