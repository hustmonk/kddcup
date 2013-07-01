/**
 *  =============================================================
 *
 *
 *  	Filename:  Author.h
 *   Description:  author infomation from [../data/Author.csv]
 *       Created:  05/06/2013 03:26:50 PM
 *       Version:  
 *      Revision:  #1;#4
 *        Author:  liujingming(liujingminghust@163.com)
 *
 *  =============================================================
 **/
#ifndef C_Author_Disambiguation_Author
#define C_Author_Disambiguation_Author

#include <string>
#include <stdio.h>
#include <map>
#include <list>
#include <vector>
#include "PaperAuthor.h"
#include "Name.h"
#include "LossMiddleName.h"
#include "NameIndex.h"

using namespace std;
extern char DATA_DIR[];

#define SPECIAL_SCORE 400
#define IS_SPECIAL_SCORE(x) (x >= SPECIAL_SCORE && x < SPECIAL_SCORE + 50)

class Candidate {
  public:
    int author_id;
    int score;
    int my_name_type;
    int other_name_type;
};

typedef struct _GameNameCount{
  string text;
  int    count;
} GameNameCount; 
int cmp_game_name_count(const void *a, const void *b) {
  return ((GameNameCount *)b)->count - ((GameNameCount *)a)->count;
}

class CAuthor {
  map<int, int> name_similar_candidates; //
  vector<Candidate> same_author_candidates;
  map<int, int> my_name_type_map;    // for checking
  map<int, int> other_name_type_map; // for checking
  CName cname; // author name infomation
  vector<CName*> game_names; // name from author's papers
  bool is_game_name;  // is contain game name
  int id;
  int paper_num;
  vector<int> game_status;
  string affiliation;
  string university;
  map<int, bool> parters;
  map<string, int> game_name_confidence;
  vector<CName *> mypaper_name_list;

 public:
  bool get_is_game_name() {
    return is_game_name;
  }
  vector<CName *> &get_mypaper_name_list() {
    return mypaper_name_list;
  }

  vector<SubName> &get_names(unsigned int name_type) {
    if (name_type == 0) {
      return cname.names;
    }
    if (name_type <= game_names.size()) {
      return game_names[name_type - 1]->names;
    } else {
      printf("ERROR TYPE:[%d] [%d]", name_type, (int)game_names.size());
    }
    return cname.names;
  }

  string &get_simply_name() {
    return cname.simply_name;
  }

  int get_names_size() {
    return cname.names.size();
  }

  vector<Candidate> &get_same_author_candidates() {
    return same_author_candidates;
  }

  bool ContainCandidate(int author_id) {
    if (author_id == id) {
        return true;
    }
    if (name_similar_candidates.find(author_id) == name_similar_candidates.end()) {
      return false;
    }
    return true;
  }

  void delete_candidate(int author_id) {
    for (unsigned int i = 0; i < same_author_candidates.size(); ++i) {
      if (same_author_candidates[i].author_id == author_id) {
        same_author_candidates.erase(same_author_candidates.begin() + i);
        break;
      }
    }
  }

  void clear_same_author_candidates() {
    same_author_candidates.clear();
  }

  string &get_university() {
    return university;
  }

  void deal_paper_name() {
    CPaPerAuthorList *paper_author_list = CPaPerAuthorList::get_instance();
    map<int, string> *author_names
      = paper_author_list->get_author_names_by_author_id(id);
    if (author_names == NULL) {
      return;
    }
    map<string, int> name_set;
    for (map<int, string>::iterator it = author_names->begin();
        it != author_names->end();
        ++it) {
      CName paper_name = CName(it->second);
      string name = paper_name.normalized_name;
      if ((paper_name.names.size() < 4 && paper_name.is_abbr) || name.length() < 6
          || (paper_name.family_name.length() <= 2 && paper_name.names.size() < 3)) {
        continue;
      }
      if (name_set.find(name) == name_set.end()) {
        name_set[name] = 1;
      } else {
        name_set[name] = name_set[name] + 1;
      }
    }
    if (name_set.size() == 0) {
      return;
    }

    CName *max_name = NULL;
    int max_count = 0;
    for (map<string, int>::iterator it = name_set.begin();
        it != name_set.end();
        ++it
        ) {
      int confidence = 1;
      if (author_names->size() > 5 && author_names->size() < 20) {
        if (it->second == 1){
          confidence = 0;
        }
      } else if (author_names->size() >= 20) {
        if (it->second > 10) {

        } else if (it->second * 10 < (int)author_names->size()) {
          confidence = 0;
        }
      }
      
      if (author_names->size() >= 10) {
        if (it->second == 1){
          continue;
        }
      }

      CName *game_name = new CName(it->first);
      if (game_name->names.size() < 2) {
        continue;
      }
      if (it->second > max_count) {
        max_count = it->second;
        max_name = game_name;
      }
      mypaper_name_list.push_back(game_name);
      game_name_confidence[game_name->normalized_name] = confidence;
    }

    if (max_count == 0 || max_name == NULL) {
      return;
    }
    if (cname.normalized_name.length() < 5) {
      cname = CName(max_name->normalized_name);
    }
#ifdef DEBUG
    if (cname.normalized_name.find("*") != string::npos) {
      printf("STAR:[%s] [%s]\n", cname.raw_name.c_str(), max_name->raw_name.c_str());
    }
#endif
  }

  void add_is_loss_middle_name() {
    if (mypaper_name_list.size() == 0) {
      return;
    }

    LossMiddleName *loss_middle_name = LossMiddleName::get_instance();
    for (vector<CName*>::iterator it = mypaper_name_list.begin();
        it != mypaper_name_list.end(); ++it) {
      CName *game_name = *it;
      if (game_name->names.size() < 2) {
        continue;
      }
      bool status = cname.is_lost_middle_name(game_name, FROM_CG);
      if (status == true) {
        loss_middle_name->add(&cname, game_name);
      }
    }
  }


  void create_game_name() {
    if (mypaper_name_list.size() == 0) {
      return;
    }

    for (vector<CName*>::iterator it = mypaper_name_list.begin();
        it != mypaper_name_list.end(); ++it) {
      CName *game_name = *it;
      if (game_name == NULL) {
        continue;
      }
      if (game_name->names.size() < 2 || game_name_confidence[game_name->normalized_name] == 0) {
        continue;
      }
      int type = get_game_name_similar_type(*game_name, 1, paper_num, "CG");
#ifdef DEBUG
      if (cname.family_name.compare(game_name->family_name) == 0 && type == -1) {
        printf("KK:[%d] [%s] [%s]\n", id,
            cname.normalized_name.c_str(), game_name->normalized_name.c_str());
      }
#endif
      if (type == 16 && (game_name->is_abbr || game_name->confidence < 2 || game_name->normalized_name.length() < 15)) {
        continue;
      }
      if (game_name->confidence < 1 && game_name->names.size() < 5) {
        continue;
      }
      if (game_name->names.size() < cname.names.size()
          && game_name->normalized_name.length() < cname.normalized_name.length()) {
        continue;
      }
      if (type > 0 && type != 6 && type != 8) {
        game_names.push_back(game_name);
        game_status.push_back(type);
      } else if (type == 8){
        string temp_gam_name = game_name->normalized_name;
        CName *game_name = new CName(cname.normalized_name);
        game_names.push_back(game_name);
        game_status.push_back(type);
        cname = CName(temp_gam_name);
      }
    }
    if (game_names.size() > 2) {
      for (unsigned int i = 0; i < game_names.size(); ++i) {
        if (game_status[i] != 18) {
          continue;
        }
      }
    }
    if (game_status.size() != game_names.size()) {
      printf("ERROR\n");
    }
    if (game_names.size() > 0) {
      is_game_name = true;
#ifdef DEBUG
      printf("[_get_game_name:%d] [%d] ", id, paper_num);
      for (unsigned int i = 0; i < game_names.size(); ++i){
        printf("{%s}", game_names[i]->normalized_name.c_str());
      }
      printf("[%d]\n", (int)game_names.size());
#endif
    }
  }

  vector<CName*> &get_game_names() {
    return game_names;
  }

  CName *get_cname() {
    return &cname;
  }

  bool is_contain_dom(const char *str, char *str1, int len1, char *str2, int len2) {
    int len = strlen(str);
    memset(str1, 0, len1);
    memset(str2, 0, len2);
    int  idx1 = 0, idx2 = 0;
    bool is_dom = false;
    for (int i = 0; i < len; ++i) {
      if (str[i] == '\'') {
        is_dom = true;
        continue;
      }
      str1[idx1++] = str[i];
    }

    for (int i = 0; i < len; ++i) {
      if (str[i] == '\'') {
        str2[idx2++] = ' ';
      } else {
        str2[idx2++] = str[i];
      }
    }
    return is_dom;
  }

  CAuthor(const char *line, char **rows, int max_row_count, bool is_test = false) {
    char str1[64], str2[64];
    paper_num = 0;
    is_game_name = false;
    int row_cout;
    CSV::parser(line, rows, max_row_count, row_cout);
    assert(row_cout > 2);
    id = atoi(rows[0]);
    string name = string(rows[1]);
    if (is_contain_dom(rows[1], str1, 64, str2, 64)) {
      cname = CName(string(str1));
      CName *game_name = new CName(string(str2));
      game_names.push_back(game_name);
      game_status.push_back(55);
#ifdef DEBUG
      printf("familyup:[%s] [%s] [%d] [%s] [%s]\n",
          cname.family_name.c_str(), rows[1], id,
          cname.normalized_name.c_str(), game_name->normalized_name.c_str());
#endif
    } else {
      cname = CName(name);
    }
    cname = CName(name);
    if (cname.family_name.length() <= 2 && cname.normalized_name.length() > 5) {
      if (cname.family_name.length() <= 1 || cname.family_name[1] == '.') {
        int len = cname.normalized_name.length();
        string other = cname.normalized_name.substr(len - 2, 2)
            + " " + cname.normalized_name.substr(0, len - 2);
        CName *game_name = new CName(cname.normalized_name.substr(0, len - 2));
        game_names.push_back(game_name);
        game_status.push_back(55);
        cname = CName(other);
      }
    }

    cname.add_family_name();
    if (is_test == false) {
      deal_paper_name();
      get_parters();
    }
    affiliation = string(rows[2]);
    university = AffiliationIndex::get_university(affiliation);
  }

  CAuthor() {
  }

  string get_name() {
    return cname.normalized_name;
  }

  CAuthor(const CAuthor& author) {
    *this = author;
  }

  int get_id() {
    return id;
  }

  SubName &get_names_by_index(unsigned int idx) {
    assert(idx < cname.names.size());
    return cname.names[idx];
  }

  void Print(const char *str = "") {
    printf("%s:%10d:%s\t[%s]\n",
        str, id, cname.ToString().c_str(), university.c_str());
  }

  string ToString() {
    return id + "\t["+cname.ToString();
  }

  bool IsCheckPartners() {
    //return false;
    if (cname.normalized_name.length() < 1) {
      return false;
    }
    return true;
  }

  int GetNameSimilarType(CAuthor *one, int type, int &other_name_type, bool is_print_info) {
    int similar = -1;
    CName *my_cname = type == 0?&cname:game_names[type-1];
    similar = my_cname->GetNameSimilarType(&one->cname);
#ifdef DEBUG
    if (similar == -200) {
      printf("TEST:[%7d] [%7d] [%s] [%s]\n", id, one->id, my_cname->normalized_name.c_str(), one->cname.normalized_name.c_str());
    }
#endif
    if (similar < 0) {
      if (one->get_is_game_name()) {
        for (unsigned int i = 0; i < one->game_names.size(); ++i) {
          similar = my_cname->GetNameSimilarType(one->game_names[i]);
#ifdef DEBUG
          if (similar == -200) {
            printf("TEST:[%7d] [%7d] [%s] [%s]\n", id, one->id, my_cname->normalized_name.c_str(), one->game_names[i]->normalized_name.c_str());
          }
#endif
          if (IS_NAME_NOT_COMPLETE_ABBT_MATCH == similar) {
            if (one->game_status[i] == 17) {
              similar = 0;
            }
          }
          if (similar > 0) {
            similar = SPECIAL_SCORE + one->game_status[i];
            other_name_type = i+1;
#ifdef DEBUG
            if (is_print_info) {
            printf("Ks2:[%4d] [%4d] [%3d] [%d] [%d] [%s] [%s] [%d] [%d]\n",
                similar, this->get_paper_num(), one->get_paper_num(), type, other_name_type,
                my_cname->normalized_name.c_str(), one->game_names[i]->normalized_name.c_str(), id, one->id);
            }
#endif
            break;
          }
        }
      }
    } else if (is_print_info){
#ifdef DEBUG
            printf("Ks1:[%4d] [%3d] [%3d] [%d] [%d] [%s] [%s] [%d] [%d]\n",
                similar, this->get_paper_num(), one->get_paper_num(), type, other_name_type,
                my_cname->normalized_name.c_str(), one->cname.normalized_name.c_str(), id, one->id);
#endif
    }
    if (type > 0 && similar > 0) {
      if (IS_NAME_NOT_COMPLETE_ABBT_MATCH == similar) {
        if (game_status[type-1] == 17) {
          similar = 0;
        }
      }
      if (similar > 0) {
        similar = SPECIAL_SCORE + game_status[type - 1];
      }
    }
    return similar;
  }

  int AddNameCandidates(CAuthor *one, int type) {
    if (name_similar_candidates.find(one->id) != name_similar_candidates.end()
        || paper_num < 1) {
      return -1;
    }
    int my_name_type = type;
    int other_name_type = 0;
    int similar = GetNameSimilarType(one, type, other_name_type, true);
    if (similar == -100 && (is_parter(one) || is_same_university(one))) {
#ifdef DEBUG
      fprintf(stderr, "VK+++[%4d] [%6d] [%s]\n", similar, id, cname.ToString().c_str());
      fprintf(stderr, "VK---[%4d] [%6d] [%s]\n", similar, one->id, one->cname.ToString().c_str());
#endif
      similar = 100;
    }
    if (similar == -200 && (is_parter(one) || is_same_university(one))) {
#ifdef DEBUG
      fprintf(stderr, "TEST+++[%4d]-[%7d]-[%s] :%d:-\n", similar, id, cname.ToString().c_str(), cname.normalized_name.length());
      fprintf(stderr, "TEST---[%4d] [%7d] [%s] [%d]\n", similar, one->id, one->cname.ToString().c_str(), one->cname.normalized_name.length());
#endif
    }
    if (similar > 0) {
      AddSameAuthorCandidate(one, similar, my_name_type, other_name_type);
    }
    return similar;
  }

  void AddSameAuthorCandidate(CAuthor *one, int score, int my_name_type, int other_name_type) {
    if (score > 0) {
      AddSameAuthorCandidate(one->id, score, my_name_type, other_name_type);
      one->AddSameAuthorCandidate(id, score, other_name_type, my_name_type);
#ifdef DEBUG
      fprintf(stderr, "+++[%4d] [%6d] [%s]\n", score, id, cname.ToString().c_str());
      fprintf(stderr, "---[%4d] [%6d] [%s]\n", score, one->id, one->cname.ToString().c_str());
#endif
    }
  }

  bool AddSameAuthorCandidate(int author_id, int score, int my_name_type, int other_name_type) {
    if (ContainCandidate(author_id)) {
      return false;
    }
    name_similar_candidates[author_id] = score;
    my_name_type_map[author_id] = my_name_type;
    other_name_type_map[author_id] = other_name_type;

    Candidate candidate;
    candidate.score = score;
    candidate.author_id = author_id;
    candidate.my_name_type = my_name_type;
    candidate.other_name_type = other_name_type;
    same_author_candidates.push_back(candidate);
    return true;
  }

 public:

  bool is_parter(CAuthor *one) {
    map<int, bool> &one_parters = one->get_parters();
    for (map<int, bool>::iterator beg = one_parters.begin();
        beg != one_parters.end(); ++beg) {
      int parter_author_id = beg->first;
      if (parters.find(parter_author_id) != parters.end()) {
        return true;
      }
    }
    return false;
  }

  void PrintParters(const char *prefix) {
    printf("%s", prefix);
    for (map<int, bool>::iterator beg = parters.begin();
        beg != parters.end(); ++beg) {
      printf(" %d", beg->first);
    }
    printf("\n");
  }

  void ClearParter() {
    parters.clear();
  }

  bool is_same_university(CAuthor *one) {
    AffiliationIndex *affiliation_index = AffiliationIndex::get_instance();
    set<string> &set1 = affiliation_index->get_author_university_by_author_id(id);
    set<string> &set2 = affiliation_index->get_author_university_by_author_id(one->id);
    if (set1.size() == 0 || set2.size() == 0) {
      return false;
    }
    for (set<string>::iterator beg = set1.begin(); beg != set1.end(); ++beg) {
      if (set2.find(*beg) != set2.end()) {
#ifdef DEBUG
        printf("is_same_university:[%s] [%s] [%s] [%d] [%d]\n",
            beg->c_str(), cname.normalized_name.c_str(), one->cname.normalized_name.c_str(), id, one->id);
#endif
        return true;
      }
    }
    return false;
  }

  map<int, bool> &get_parters() {
    if (parters.size() == 0) {
      CPaPerAuthorList *paper_author_list = CPaPerAuthorList::get_instance();
      list<int> *papers = paper_author_list->get_papers_list_by_author(id);
      if (papers == NULL) {
        return parters;
      }
      paper_num = papers->size();
      for (list<int>::iterator papers_it = papers->begin();
          papers_it != papers->end(); ++papers_it) {
        int paperid = *papers_it;
        list<int> *authors =
          paper_author_list->get_authors_list_by_paper(paperid);
        for (list<int>::iterator authors_it = authors->begin();
            authors_it != authors->end(); ++authors_it) {
          parters[*authors_it] = 0;
        }
      }
    }
    return parters;
  }

  int get_paper_num() {
    return paper_num;
  }

  int get_game_name_similar_strict_type(CAuthor *one, CName *c_max_name) {
    LossMiddleName *loss_middle_name = LossMiddleName::get_instance();
    string max_name = c_max_name->normalized_name;
    if (max_name.compare(cname.normalized_name) == 0) {
      return -1;
    }

    if (c_max_name->names.size() == cname.names.size()) {
      if (c_max_name->is_invert_name(&cname)) {
        return 15;
      }
    }
    if (c_max_name->is_ignore_blank_space(&cname)) {
      return 17;
    }
    if (cname.is_invert_merger_name(c_max_name)) {
      return 25;
    }
    
    if (cname.is_first_char_merger(c_max_name)) {
      return 31;
    }
    
    if (cname.normalized_name.length() < 7 || c_max_name->normalized_name.length() < 7
        || cname.names.size() < 2 || c_max_name->names.size() < 2
        || cname.is_abbr
        || c_max_name->is_abbr) {
      return -1;
    }
    if (cname.family_name.find(c_max_name->family_name) != string::npos
        || c_max_name->family_name.find(cname.family_name) != string::npos
        || (c_max_name->get_family_name_people_num() > 10 && cname.get_family_name_people_num() > 10)) {
      if (cname.is_valid_family_name(cname.family_name, c_max_name->family_name) == false) {
        return -1;
      }
    }

    bool is_more_strict = false;
    CPaPerAuthorList *paper_author_list = CPaPerAuthorList::get_instance();
    if (paper_author_list->get_author_name_num(c_max_name->normalized_name) > 10
        && paper_author_list->get_author_name_num(cname.normalized_name) > 10) {
      is_more_strict = true;
    }
    if (university.length() > 3 && one->university.length() > 3 && one->university.compare(university) == 0) {
      is_more_strict = false;
    }

    if ((loss_middle_name->is_valid(&cname)
        && loss_middle_name->is_valid(c_max_name)) || c_max_name->names.size() == cname.names.size()) {
      if (cname.is_lost_middle_name(c_max_name, FROM_COMMON)) {
        return 18;
      }
      string match_string;
      if (cname.is_strict_contain_name(c_max_name, is_more_strict, match_string)
          || c_max_name->is_strict_contain_name(&cname, is_more_strict, match_string)) {
        if (loss_middle_name->is_valid(match_string) || c_max_name->names.size() == cname.names.size()) {
          return 29;
        }
      }
      int status = -1;
      if (c_max_name->is_contain_name(&cname, "STRICT", match_string)) {
        if (loss_middle_name->is_valid(match_string) || c_max_name->names.size() == cname.names.size()) {
          status = 23;
        }
      } else if (cname.is_contain_name(c_max_name, "STRICT", match_string)) {
        if (loss_middle_name->is_valid(match_string) || c_max_name->names.size() == cname.names.size()) {
          status = 24;
        }
      }
      if (status > 0 && (is_parter(one) || is_same_university(one))) {
        return status;
      }
    }

    return -1;
  }

  int _get_game_name_similar_type(CName &c_max_name,
      int game_name_count, int all_count, const char*from) {
    string max_name = c_max_name.normalized_name;
    if (max_name.compare(cname.normalized_name) == 0) {
      return -2;
    }

    if (c_max_name.names.size() == cname.names.size()) {
      if (cname.is_simply_name(&c_max_name)) {
        return 6;
      } else if (c_max_name.is_simply_name(&cname)) {
        return 8;
      } else if (c_max_name.is_invert_name(&cname)) {
        return 15;
      } else if (c_max_name.is_star_name(&cname)) {
        return 16;
      }
    }
    if (c_max_name.is_ignore_blank_space(&cname)) {
      return 17;
    }
    if (strcmp(from, "CG") != 0) {
      if (cname.normalized_name.length() < 7 || c_max_name.normalized_name.length() < 7
          || cname.names.size() < 2 || c_max_name.names.size() < 2
          || cname.is_abbr
          || c_max_name.is_abbr) {
        return -1;
      }
    }
    LossMiddleName *loss_middle_name = LossMiddleName::get_instance();
    int type = c_max_name._GetNameSimilarType(&cname);
    if (type > 0) { //|| type == -100 || type == -200) {
      return 19;
    } else if (cname.is_invert_merger_name(&c_max_name)) {
      return 25;
    } else if (cname.is_star_to_blank(&c_max_name)) {
      return 26;
    } else if (cname.family_name.find(c_max_name.family_name) != string::npos
        || c_max_name.family_name.find(cname.family_name) != string::npos
        || (c_max_name.get_family_name_people_num() > 10 && cname.get_family_name_people_num() > 10)) {
      if (cname.is_valid_family_name(cname.family_name, c_max_name.family_name) == false) {
        return -1;
      }
    }

    if (cname.is_first_char_merger(&c_max_name)) {
      return 31;
    }
    /*
    CPaPerAuthorList *paper_author_list = CPaPerAuthorList::get_instance();
    if (paper_author_list->get_author_name_num(c_max_name.normalized_name) > 10
        && paper_author_list->get_author_name_num(cname.normalized_name) > 10) {
      return -1;
    }
    */

    string match_string;
      if ((loss_middle_name->is_valid(&cname)
          && loss_middle_name->is_valid(&c_max_name))|| c_max_name.names.size() == cname.names.size()
          || strcmp(from, "CG") == 0) {
        if (cname.is_lost_middle_name(&c_max_name, FROM_COMMON)) {
          return 18;
        } else if (c_max_name.is_strict_contain_name(&cname, false, match_string)
            || cname.is_strict_contain_name(&c_max_name, false, match_string)) {
          return 29;
        } else if (c_max_name.is_contain_name(&cname, from, match_string)) {
          return 23;
        } else if (cname.is_contain_name(&c_max_name, from, match_string)) {
          return 24;
        }
      }

    return -1;
  }

  int get_game_name_similar_type(CName &c_max_name, int game_name_count, int all_count, const char*from) {
    int status = _get_game_name_similar_type(c_max_name, game_name_count, all_count, from);
    if (status > 0) {
#ifdef DEBUG
    printf("[YYY_%s_%d]%8d [%s] [%s] [%d] [%d]\n",
        from, status, id, cname.normalized_name.c_str(),
        c_max_name.normalized_name.c_str(),
        game_name_count, all_count);
#endif
    }
    return status;
  }
};

class CAuthorList {
private:
  map<int, CAuthor*> author_map;
  map<string, CMyArray>  simply_name_map;
  vector<int> author_id_list;
  vector<int> only_author_id_list;
  static CAuthorList *_instance;
  CAuthorList() {
    char filename[1024];
    memset(filename, 0, 1024*sizeof(char[0]));
    snprintf(filename, 1024, "%s/Author.csv", DATA_DIR);
    FILE *fin = fopen(filename, "r");
    assert(fin != NULL);

    NameIndex *name_index = NameIndex::get_instance();

    int empty_name = 0;
    int paper_num_zero = 0;
    int university_empty = 0;
    char *line = new char[1024000];
    int max_row_count = 4;
    char **rows = new char*[max_row_count];
    for (int i = 0; i < max_row_count; ++i) {
      rows[i] = new char[1024000];
    }
    fgets(line, 1024000, fin);
    AffiliationIndex *affiliation_index = AffiliationIndex::get_instance();
    while (!feof(fin)) {
      memset(line, 0, 10240);
      fgets(line, 1024000, fin);
      if (feof(fin)) {
        break;
      }
      CAuthor *author = new CAuthor(line, rows, max_row_count);
#ifdef DEBUG_Init_CAuthorList
      printf("+++[%s]\n", line);
      author.Print();
#endif
      author_map[author->get_id()] = author;
      CName *cname = author->get_cname();

      if (cname->normalized_name.length() == 0) {
        ++empty_name;
      }
      if (author->get_university().length() == 0) {
        ++university_empty;
      }
      int paper_num = author->get_paper_num();
      if (paper_num > 0) {
        affiliation_index->add(author->get_university(), author->get_id());
        //printf("AL:[%s]\n", cname->normalized_name.c_str());
        if (simply_name_map.find(cname->simply_name) ==
            simply_name_map.end()) {
          simply_name_map[cname->simply_name] = CMyArray();
        }
        simply_name_map[cname->simply_name].addUnit(author->get_id());
        if (author->get_is_game_name()) {
          vector<CName*> &game_names = author->get_game_names();
          for (unsigned int w = 0; w < game_names.size(); ++w) {
            //printf("AL:[%s]\n", game_names[w]->normalized_name.c_str());
            if (simply_name_map.find(game_names[w]->simply_name)
                == simply_name_map.end()) {
              simply_name_map[game_names[w]->simply_name] = CMyArray();
            }
            simply_name_map[game_names[w]->simply_name].addUnit(author->get_id());
          }
        }
        only_author_id_list.push_back(author->get_id());
      } else {
        ++paper_num_zero;
      }
      author_id_list.push_back(author->get_id());
      vector<SubName> &names = cname->names;
      for (unsigned int i = 0; i < names.size(); ++i) {
        if (names[i].abbr_type == ABBR_TYPE) {
          continue;
        }
        string string_name = names[i].text;
        name_index->add(string_name, author->get_id());
      }
    }
    printf("author_list size:[%d], empty_name:[%d], paper_num_zero:[%d],"
        "university_empty:[%d]\n",
        get_author_map_count(), empty_name, paper_num_zero,
        university_empty);

    delete []line;
    for (int i = 0; i < max_row_count; ++i) {
      delete []rows[i];
    }
    delete []rows;
  }

public:
  static CAuthorList *get_instance() {
    if (_instance == NULL) {
      _instance = new CAuthorList();
    }
    return _instance;
  }

  int get_author_map_count() {
    return author_map.size();
  }

  map<string, CMyArray> *get_simply_name_map() {
    return &simply_name_map;
  }

  vector<int> *get_author_id_list() {
    return &author_id_list;
  }

  vector<int> *get_only_author_id_list() {
    return &only_author_id_list;
  }

  map<int, CAuthor*> *get_author_map() {
    return &author_map;
  }

  CAuthor *get_author_by_id(int author_id) {
    if (author_map.find(author_id) == author_map.end()) {
      return NULL;
    }
    return author_map[author_id];
  }

  void PrintSimplyNameMap() {
    for (map<string, CMyArray>::iterator it = simply_name_map.begin();
        it != simply_name_map.end();
        ++it) {
      printf("%s ", it->first.c_str());
      it->second.Print();
    }
  }

  list<int> *get_same_simply_name_list(string simply_name) {
    if (simply_name_map.find(simply_name) == simply_name_map.end()) {
      return NULL;
    }
    return simply_name_map[simply_name].get_array();
  }

};

#endif
