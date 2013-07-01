/**
 *  =============================================================
 *
 *
 *  	  Filename:  Affiliation.h
 *   Description:  
 *       Created:  06/04/2013 08:11:34 PM
 *       Version:  
 *      Revision:  #1;#4
 *        Author:  liujingming(liujingminghust@163.com)
 *
 *  =============================================================
 **/

#ifndef Affiliation
#define Affiliation
class AffiliationIndex {
  static AffiliationIndex* _instance;
  map<string, CMyArray> affiliation_index;
  map<int, set<string> > author_university;
  set<string> NULL_SET;
  public:
  static AffiliationIndex* get_instance() {
    if (_instance == NULL) {
      _instance = new AffiliationIndex();
    }
    return _instance;
  }

  void add(string _key, int author_id) {
    string key = get_university(_key);
    if (key.length() < 5) {
      return;
    }
    if (affiliation_index.find(key) == affiliation_index.end()) {
      affiliation_index[key] = CMyArray();
    }
    if (affiliation_index[key].find(author_id) == true) {
      return;
    }
    affiliation_index[key].addUnit(author_id);

    if (author_university.find(author_id) == author_university.end()) {
      author_university[author_id] = set<string>();
    }
    author_university[author_id].insert(key);
  }

  set<string> &get_author_university_by_author_id(int author_id) {
    if (author_university.find(author_id) == author_university.end()) {
      return NULL_SET;
    }
    return author_university[author_id];
  }

  list<int> *get_affiliation_index(string key) {
    if (affiliation_index.find(key) == affiliation_index.end()) {
      return NULL;
    }
    return affiliation_index[key].get_array();
  }

  static bool is_university_split(char ch) {
    if (ch == '|' || ch == ',' || ch == ';' || ch == '"' || ch == '.' || ch == '-') {
      return true;
    }
    //d. ron kikinis$ch == '|' || $ch == ',' || $ch == ';' || $ch == '"' || $ch == "." || $ch == "-")
    return false;
  }

  static string get_sub_string(string &affiliation, const string &reg) {
    unsigned long pos = affiliation.find(reg);

    string university;
    if (pos == string::npos || pos > 240) {
      return university;
    }
    int start = pos - 1;
    int end = pos + 1;
    for (start = pos - 1; start >= 0; --start) {
      if (is_university_split(affiliation[start])) {
        break;
      }
    }
    for (end = pos + 1; end < (int)affiliation.length(); ++end) {
      if (is_university_split(affiliation[end])) {
        break;
      }
    }

    while(affiliation[start+1] == ' ') {
      ++start;
    }
    if (end < (int)affiliation.length()) {
      while(is_university_split(affiliation[end])) {
        --end;
      }
    }
    university = affiliation.substr(start + 1, end - start);
    return university;
  }

  static string trim(const string &str) {
    char buf[1024];
    memset(buf, 0, sizeof(buf[0]) * 1024);
    int idx = 0;
    for (int i = 0; i < str.length(); ++i) {
      if (str[i] >= 'a' && str[i] <= 'z') {
        buf[idx++] = str[i];
      }
    }
    return string(buf);
  }

  static string get_university(string &affiliation) {
    string university;
    if (affiliation.length() < 5) {
      return "";
    }
    university = get_sub_string(affiliation, "univers");
    if (university.length() < 1) {
      university = get_sub_string(affiliation, "academy");
    }
    if (university.length() < 1) {
      university = get_sub_string(affiliation, "college");
    } 
    if (university.length() < 1) {
      university = get_sub_string(affiliation, "institute");
    } 
    if (university.length() < 1) {
      university = get_sub_string(affiliation, "institu");
    } 
    if (university.length() < 1) {
      university = get_sub_string(affiliation, "school");
    } 
    if (university.length() < 1) {
      university = get_sub_string(affiliation, "research");
    } 
    if (university.length() < 1) {
      university = affiliation;
    }
    //university = trim(university);
    //printf("AIXX:%s\n", university.c_str());
    return university;
  }
};
#endif
