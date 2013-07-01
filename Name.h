/**
 *  =============================================================
 *
 *    	Filename:  test.cpp
 *   Description:  name class
 *       Created:  05/06/2013 11:27:35 AM
 *       Version:  
 *      Revision:  #1;#4
 *        Author:  liujingming(liujingminghust@163.com)
 *
 *  =============================================================
 **/
#ifndef C_Author_Disambiguation_Name
#define C_Author_Disambiguation_Name
#include <string.h>
#include <set>
#include <iterator>
#include <map>
#include <string>
#include <vector>
#include "common.h"
#include "NameIndex.h"
#include "Tom.h"
#define FROM_CG 2
#define VALID_LOST 3
#define FROM_COMMON 1

using namespace std;
enum Type {
  COMMON_TYPE = 0,
  ABBR_TYPE = 1,
  STAR_TYPE = 2,
};

class SubName {
 public:
  string text;
  Type abbr_type;
  SubName(){
  }
  
  SubName(const string &_text, Type _abbr_type) {
    text = _text;
    abbr_type = _abbr_type;
  }

  SubName(const SubName &sub_name) {
    text = sub_name.text;
    abbr_type = sub_name.abbr_type;
  }
};

#define IS_NAME_COMPLETE_MATCH_NOT_ABBR 30
#define IS_NAME_COMPLETE_MATCH_IS_ABBR 20
#define NOT_CONSIDER -1
#define IS_NAME_COMPLETE_CONTAIN 23
#define IS_NAME_COMPLETE_ABBR_MATCH 18
#define IS_NAME_COMPLETE_MATCH_TRANSFER 25
#define IS_NAME_NOT_COMPLETE_ABBT_MATCH 16

class CName {
 public:
  static map<string, int> family_name_map;
  static map<string, set<string> > family_name_count_map;
  static map<string, set<string> > family_name_first_name_map;
  static map<string, vector<CName*> > global_similar_author;
  static map<string, map<string, int> > similar_family_name_map;
  string raw_name;
  string normalized_name;
  string family_name;
  string simply_name;
  vector<SubName> names;
  bool is_abbr;
  int confidence;
  CName() {
  }

  explicit CName(const string &_name) {
    confidence = 0;
    raw_name = _name;
    if (raw_name.length() < 4) {
      return;
    }
    normal_name();
  }

  void add_family_name() {
    if (family_name.length() > 2) {
      if (family_name_map.find(family_name) == family_name_map.end()) {
        family_name_map[family_name] = 1;
        family_name_count_map[family_name] = set<string>();
        family_name_first_name_map[family_name] = set<string>();
      } else {
        family_name_map[family_name] = family_name_map[family_name] + 1;
      }
      set<string> &family_name_count_set = family_name_count_map[family_name];
      family_name_count_set.insert(normalized_name.substr(0,2));

      set<string> &family_name_first_name_set = family_name_first_name_map[family_name];
      family_name_first_name_set.insert(names[0].text);
    }
  }

  static string normal_name(string &input_name) {
    char new_name[64];
    memset(new_name, 0, 64);
    int idx = 0;
    int family_start = 0;
    for (unsigned int i = 0; i < input_name.length(); ++i) {
      char ch = input_name[i];
      if (ch == '\\' && input_name[i+1] == '\'') {
        ++i;
        continue;
      }
      if (ch >= 'A' && ch <= 'Z') {
        ch = ch + 'a' - 'A';
      }
      if (ch == '-' || ch == '\'') {
        ch = ' ';
      } else if (ch < 0 || ch == '*') {
        ch = '*';
      } else if (ch == '(' && input_name[input_name.length() - 1] == ')') {
        break;
      } else if (ch == '?' || ch == '`' || ch < 0) {
        continue;
      } else if (i > 0 && ch != ' ' && ch != '.' && (ch > 'z' || ch < 'a' )) {
        continue;
      }
      if (ch == ' ' && new_name[idx - 1] == ' ') {
        continue;
      }
      new_name[idx++] = ch;
      if (ch == '.' && input_name[i+1] != ' ') {
        new_name[idx++] = ' ';
      }
      if (idx > 3 && new_name[idx-1] == ' ' && new_name[idx-3] == ' ') {
        new_name[idx-1] = '.';
        new_name[idx++] = ' ';
      }
      if (new_name[idx - 1] == ' ') {
        family_start = idx;
      }
    }
    while (new_name[idx-1] == ' ') {
      new_name[idx-1] = '\0';
      --idx;
    }

    char new_name2[64];
    memset(new_name2, 0, 64);
    int new_idx = 0;
    new_name2[new_idx++] = new_name[0];
    if (new_name[1] == ' ') {
      new_name2[new_idx++] = '.';
    }
    bool is_wildcard = false;  // *
    if (new_name[0] == '*') {
      is_wildcard = true;
    }
    int blank_num = 0;
    for (unsigned int i = 1; i < strlen(new_name); ++i) {
      if (i > 2 && new_name[i] == '.' && new_name[i-2] != ' ') {
        continue;
      }
      if (new_name[i] == '*') {
        if (is_wildcard == false) {
          new_name2[new_idx++] = new_name[i];
          is_wildcard = true;
        } else {
          is_wildcard = false;  // **
        }
      } else {
        new_name2[new_idx++] = new_name[i];
        is_wildcard = false;
      }
      if (new_name[i] == ' ') {
        ++blank_num;
      }
    }

    delete_tail(new_name2, " jr");
    delete_tail(new_name2, " .");
    delete_tail(new_name2, " ii");
    delete_tail(new_name2, " iii");
    delete_tail(new_name2, " .");
    delete_tail(new_name2, " .");
    delete_tail(new_name2, " junior");
    delete_tail(new_name2, " *");
    if (strlen(new_name2) < 5) {
      new_name2[0] = 0;
    } else if (blank_num < 3){
      if (new_name2[strlen(new_name2) - 1] == '.'
          && new_name2[strlen(new_name2) - 3] == ' ') {
        new_name2[strlen(new_name2) - 3] = 0;
      }
    }
    return string(new_name2);
  }

  void normal_name() {
    normalized_name = normal_name(raw_name);
    int blank_pos[10];
    int idx = 0;
    memset(blank_pos, 0, 10*sizeof(blank_pos[0]));
    blank_pos[idx++] = -1;
    for (unsigned int i = 0; i < normalized_name.length(); ++i) {
      if (normalized_name[i] == ' ') {
        blank_pos[idx++] = i;
      }
    }
    blank_pos[idx] = normalized_name.length()+1;

#ifdef DEBUG_SUBNAME
    printf("%s\n", normalized_name.c_str());
#endif
    is_abbr = false;
    for (int i = 0; i < idx; ++i) {
      SubName sub_name;
      int len = blank_pos[i+1] - blank_pos[i] - 1;
      sub_name.text = normalized_name.substr(blank_pos[i]+1, len);
      sub_name.abbr_type = sub_name.text.length() == 1?ABBR_TYPE:COMMON_TYPE;
      for (unsigned int i = 0; i < sub_name.text.length(); ++i) {
        if (sub_name.text[i] == '.') {
          sub_name.abbr_type = ABBR_TYPE;
          is_abbr = true;
        }
        if (sub_name.text[i] == '*') {
          sub_name.abbr_type = STAR_TYPE;
        }
      }
      if (sub_name.text.length() == 0) {
        continue;
      }

      if (sub_name.text.length() == 1) {
        if ('*' == sub_name.text[0]) {
          continue;
        }
        if ('.' == sub_name.text[0]) {
          continue;
        }
        sub_name.text = sub_name.text + ".";
        is_abbr = true;
      } else if (sub_name.text.length() < 2 && sub_name.abbr_type == STAR_TYPE) {
        if (sub_name.text[0] == '*') {
          continue;
        } else if (sub_name.text[1] == '*') {
          sub_name.text[1] = '.';
          sub_name.abbr_type = ABBR_TYPE;
        }
        //continue; //BY LJM
      }
      if (sub_name.text[sub_name.text.length()-1] == '.') {
        if (sub_name.text.length() < 5) {
          //sub_name.text = sub_name.text.substr(0, 1) + ".";
        } else {
          sub_name.text = sub_name.text.substr(0, sub_name.text.length()-1);
          sub_name.abbr_type = COMMON_TYPE;
        }
      }
      names.push_back(sub_name);
      if (names.size() > 6) {
        break;
      }
#ifdef DEBUG_SUBNAME
      printf("[%s]\n", sub_name.text.c_str());
#endif
    }
    if (names.size() == 0) {
      return;
    }

    simply_name = "";
    for (unsigned int i = 0; i < names.size() - 1; ++i) {
      if (names[i].text.length() > 2) {
        is_abbr = false;
        ++confidence;
      }
      simply_name = simply_name + names[i].text[0];
    }
    simply_name = simply_name + names[names.size()-1].text[0];
    normalized_name = "";
    for (unsigned int i = 0; i < names.size()-1; ++i) {
      normalized_name = normalized_name + names[i].text + " ";
    }
    normalized_name = normalized_name + names[names.size()-1].text;

#ifdef DEBUG_SUBNAME
    exit(-1);
#endif

    family_name = names[names.size()-1].text;
#ifdef DEBUG_FAMILY_NAME
    printf("family_name:[%s]\n", family_name.c_str());
#endif
    
  }
    
  static bool delete_tail(char *name, const char *tail) {
      unsigned int len1 = strlen(name);
      unsigned int len2 = strlen(tail);
      if (len1 < len2) {
        return false;
      }
      for (unsigned int i = 0; i < len2; ++i) {
        if (name[len1 - len2 + i] != tail[i]) {
          return false;
        }
      }
      name[len1 - len2] = 0;
      return true;
    }


  void Print() {
    printf("%s\n", ToString().c_str());
  }

  string ToString() {
    return "[" + raw_name + "]\t[" + normalized_name
      + "]\t[" + simply_name + "]\t[" + family_name + "]";
  }

  int MyCompareString(SubName &name1, SubName &name2) {
    int len1 = name1.text.length();
    int len2 = name2.text.length();
    if (len1 >= 2 && len2 >= 2
        && (name1.text[len1 - 1] == '.' || name2.text[len2 - 1] == '.')) {
      int smaller = len1 > len2 ? len2 : len1;
      int diff = 0;
      for (int i = 0; i < smaller-1; ++i) {
        if (name1.text[i] != name2.text[i]) {
          diff = 1;
          break;
        }
      }
      if (diff == 0) {
        return 0;
      } else {
        return 3;
      }
    }
    if (name1.text[0] == name2.text[0] && (len1 == 1 || len2 == 1)) {
      return 0;
    }
    if (len1 == len2) {
      bool is_same = true;
      for (int i = 0; i < len1; ++i) {
        if (name1.text[i] != name2.text[i]
            && name1.text[i] != '*' && name2.text[i] != '*') {
          is_same = false;
          break;
        }
      }
      if (is_same == true) {
        return 0;
      }
    }
    /*
    NameIndex *name_index = NameIndex::get_instance();
    if (name_index->get_size(name1.text) > 20 && name_index->get_size(name1.text) > 20) {
      return 5;
    }
    */

    int same = Common::string_same_length(name1.text, name2.text);
    int minlen = len1 > len2?len2:len1;
    int maxlen = len1 > len2?len1:len2;
    /*
    if (maxlen > minlen && same > minlen && minlen < 7) {
      same = minlen;
    }
    */

    if (same >= maxlen) {
      return 0;
    } else if (same >= minlen && minlen > 7) {
      return 1;
    } else if (same >= minlen || same == maxlen - 1) {
      return 1;
    } else if (same == minlen - 1) {
      return 2;
    } else if (same < minlen) {
      return 4;
    } else {
      return 5;
    }
  }

  // [jas c. lang] [jc lang]
  bool is_first_char_merger(CName *cname) {
    if (names.size() == 1 + cname->names.size()) {
      return _is_first_char_merger(cname);
    } else if (names.size() + 1 == cname->names.size()) {
      return cname->_is_first_char_merger(this);
    }
    return false;
  }
  bool _is_first_char_merger(CName *cname) {
    if (cname->names[0].text.length() != 2) {
      return false;
    }
    if (cname->names[0].text[0] != names[0].text[0]
        || cname->names[0].text[1] != names[1].text[0]) {
      return false;
    }
    for (unsigned int i = 1; i < cname->names.size(); ++i) {
      if (MyCompareString(cname->names[i], names[i+1]) != 0) {
        return false;
      }
    }
    return true;
  }


  /*
  bool is_similar_family_name(string str) {
    string larger_string  = family_name.length()>str.length()?family_name:str;
    string smaller_string = family_name.length()>str.length()?str:family_name;
    if (larger_string.find(smaller_string) != string::npos) {
      if (smaller_string.length() < 4 && larger_string.length() > smaller_string.length() + 1) {
        return false;
      }
      return true;
    }
    if (Common::string_same_length(smaller_string, larger_string) + 2 >= smaller_string.length()) {
      return true;
    }
    return false;
  }
  */

  void _add_similar_family_name(string str1, string str2) {
    if (similar_family_name_map.find(str1) == similar_family_name_map.end()) {
      similar_family_name_map[str1] = map<string, int>();
    }
    map<string, int> &map1 = similar_family_name_map[str1];
    if (map1.find(str2) == map1.end()) {
      map1[str2] = 1;
    } else {
      map1[str2] = map1[str2] + 1;
    }
  }

  /*
  void add_similar_family_name(CName *cname) {
    if (family_name.length() < 3 || cname->family_name.length() < 3) {
      return;
    }
    if (family_name.compare(cname->family_name) == 0) {
      return;
    }
    if (is_similar_family_name(cname->family_name) == false) {
      return;
    }
    _add_similar_family_name(family_name, cname->family_name);
    _add_similar_family_name(cname->family_name, family_name);
  }
*/
  void print_similar_family_name(){
    for (map<string, map<string, int> >::iterator beg = similar_family_name_map.begin();
        beg != similar_family_name_map.end();
        ++beg) {
      printf("[%s]:{", beg->first.c_str());
      for (map<string, int>::iterator it1 = beg->second.begin();
          it1 != beg->second.end();
          ++it1) {
        printf("[%s:%d],", it1->first.c_str(), it1->second);
      }
      printf("}\n");
    }
  }

  bool is_strict_contain_name(CName *cname, bool is_more_strict, string &match_string) {
    if (cname->names.size() > names.size()) {
      return false;
    }
    if (cname->names.size() < names.size() && names.size() == 2
        && cname->normalized_name.length() > normalized_name.length()) {
      return false;
    }
    bool is_done[10];
    memset(is_done, 0, sizeof(is_done[0])*10);
    int found_count = 0;
    int found_length = 0;
    match_string = "";
    Tom *tom = Tom::get_instance();
    for (unsigned int i = 0; i < cname->names.size(); ++i) {
      string str = cname->names[i].text;
      bool found = false;
      for (unsigned int j = 0; j < names.size(); ++j) {
        if (is_done[j] == 1) {
          continue;
        }
        is_done[j] = 1;
        if (names[j].text.compare(str) == 0 ) {
          found_length += str.length();
          found = true;
          match_string = match_string + str + " ";
          break;
        }
        
        if (i != cname->names.size() - 1 && j != names.size() - 1 && (i == 0 || j == 0) && tom->is_tom(names[j].text, str)) {
          found_length += str.length();
          found = true;
          match_string = match_string + str + " ";
          break;
        }
        
        if (is_more_strict != true) {
          if (names[j].text.length() > str.length() && (str.length() > 5)) {
            if ( names[j].text.find(str) == 0) {
              found_length += str.length();
              found = true;
              match_string = match_string + str + " ";
              break;
            }
          }
          if (names[j].text.length() < str.length()
              && (names[j].text.length() > 5)) {
            if (str.find(names[j].text) == 0) {
              found_length += str.length();
              found = true;
              match_string = match_string + names[j].text + " ";
              break;
            }
          }
        }
        if (names[j].text.find("*") != string::npos || str.find("*") != string::npos) {
          if (MyCompareString(names[j], cname->names[i]) == 0) {
              found_length += str.length();
              found = true;
              match_string = match_string + names[j].text + " ";
          }
        }
        is_done[j] = 0;
      }
      if (cname->names[i].abbr_type != ABBR_TYPE && found == false) {
        return false;
      }
      if (found) {
        ++found_count;
      }
    }
    for (unsigned int j = 0; j < names.size(); ++j) {
      if (is_done[j] == 0 && names[j].abbr_type != ABBR_TYPE) {
        return false;
      }
    }
    if (found_count < (int)cname->names.size() && found_count < (int)names.size()) {
      return false;
    }
    match_string = match_string.substr(0, match_string.length()-1);
    return true;
  }


  bool is_contain_name(CName *cname, const char *from, string &match_string) {
    if (cname->names.size() > names.size()) {
//        || cname->normalized_name.length() > normalized_name.length()) {
      return false;
    }
    if (strcmp(from, "CG") != 0) {
      if (family_name_map[cname->family_name] > 50 && cname->normalized_name.length() < 12) {
        return false;
      }
    }
    int is_comp = 0;
    bool is_done[10];
    Tom *tom = Tom::get_instance();
    memset(is_done, 0, sizeof(is_done[0])*10);
    for (unsigned int i = 0; i < cname->names.size(); ++i) {
      string str = cname->names[i].text;
      bool found = false;
      for (unsigned int j = 0; j < names.size(); ++j) {
        if (is_done[j] == 1) {
          continue;
        }
        is_done[j] = 1;
        
        if (((i != cname->names.size() - 1 && j != names.size() - 1 && (i == 0 || j == 0))|| strcmp(from, "CG") == 0) && tom->is_tom(names[j].text, str)) {
          found = true;
          match_string = match_string + str + " ";
          ++is_comp;
          break;
        }
        
        if (names[j].text.find(str) == 0) {
          if (names[j].text.length() == str.length()
              || str.length() > 4
              || (strcmp(from, "CG") == 0 && str.length() > 2)) {
            found = true;
            match_string += match_string + str + " ";
            if (str.length() > 2) {
              ++is_comp;
            }
            break;
          }
          
        } else if (str.find(names[j].text) == 0) {
          if (names[j].text.length() == str.length()
              || names[j].text.length() > 4
              || (strcmp(from, "CG") == 0 && names[j].text.length() > 2)) {
            /*
            found = true;
            match_string += match_string+names[j].text + " ";
            if (str.length() > 2) {
              ++is_comp;
            }
            break;
            */
          }
        } else if (names[j].text[0] == str[0]
            &&(names[j].text[1] == '.' || str[1] == '.')) {
          found = true;
          match_string += match_string + str + " ";
          break;
        }
        is_done[j] = 0;
      }
      if (found == false) {
        return false;
      }
    }
    match_string = match_string.substr(0, match_string.length()-1);
    if (is_comp > 2 || (int)cname->names.size() == is_comp || strcmp(from, "CG") == 0) {
      return true;
    } else {
      return false;
    }
  }

  bool is_star_name(CName *cname) {
    if (normalized_name.length() != cname->normalized_name.length()) {
      return false;
    }
    for (unsigned int i = 0; i < normalized_name.length(); ++i) {
      if (normalized_name[i] != cname->normalized_name[i]
          && normalized_name[i] != '*' && cname->normalized_name[i] != '*') {
        return false;
      }
    }
    return true;
  }

  bool is_ignore_blank_space(CName *cname) {
    unsigned int idx1 = 0;
    unsigned int idx2 = 0;
    while(idx1 < normalized_name.length()
        && idx2 < cname->normalized_name.length()) {
      if (normalized_name[idx1] == cname->normalized_name[idx2]
          || normalized_name[idx1] == '*'
          || cname->normalized_name[idx2] == '*') {
        ++idx1;
        ++idx2;
      } else if (normalized_name[idx1] == ' ') {
        ++idx1;
      } else if (cname->normalized_name[idx2] == ' ') {
        ++idx2;
      } else {
        return false;
      }
    }
    if ((idx1 == normalized_name.length() || (idx1 == normalized_name.length() - 1 && normalized_name.length() > 10))
        && (idx2 == cname->normalized_name.length() || (idx2 == cname->normalized_name.length() - 1 && cname->normalized_name.length() > 10))) {
      return true;
    }
    /*
    printf("is_ignore_blank_space:%d %d %d %d [%s] [%s]\n",
        idx1, idx2, (int)normalized_name.length(), (int)cname->normalized_name.length(),
        normalized_name.c_str(), cname->normalized_name.c_str());
        */
    return false;
  }

  bool is_lost_middle_name(CName *cname, int is_strict) {
    if (is_strict == VALID_LOST && cname->names.size() == names.size()) {
      return _is_lost_middle_name(cname, this, is_strict);
    }
    if (cname->names.size() > names.size()) {
      return _is_lost_middle_name(cname, this, is_strict);
    } else if (cname->names.size() < names.size()){
      return _is_lost_middle_name(this, cname, is_strict);
    } else {
      return false;
    }
  }

  bool _is_lost_middle_name(CName *cname1, CName *cname2, int is_strict) {
    if (cname2->names.size() == 1) {
      return false;
    }
    if (is_strict != FROM_CG && is_strict != VALID_LOST) {
      if (cname2->names.size() == 2) {
        if (cname1->normalized_name.length() > cname2->normalized_name.length() * 2) {
          return false;
        }
      }
      if ( family_name_map[cname2->family_name] > 30
          && cname2->normalized_name.length() < 10) {
        return false;
      }
    }
    bool hasfound[10];
    memset(hasfound, 0, sizeof(hasfound[0]) * 10);
    int no_abbr_num = 0;
    Tom *tom = Tom::get_instance();
    for (unsigned int i = 0; i < cname2->names.size(); ++i) {
      bool found = false;
      if (cname2->names[i].abbr_type != ABBR_TYPE) {
        for (unsigned int j = 0; j < cname1->names.size(); ++j) {
          if (hasfound[j] == false && cname1->names[j].abbr_type != ABBR_TYPE
              && MyCompareString(cname1->names[j], cname2->names[i]) == 0) {
            if (cname1->names[j].abbr_type != ABBR_TYPE
                && cname2->names[i].abbr_type != ABBR_TYPE) {
              ++no_abbr_num;
            }
            hasfound[j] = true; 
            found = true;
            break;
          } else {
            continue;
          }
        }
      } else {
        for (unsigned int j = 0; j < cname1->names.size(); ++j) {
          if (hasfound[j] == false && cname1->names[j].abbr_type == ABBR_TYPE
              && MyCompareString(cname1->names[j], cname2->names[i]) == 0) {
            hasfound[j] = true; 
            found = true;
            break;
          } else {
            continue;
          }
        }
      }
      if (found == false) {
        for (unsigned int j = 0; j < cname1->names.size(); ++j) {
          if (hasfound[j] == false) {
            int diff = MyCompareString(cname1->names[j], cname2->names[i]);
       //     if (is_strict == FROM_CG) {
              if (((i != cname2->names.size() - 1 && j != cname1->names.size() - 1 && (i == 0 || j == 0)) || is_strict == VALID_LOST)
                  && tom->is_tom(cname1->names[j].text, cname2->names[i].text)) {
                diff = 0;
              }
         //   }

            if (diff == 0 || (diff == 1 && is_strict == VALID_LOST)) {
              if (cname1->names[j].abbr_type != ABBR_TYPE
                  && cname2->names[i].abbr_type != ABBR_TYPE) {
                ++no_abbr_num;
              }
              hasfound[j] = true; 
              found = true;
              break;
            } else {
              continue;
            }
          } else {
            continue;
          }
        }
      }
      if (found == false ) {
        return false;
        break;
      }
    }
    if (no_abbr_num >= 2) {
      return true;
    }
    return false;
  }

  bool _is_star_to_blank(string star_string, CName *cname) {
    char new_name[64];
    memset(new_name, '0', 64);
    for (unsigned int i = 0; i < star_string.length(); ++i) {
      if (star_string[i] == '*') {
        new_name[i] = ' ';
      } else {
        new_name[i] = star_string[i];
      }
    }
    string new_name_string = string(new_name);
    CName new_c_name = CName(new_name_string);
    int similar = new_c_name._GetNameSimilarType(cname);
    if (similar > 0) {
      return true;
    }
    return false;
  }

  bool is_star_to_blank(CName *cname) {
    if (normalized_name.find('*') == normalized_name.length()-2) {
      return false;
    }
    if (normalized_name.find('*') != string::npos) {
      return _is_star_to_blank(normalized_name, cname);
    } else if (cname->normalized_name.find('*') != string::npos) {
      return _is_star_to_blank(cname->normalized_name, this);
    }
    return false;
  }

  bool is_invert_merger_name(CName *cname) {
    string str1 = "";
    string str2 = "";
    if (names[0].text.compare(cname->family_name) == 0) {
      for (unsigned int i = 1; i < names.size(); ++i) {
        str1 = str1 + names[i].text;
      }
      for (unsigned int i = 0; i < cname->names.size()-1; ++i) {
        str2 = str2 + cname->names[i].text;
      }
    }
    else if (family_name.compare(cname->names[0].text) == 0) {
      for (unsigned int i = 1; i < cname->names.size(); ++i) {
        str1 = str1 + cname->names[i].text;
      }
      for (unsigned int i = 0; i < names.size()-1; ++i) {
        str2 = str2 + names[i].text;
      }
    } else {
      return false;
    }
    if (str1.compare(str2) == 0) {
      return true;
    }
    return false;
  }

  bool is_invert_name(CName *cname) {
    if (names.size() != cname->names.size()) {
      return false;
    }
    if ((names.size() == 2 && is_abbr)
      || (cname->names.size() == 2 && cname->is_abbr)) {
        return false;
    }
    int size = names.size();
    // C A. B & A B. C
    int diff = 0;
    if (names[0].text.compare(cname->family_name) == 0
        && (cname->names[size-2].abbr_type != ABBR_TYPE)) {
      for (unsigned int i = 1; i < names.size(); ++i) {
        diff += MyCompareString(names[i], cname->names[i-1]);
      }
      if (diff == 0) {
        return true;
      }
    }

    // A B. C & C A. B
    diff = 0;
    if (family_name.compare(cname->names[0].text) == 0
        && (names[size-2].abbr_type != ABBR_TYPE)) {
      for (unsigned int i = 1; i < names.size(); ++i) {
        diff += MyCompareString(names[i-1], cname->names[i]);
      }
      if (diff == 0) {
        return true;
      }
    }

    bool hasfound[10];
    memset(hasfound, 0, sizeof(hasfound[0])*10);
    for (unsigned int i = 0; i < names.size(); ++i) {
      bool found = false;
      for (unsigned int j = 0; j < names.size(); ++j) {
        if (names[i].text.compare(cname->names[j].text) == 0 && hasfound[j] == false) {
          found = true;
          hasfound[j] = true;
          break;
        }
      }
      if (found == false) {
        return false;
      }
    }
    return true;
  }

  bool is_simply_name(CName *cname) {
    if (names.size() != cname->names.size()
        || simply_name.compare(cname->simply_name) != 0) {
      return false;
    }
    for (unsigned int i = 0; i < names.size(); ++i) {
      if (names[i].text.compare(cname->names[i].text) == 0) {
        continue;
      } else if (names[i].text[0] == cname->names[i].text[0]
          && (cname->names[i].text[1] == '.')) {
        continue;
      }
      return false;
    }
    return true;
  }

  int add_global_similar(string &normalized_name, CName *one){
    if (global_similar_author.find(normalized_name) == global_similar_author.end()) {
      global_similar_author[normalized_name] = vector<CName *>();
    }
    vector<CName *> &similar_list = global_similar_author[normalized_name];
    similar_list.push_back(one);
    return 0;
  }

  void valid_global_similar_author() {
    bool mark[400];
    bool is_ignore[400];
    for (map<string, vector<CName *> >::iterator it = global_similar_author.begin();
        it != global_similar_author.end();
        ++it) {
      string normalized_name = it->first;
      vector<CName *> &similar_list = it->second;
      if (similar_list.size() > 400) {
        continue;
      }
      if (normalized_name[1] == '.') {
        continue;
      }
      memset(mark, 0, 400);
      memset(is_ignore, 0, 400);
      int is_not_valid_num = 0;
      for (unsigned int i = 0; i < similar_list.size(); ++i) {
        CName * cname1 = similar_list[i];
        for (unsigned int j = i+1; j < similar_list.size(); ++j) {
          CName * cname2 = similar_list[j];
          int similar = cname1->_GetNameSimilarType(cname2);
          if (similar < 0) {
            mark[i] = 1;
            mark[j] = 1;
          }
          /*
          if (cname1->normalized_name[1] != '.' && cname2->normalized_name[1] != '.'
              && cname1->normalized_name[1] != cname2->normalized_name[1]) {
            is_ignore[i] = 1;
            is_ignore[j] = 1;
          }
          */
        }
      }
      if (similar_list.size() > 50) {
        is_not_valid_num = 400;
        memset(mark, 1, 400);
      } else {
        for (unsigned int i = 0; i < similar_list.size(); ++i) {
          if (mark[i] == true && is_ignore[i] == 0) {
            ++is_not_valid_num;
          }
        }
      }
      if (is_not_valid_num > 0) {
        printf("GLOBAL [%3d] [%s]\n", (int)similar_list.size(), normalized_name.c_str());
        printf("normalized_name:[%s][%d]\n", normalized_name.c_str(), is_not_valid_num);
        for (unsigned int i = 0; i < similar_list.size(); ++i) {
          if (mark[i] == true && is_ignore[i] == 0) {
            similar_list[i]->Print();
          }
        }
      }
    }
  }

  int GetNameSimilarType(CName *cname) {
    int similar = _GetNameSimilarType(cname);
    if (similar > 0) {
      /*
      add_global_similar(normalized_name, cname);
      add_global_similar(cname->normalized_name, this);
      */
    }
    return similar;
  }

  bool is_similar_family_name(string str1, string str2) {
    string larger_string  = str1.length()>str2.length()?str1:str2;
    string smaller_string = str1.length()>str2.length()?str2:str1;
    if (larger_string.length() > smaller_string.length() + 3) {
//      return false;
    }
    set<string> set2 = family_name_first_name_map[larger_string];
    set<string> set1 = family_name_first_name_map[smaller_string];
    int similar = 0;
    for (set<string>::iterator it = set1.begin();
        it != set1.end();
        ++it){
      if (set2.find(*it) != set2.end()) {
        ++similar;
      }
    }
    int smaller_size = set1.size() > set2.size() ? set2.size() : set1.size();
    if (similar * 3 > smaller_size || smaller_size - similar < 2) {
      return true;
    }
    return false;
  }

  bool is_wrong_family_name(string str) {
    if (str.find("*") != string::npos) {
      return true;
    }
    if (family_name_map[str] > 10
        || (str.length() < 5 && family_name_map[str] > 2)) {
      return false;
    }
    if (family_name_count_map.find(str) != family_name_count_map.end()
        && family_name_count_map[str].size() > 3) {
      return false;
    }
    return true;
  }

  int get_family_name_people_num() {
    if (family_name_map.find(family_name) == family_name_map.end()) {
      return 0;
    }
    return family_name_map[family_name];
  }

  bool is_valid_family_name(string str1, string str2) {
    if (is_wrong_family_name(str1) == true) {
      return true;
    }
    if (is_wrong_family_name(str2) == true) {
      return true;
    }

    if (str1.compare(str2) == 0) {
      return true;
    }

    if (family_name_map[str1] > 20 && family_name_map[str2] > 20) {
      return false;
    }

    if (is_similar_family_name(str1, str2)) {
      return true;
    }
    return false;
  }

  bool up_time(string &str1, string &str2) {
    if (str1.length() != str2.length() ) {
      return false;
    }
    for (unsigned int i = 0; i < str1.length(); ++i) {
      if (str2.find(str1[i]) == string::npos) {
        return false;
      }
    }
    return true;
  }

  int _GetNameSimilarType(CName *cname) {
    // based by the same simply_name
    
    if (names.size() != cname->names.size()) {
      return NOT_CONSIDER;
    }

    if (cname->normalized_name.length() < 5 || normalized_name.length() < 5) {
      return NOT_CONSIDER;
    }

    if (up_time(simply_name, cname->simply_name) == true) {
      if (cname->normalized_name.compare(normalized_name) == 0) {
        if (is_abbr) {
          return IS_NAME_COMPLETE_MATCH_IS_ABBR;
        } else {
          return IS_NAME_COMPLETE_MATCH_NOT_ABBR;
        }
      }
      if (is_invert_name(cname) == true)  {
        return IS_NAME_COMPLETE_CONTAIN;
      }
    }

    if (names.size() < 2
        || cname->names.size() < 2
        || names.size() != cname->names.size()
        || simply_name.compare(cname->simply_name) != 0
        || (names[names.size()-1].text.length() <= 2 && cname->names.size() == 2 && names[names.size()-1].abbr_type == ABBR_TYPE)
        || (cname->names[cname->names.size()-1].text.length() <= 2 && cname->names.size() == 2 && cname->names[cname->names.size()-1].abbr_type == ABBR_TYPE)) {
      return NOT_CONSIDER;
    }

    if ((family_name.length() < 5 || cname->family_name.length() < 5)) {
      if ((names.size() < 3 && is_abbr)
          || (cname->names.size() < 3 && cname->is_abbr)) {
        return NOT_CONSIDER;
      }
    }

    bool is_strict = false;
    if (family_name.length() < 3 || cname->family_name.length() < 3) {
      is_strict = true;
    }

    if (family_name_map[family_name] < 3 || family_name_map[cname->family_name] < 3) {
      
    } else if (is_valid_family_name(family_name, cname->family_name) == false) {
      return NOT_CONSIDER;
    }
    /*
    if (family_name_map[family_name] > 6 && family_name_map[cname->family_name] > 6 && family_name.compare(cname->family_name) != 0
        && family_name.find('*') == string::npos && cname->family_name.find('*') == string::npos) {
      return NOT_CONSIDER;
    }
    */
/*
    if (normalized_name.length() > 10 && cname->normalized_name.length() > 10
        && family_name.length() > 4 && cname->family_name.length() > 4) {
      if (normalized_name.find(cname->normalized_name) == 0
          || cname->normalized_name.find(normalized_name) == 0) {
        return IS_NAME_COMPLETE_CONTAIN;
      }
    }
*/
    // A B C & A B. C
    int diff = 0;
    int complete_matches = 0;
    int count = 0;
    Tom *tom = Tom::get_instance();
    for (unsigned int i = 0; i < cname->names.size(); ++i) {
      int temp = MyCompareString(names[i], cname->names[i]);
      if (i != cname->names.size() - 1 && tom->is_tom(names[i].text, cname->names[i].text)) {
        temp = 0;
      }
      if (temp == 0) {
        if (names[i].text.length()  > 2 && cname->names[i].text.length() > 2) {
          ++complete_matches;
        }
        continue;
      }
      diff += temp;
      if ( i != cname->names.size()-1) {
        if (names.size() < 4 || cname->is_abbr || is_abbr) {
          if (cname->family_name.length() + cname->names[i].text.length() < 11
              ||family_name.length() + names[i].text.length() < 11) {
            diff += temp;
            if (cname->names[i].text.find(names[i].text) || names[i].text.find(cname->names[i].text)) {
              ++count;
            }
          }
        }
      } else {
        if(family_name_map[family_name] > 1 && family_name_map[cname->family_name] > 1) {
          ++count;
          diff += temp;
        } else if (family_name.length() > cname->family_name.length() + 1
            && family_name.find(cname->family_name) != 0){
          ++count;
          diff += temp;
        } else if (cname->family_name.length() > family_name.length() + 1
            && cname->family_name.find(family_name) != 0){
          ++count;
          diff += temp;
        } else if ((cname->family_name.length() < 5 && cname->family_name.length() + 2 < family_name.length())
            || (family_name.length() < 5 && family_name.length() + 2 < cname->family_name.length())) {
          ++count;
          diff += temp;
        } else if ((cname->family_name.length() < 5 && family_name.find(cname->family_name) == string::npos)
            || (family_name.length() < 5 && cname->family_name.find(family_name) == string::npos)){
          ++count;
          diff += temp;
        } else if (cname->family_name.length() == family_name.length() && family_name.length() < 7) {
          ++count;
          diff += temp;
        }
      }
      if (cname->normalized_name.length() < 13 || normalized_name.length() < 13) {
        diff += temp;
      }
    }
    if (family_name_map[family_name] > 30 && family_name_map[cname->family_name] > 30 && cname->names.size() <= 3) {
      ++diff;
    }
    if (is_strict) {
      if (complete_matches < 3 || complete_matches < (int)cname->names.size() - 1) {
        return NOT_CONSIDER;
      }
    }
    if (diff == 2 &&  count == 1) {
#ifdef DEGUB
      printf("VK:[%s] [%s]\n", normalized_name.c_str(), cname->normalized_name.c_str());
#endif
      return -100;
    }
    if (complete_matches == 0) {
      if (diff < 3 && cname->names.size() > 2 && cname->family_name.length() >= 5 && family_name.length() >= 5) {
#ifdef DEBUG
        printf("VK:[%s] [%s]\n", normalized_name.c_str(), cname->normalized_name.c_str());
#endif
        return -100;
      } else {
        return NOT_CONSIDER;
      }
    }

    if (diff > 1 && diff < 4) {
      int len = 0;
      for (unsigned int i = 0; i < cname->names.size(); ++i) {
        int temp = MyCompareString(names[i], cname->names[i]);
        if ((i != cname->names.size() - 1 && tom->is_tom(names[i].text, cname->names[i].text))
            || names[i].text.compare(cname->names[i].text) == 0) {
          len += names[i].text.length() > 5?5:names[i].text.length();
          continue;
        }
        int same_len = 0;
        if (names[i].abbr_type != ABBR_TYPE && cname->names[i].abbr_type != ABBR_TYPE) {
          if (names[i].text.length() > cname->names[i].text.length() + 2
              || cname->names[i].text.length() > names[i].text.length() + 2) {
            continue;
          }
          same_len = Common::string_same_length(names[i].text, cname->names[i].text);
        } else {
          if (names[i].text[0] == cname->names[i].text[0]) {
            same_len = 3;
          }
        }

        len += same_len > 5?5:same_len;
      }
      if (len < 13) {
        return NOT_CONSIDER;
      }
      if (len > 13) {
        return 999;
      }
      return -200;
    }
    if (diff == 0) {
      return IS_NAME_COMPLETE_ABBR_MATCH;
    } else if (diff == 1) {
      return IS_NAME_NOT_COMPLETE_ABBT_MATCH;
    }

    return NOT_CONSIDER;
  }
};


#endif  // C_Author_Disambiguation_Name
