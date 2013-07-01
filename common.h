/**
 *  =============================================================
 *
 *  	  Filename:  common.h
 *   Description:  
 *       Created:  05/27/2013 10:18:26 AM
 *       Version:  
 *      Revision:  #1;#4
 *        Author:  liujingming(liujingminghust@163.com)
 *
 *  =============================================================
 **/

#ifndef C_Author_Disambiguation_Common
#define C_Author_Disambiguation_Common
#include <string>
using namespace std;
class Common {
public:
  static bool is_same_char(char ch1, char ch2) {
    if (ch1 != ch2 && ch1 != '.'
        && ch2 != '.' && ch1 != '*' && ch2 != '*') {
      return false;
    }
    return true;
  }

  static bool is_not_valid_string(string &str1, string &str2) {
    if (str1.length() == 1 || str2.length() == 1) {
      if (str1[0] == str2[0]) {
        return false;
      } else {
        return true;
      }
    }

    if (str1.length() == 2 || str2.length() == 2) {
      for(unsigned int i = 0; i < str1.length() && i < str2.length(); ++i) {
        if (Common::is_same_char(str1[i], str2[i]) == false) {
          return true;
        } 
      }
      return false;
    }
    if (str1.length() > 5 && str2.length() > 5) {
      int same_length = string_same_length(str1, str2);
      if (same_length + 1 >= (int)str1.length() || same_length + 1 >= (int)str2.length()) {
        return false;
      }
    }
    if (Common::is_same_char(str1[1], str2[1]) == false
        ||Common::is_same_char(str1[2], str2[2]) == false) {
      return true;
    }
    return false;
  }

  static unsigned int string_same_length(string str1, string str2) {
    unsigned int len1 = str1.length();
    unsigned int len2 = str2.length();
    int head_same = 0;
    int tail_same = 0;
    for (unsigned int i = 0; i < len1 && i < len2; ++i) {
      if (str1[i] == str2[i]) {
        ++head_same;
      } else if (str1[i] == '*' || str2[i] == '*') {
        ++head_same;
        break;
      } else {
        break;
      }
    }
    for (unsigned int i = 0; i < len1 && i < len2; ++i) {
      if (str1[len1 - i - 1] == str2[len2 - i - 1]) {
        ++tail_same;
      } else if (str1[len1 - i - 1] == '*' ||
          str2[len2 - i - 1] == '*') {
        ++tail_same;
        break;
      } else {
        break;
      }
    }
    int same_len = head_same + tail_same;
    if (len1 > len2) {
      if (same_len > len2 && len2 < 7) {
        same_len = len2;
      }
    } else {
      if (same_len > len1 && len1 < 7) {
        same_len = len1;
      }
    }

    if (same_len >= 5 && head_same > 0) {
      if (len1 == len2 + 1) {
        if (same_len == len2 && str1[head_same] == 'e'
            && (head_same == same_len || same_len > 6 || str1[head_same-1] == 'o' || str1[head_same-1] == 'u')) {
          ++same_len;
        }
      } else if (len2 == len1 + 1) {
        if (same_len == len1 && str2[head_same] == 'e'
            && (head_same == same_len || same_len > 6 || str2[head_same-1] == 'o' || str2[head_same-1] == 'u')) {
          ++same_len;
        }
      }
    }

    if (len1 == len2 && same_len  == len2 - 2) {
      if (str1[head_same] == str2[head_same+1] && str2[head_same] == str1[head_same+1]) {
        if (len1 > 5) {
          same_len += 2;
        } else {
          ++same_len;
        }
      }
    }
    if (len1 >= len2 + 3 || (len1 >= len2 + 2 && len2 <= 4)) {
      if (str1.find(str2) == string::npos) {
        return same_len - 1;
      }
    } else if (len2 >= len1 + 3 || (len2 >= len1 + 2 && len1 <= 4)) {
      if (str2.find(str1) == string::npos) {
        return same_len - 1;
      }
    }
    if (head_same >= 7 || tail_same >= 7) {
      return same_len + 2;
    } 
    return same_len;
  }

};
#endif
