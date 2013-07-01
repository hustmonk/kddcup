/**
 *  =============================================================
 *
 *  	  Filename:  CSV.h
 *   Description:  my private csv parser
 *       Created:  05/06/2013 10:47:06 AM
 *       Version:  
 *      Revision:  #1;#4
 *        Author:  liujingming(liujingminghust@163.com)
 *
 *  =============================================================
 **/

#ifndef C_Author_Disambiguation_CSV
#define C_Author_Disambiguation_CSV
#include <string.h>
#include <assert.h>
#include <string>
using namespace std;

class CSV {
  public:
    static bool parser(const string *line, char **rows,
        int max_row_count, int &row_count) {
      return parser(line->c_str(), rows, max_row_count, row_count);
    }

    static bool parser(const char *line, char **rows,
         int max_row_count, int &row_count) {
      return parser(line, rows, max_row_count, row_count, true);
    }

    static bool parser(const char *line, char **rows,
        int max_row_count, int &row_count, bool is_to_lower) {
      assert(line != NULL);
      int len = strlen(line);
      int start = 0;
      int end = 0;
      row_count = 0;
      while (start < len && row_count < max_row_count) {
        if (line[start] == ',') {
          rows[row_count][0] = '\0';
          ++row_count;
          ++start;
          continue;
        }

        end = start + 1;
        if (line[start] == '\"') {
          while (end < len) {
            if (line[end] == '\"') {
              if (end < len - 1 && line[end + 1] == '\"') {
                ++end;
                continue;
              }
            }

            if (line[end] == '\"' && line[end - 1] != '\"'
                && line[end - 1] != '\\') {
              break;
            }
            ++end;
          }
          ++end;
        } else {
          while (end < len && line[end] != ',') {
            ++end;
          }
        }

        int row_idx = 0;
        for (int i = start; i < end; ++i) {
          char ch = line[i];
          if (ch >= 'A' && ch <= 'Z') {
            if (is_to_lower) {
              ch = ch + 'a' - 'A';
            }
          } else if (ch == 10 || ch == 13 || ch == '"') {
            continue;
          } else if (ch == '\\' && line[i+1] == '\"') {
            continue;
          }
          rows[row_count][row_idx++] = ch;
        }
        rows[row_count][row_idx] = 0;
        start = end + 1;
        ++row_count;
      }
#ifdef DEBUG_CSV
      for (int i = 0; i < row_count; ++i) {
        printf("--[%s]\n", rows[i]);
      }
#endif
      return true;
    }
};

#endif  // C_Author_Disambiguation_CSV
