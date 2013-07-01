/**
 *  =============================================================
 *
 *
 *  	  Filename:  main.cpp
 *   Description:  
 *       Created:  05/06/2013 11:27:35 AM
 *       Version:  
 *      Revision:  #1;#4
 *        Author:  liujingming(liujingminghust@163.com)
 *
 *  =============================================================
 **/
#include <stdlib.h>

#include <map>
#include <string>
#include <vector>
#include <list>
#include <set>

#include "./PaperAuthor.h"
#include "./Paper.h"
#include "./Author.h"
#include "./Tom.h"
#define MERGER_SPECIAL_SCORE 666
int cmp(const void *a, const void *b) {
  return (reinterpret_cast<const Candidate *>(b))->score
    - (reinterpret_cast<const Candidate *>(a))->score;
}

CPaPerAuthorList *CPaPerAuthorList::_instance = NULL;
CPaperList *CPaperList::_instance = NULL;
CAuthorList *CAuthorList::_instance = NULL;
map<string, int> CName::family_name_map;
map<string, vector<CName*> > CName::global_similar_author;
map<string, set<string> > CName::family_name_count_map;
map<string, set<string> > CName::family_name_first_name_map;
map<string, map<string, int> > CName::similar_family_name_map;

LossMiddleName *LossMiddleName::_instance = NULL;
NameIndex *NameIndex::_instance = NULL;
AffiliationIndex *AffiliationIndex::_instance = NULL;
Tom *Tom::_instance = NULL;

char DATA_DIR[1024];

int get_candidates_num(vector<int> *author_id_list, map<int, CAuthor*> *author_map);
void delete_candidate(const vector<Candidate> &same_author_candidates,
    CAuthor &author, int name_type);

void merge_result(vector<int> *author_id_list, map<int, CAuthor*> *author_map) {
  // delete the invalid data
  int sum0 = get_candidates_num(author_id_list, author_map);
  for (unsigned int i = 0; i < author_id_list->size(); ++i) {
    int author_id = (*author_id_list)[i];
    CAuthor *author = (*author_map)[author_id];
    vector<Candidate> &candidates = author->get_same_author_candidates();
    delete_candidate(candidates, *author, 0);
    if (author->get_is_game_name()) {
      vector<CName*> game_names = author->get_game_names();
      for (unsigned int k = 0; k < game_names.size(); ++k) {
        delete_candidate(candidates, *author, 1+k);
      }
    }
  }
  int merger_count = 0;
  int sum1 = get_candidates_num(author_id_list, author_map);

  // merger
  for (unsigned int i = 0; i < author_id_list->size(); ++i) {
    int author_id = (*author_id_list)[i];
    CAuthor *author = (*author_map)[author_id];
    vector<Candidate> &candidates = author->get_same_author_candidates();
    int before_size = candidates.size();
    for (unsigned int j = 0; j < candidates.size (); ++j) {
      CAuthor *one = (*author_map)[candidates[j].author_id];
      vector<Candidate> &one_candidates = one->get_same_author_candidates();
      for (unsigned int k = 0; k < one_candidates.size (); ++k) {
        bool status = author->AddSameAuthorCandidate (one_candidates[k].author_id, MERGER_SPECIAL_SCORE, -1, -1);
        if (status == true) {
          ++merger_count;
        }
      }
    }
    int after_size = candidates.size();
#ifdef DEBUG
    if (before_size != after_size) {
      printf("%d [%d]-->[%d]\n", author_id, before_size, after_size);
    }
#endif
  }
  int sum2 = get_candidates_num(author_id_list, author_map);
  printf("merge_result count:[%5d] sum0:[%5d] sum1:[%5d] sum2[%5d]\n",
      merger_count, sum0, sum1, sum2);
}

void delete_candidate(const vector<Candidate> &same_author_candidates,
    CAuthor &author, int name_type) {
  if (author.get_name().length() == 0) {
    return;
  }
  CAuthorList *author_list = CAuthorList::get_instance();
  map<int, CAuthor*> *author_map = author_list->get_author_map();
  set<int> waiting_for_delete;

  vector<SubName> &names = author.get_names(name_type);
  for (unsigned int k = 0; k < names.size(); ++k){ 
    SubName &subname = names[k];
    if (subname.abbr_type != ABBR_TYPE) {
      continue;
    }

    for (unsigned int i = 0; i < same_author_candidates.size(); ++i) {
      int my_name_type_1 = same_author_candidates[i].my_name_type;
      if (my_name_type_1 != name_type) {
        continue;
      }

      int author_id_1 = same_author_candidates[i].author_id;
      int other_name_type_1 = same_author_candidates[i].other_name_type;

      CAuthor *author_1 = (*author_map)[author_id_1];
      vector<SubName> &names_1 = author_1->get_names(other_name_type_1);
      SubName &subname_1 = names_1[k];
      for (unsigned int j = i + 1; j < same_author_candidates.size(); ++j) {
        int my_name_type_2 = same_author_candidates[j].my_name_type;
        if (my_name_type_2 != name_type) {
          continue;
        }

        int author_id_2 = same_author_candidates[j].author_id;
        int other_name_type_2 = same_author_candidates[j].other_name_type;
        CAuthor *author_2 = (*author_map)[author_id_2];
        vector<SubName> &names_2 = author_2->get_names(other_name_type_2);
        SubName &subname_2 = names_2[k];
        if (Common::is_not_valid_string(subname_1.text, subname_2.text)) {
          waiting_for_delete.insert(author_id_1);
          waiting_for_delete.insert(author_id_2);
        }
      }
    }
  }
  if (waiting_for_delete.size() == 0) {
    return;
  }

#ifdef DEBUG
  author.Print("[[[");
#endif
  for (set<int>::iterator it1 = waiting_for_delete.begin();
      it1 != waiting_for_delete.end(); ++it1) {
    int author_id_1 = *it1;
    CAuthor *author_1 = (*author_map)[author_id_1];
#ifdef DEBUG
    author_1->Print("]]]");
#endif

    author.delete_candidate(author_id_1);
    author_1->delete_candidate(author.get_id());
  }
}

void valid(const vector<Candidate> &same_author_candidates,
    CAuthor &author, int name_type) {
  if (author.get_name().length() == 0) {
    return;
  }
  CAuthorList *author_list = CAuthorList::get_instance();
  map<int, CAuthor*> *author_map = author_list->get_author_map();

  for (unsigned int i = 0; i < same_author_candidates.size(); ++i) {
    int my_name_type_1 = same_author_candidates[i].my_name_type;
    if (my_name_type_1 != name_type
        || MERGER_SPECIAL_SCORE == same_author_candidates[i].score) {
      continue;
    }

    int author_id_1 = same_author_candidates[i].author_id;

    int other_name_type_1 = same_author_candidates[i].other_name_type;
    CAuthor *author_1 = (*author_map)[author_id_1];
    for (unsigned int j = i + 1; j < same_author_candidates.size(); ++j) {
      int my_name_type_2 = same_author_candidates[j].my_name_type;
      if (my_name_type_2 != name_type
          || MERGER_SPECIAL_SCORE == same_author_candidates[j].score) {
        continue;
      }

      int author_id_2 = same_author_candidates[j].author_id;
      CAuthor *author_2 = (*author_map)[author_id_2];
      int other_name_type_valid;
      int similar = author_1->GetNameSimilarType(author_2, other_name_type_1, other_name_type_valid, false);
      if (similar < 0) {
#ifdef DEBUG
        author_1->Print("V+");
        author_2->Print("V-");
        author.Print("V---->");
#endif
      }
    }
  }
}

int test() {
  CAuthorList *author_list = CAuthorList::get_instance();
  map<int, CAuthor*> *author_map = author_list->get_author_map();
  int id1;
  int id2;
  while (true) {
    scanf("%d %d", &id1, &id2);
    if (id1 == -1) {
      break;
    }
    CAuthor *author_1 = (*author_map)[id1];
    CAuthor *author_2 = (*author_map)[id2];
    int type = 0;
    int similar = author_1->GetNameSimilarType(author_2, 1, type, true);
    author_1->AddNameCandidates(author_2, 1);
    author_1->Print("+++");
    author_2->Print("---");
    printf("test:%d\n", similar);
  }
  return 1;
}

int get_candidates_num(vector<int> *author_id_list, map<int, CAuthor*> *author_map) {
  int sum = 0;
  int count[55];
  memset(count, 0, sizeof(count[0])*55);
  for (unsigned int i = 0; i < author_id_list->size(); ++i) {
    int author_id = (*author_id_list)[i];
    CAuthor *author = (*author_map)[author_id];
    vector<Candidate> &candidates = author->get_same_author_candidates();
    if (candidates.size() > 10) { 
#ifdef DEBUG
      printf("get_candidates_num TOOLARGER:[%d] [%d]\n", author_id, (int)candidates.size());
      author->Print("+++");
      for (unsigned int j = 0; j < candidates.size(); ++j) {
        CAuthor *author1 = (*author_map)[candidates[j].author_id];
        author1->Print("---");
      }
#endif
    } else {
      count[candidates.size()]++;
    }
    sum += candidates.size();
  }
  for (int i = 0; i < 50; ++i) {
    if (count[i] > 0) {
      printf("get_candidates_num_count:%d:[%d]\n", i, count[i]);
    }
  }
  return sum;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("%s data_dir\n", argv[0]);
    exit(-1);
  }
  memset(DATA_DIR, 0, 1024 * sizeof(DATA_DIR[1024]));
  snprintf(DATA_DIR, 1024, "%s", argv[1]);
  if (access(DATA_DIR, 0) != 0) {
    printf("%s isnot exist\n", DATA_DIR);
    printf("%s data_dir\n", argv[0]);
    exit(-1);
  }

  LossMiddleName *loss_middle_name = LossMiddleName::get_instance();
  CAuthorList *author_list = CAuthorList::get_instance();
  CPaPerAuthorList *paper_author_list = CPaPerAuthorList::get_instance();
  CPaperList *paper_list = CPaperList::get_instance();
#ifdef DEBUG
  paper_author_list->print_name_count();
#endif

  vector<int> *author_id_list = author_list->get_author_id_list();
  map<int, CAuthor*> *author_map = author_list->get_author_map();
  map<int, vector<Candidate> > *author_similar_map;
#ifdef DEBUGTEST
  test();
#endif

  // 1. valid the lost middle name type
  vector<int> *only_author_id_list = author_list->get_only_author_id_list();
  NameIndex *name_index = NameIndex::get_instance();
  // 1.1 find the middle name by using the author name
  for (unsigned int i = 0; i < only_author_id_list->size(); ++i) {
    int author_id1 = (*only_author_id_list)[i];
    CAuthor *author1 = author_list->get_author_by_id(author_id1);
    if (author1 == NULL) {
      continue;
    }
    author1->add_is_loss_middle_name();
    if (author1->get_cname()->is_abbr && author1->get_names_size() < 4) {
      continue;
    }
    vector<SubName> &names = author1->get_cname()->names;
    for (unsigned int i = 0; i < names.size(); ++i) {
      if (names[i].abbr_type == ABBR_TYPE) {
        continue;
      }
      string string_name = names[i].text;
      list<int> *name_index_by_name =
        name_index->get_name_index(string_name);
      if (name_index_by_name == NULL) {
        continue;
      }
      for (list<int>::iterator beg = name_index_by_name->begin();
          beg != name_index_by_name->end();
          ++beg) {
        int author_id2 = *beg;
        CAuthor *author2 = author_list->get_author_by_id(author_id2);
        if (author2 == NULL) {
          continue;
        }
        if (author1->get_cname()->is_lost_middle_name(author2->get_cname(), FROM_CG)) {
          loss_middle_name->add(author1->get_cname(), author2->get_cname());
        }
      }
    }
  }

  // 1.2 find the middle name by using the game name
  for (unsigned int i = 0; i < only_author_id_list->size(); ++i) {
    int author_id1 = (*only_author_id_list)[i];
    CAuthor *author1 = author_list->get_author_by_id(author_id1);
    vector<CName*> &game_names = author1->get_mypaper_name_list();
    for (unsigned int m = 0; m < game_names.size(); ++m) {
      CName* game_name = game_names[m];
      if (game_name->is_abbr && game_name->names.size() < 4) {
        continue;
      }
      vector<SubName> &names = game_name->names;
      for (unsigned int i = 0; i < names.size(); ++i) {
        if (names[i].abbr_type == ABBR_TYPE) {
          continue;
        }
        string string_name = names[i].text;
        list<int> *name_index_by_name =
          name_index->get_name_index(string_name);
        if (name_index_by_name == NULL) {
          continue;
        }
        for (list<int>::iterator beg = name_index_by_name->begin();
            beg != name_index_by_name->end();
            ++beg) {
          int author_id2 = *beg;
          CAuthor *author2 = author_list->get_author_by_id(author_id2);
          /*
          if (author2->get_cname()->_GetNameSimilarType(game_name) > 0) {
            continue;
          }
          */
          if (author2->get_cname()->is_lost_middle_name(game_name, FROM_CG)) {
            loss_middle_name->add(game_name, author2->get_cname());
          }
        }
      }
    }
  }
  // 1.3 valid
  loss_middle_name->valid();

  // 2. create game_name
  for (unsigned int i = 0; i < author_id_list->size(); ++i) {
    int author_id = (*author_id_list)[i];
    CAuthor *author = author_list->get_author_by_id(author_id);
    if (author == NULL) {
      continue;
    }
    author->create_game_name();
  }

  // 3. find the similar author
  // 3.1 use the simply name of the author name
  for (unsigned int i = 0; i < author_id_list->size(); ++i) {
    int author_id = (*author_id_list)[i];

    CAuthor *author = (*author_map)[author_id];
    if (author->get_paper_num() == 0) {
      continue;
    }
    string simply_name = author->get_simply_name();
    if (simply_name.length() > 1) {
      list<int> *same_simply_name_list =
        author_list->get_same_simply_name_list(simply_name);
      if (same_simply_name_list == NULL) {
        continue;
      }
      for (list<int>::iterator same_simplys_it = same_simply_name_list->begin();
          same_simplys_it != same_simply_name_list->end();
          ++same_simplys_it) {
        CAuthor *one = (*author_map)[*same_simplys_it];
        if (author_id == *same_simplys_it) {
          continue;
        }
        author->AddNameCandidates(one, 0);
      }
    }
  }
  printf("CANDIDATE using author name[%d]\n", get_candidates_num(author_id_list, author_map));

  // 3.2 using the simply name of the game name
  for (unsigned int i = 0; i < author_id_list->size(); ++i) {
    int author_id = (*author_id_list)[i];

    CAuthor *author = (*author_map)[author_id];
    if (author->get_paper_num() == 0) {
      continue;
    }
    if (author->get_is_game_name()) {
      vector<CName*> &game_names = author->get_game_names();
      for (unsigned int k = 0; k < game_names.size(); ++k) {
        list<int> *same_simply_name_list
          = author_list->get_same_simply_name_list(game_names[k]->simply_name);
        if (same_simply_name_list == NULL) {
          continue;
        }
        for (list<int>::iterator same_simplys_it = same_simply_name_list->begin();
            same_simplys_it != same_simply_name_list->end();
            ++same_simplys_it) {
          CAuthor *one = (*author_map)[*same_simplys_it];
          if (author_id == *same_simplys_it) {
            continue;
          }
          if (author->get_cname()->names.size() > game_names[k]->names.size()
              && one->get_cname()->names.size() > game_names[k]->names.size()) {
            continue;
          }
          int similar = author->AddNameCandidates(one, k+1);
#ifdef DEBUG
          if (similar > 0) {
            printf("GGGGG:[%s] [%d]\n",
                game_names[k]->normalized_name.c_str(), author->get_paper_num());
          }
#endif
        }
      }
    }
  }

  printf("CANDIDATE using game name[%d]\n", get_candidates_num(author_id_list, author_map));

  // 3.3 using the subname of the author name
  for (unsigned int i = 0; i < only_author_id_list->size(); ++i) {
    int author_id1 = (*only_author_id_list)[i];
    CAuthor *author1 = author_list->get_author_by_id(author_id1);
    if (author1->get_cname()->is_abbr && author1->get_names_size() < 4) {
      continue;
    }
    vector<SubName> &names = author1->get_cname()->names;
    for (unsigned int i = 0; i < names.size(); ++i) {
      if (names[i].abbr_type == ABBR_TYPE) {
        continue;
      }
      string string_name = names[i].text;
      list<int> *name_index_by_name =
        name_index->get_name_index(string_name);
      if (name_index_by_name == NULL) {
        continue;
      }
      for (list<int>::iterator beg = name_index_by_name->begin();
          beg != name_index_by_name->end();
          ++beg) {
        int author_id2 = *beg;
        if (author_id1 > author_id2) {
          continue;
        }
        CAuthor *author2 = author_list->get_author_by_id(author_id2);
        if (author2->get_paper_num() < 1) {
          continue;
        }
        if (author1->ContainCandidate(author_id2)) {
          continue;
        }
        if ((author2->get_cname()->is_abbr == false || author2->get_names_size() > 3)) {
          int type = author1->get_game_name_similar_strict_type(author2, author2->get_cname());
          if (type < 0) {
            continue;
          }
          author1->AddSameAuthorCandidate(author2, MERGER_SPECIAL_SCORE, -1, -1);
#ifdef DEBUG
          printf("XXX [%d] [%s] [%s]\n", type, author1->get_cname()->normalized_name.c_str(), author2->get_cname()->normalized_name.c_str());
#endif
        }
      }
    }
  }
  printf("CANDIDATE using name index name[%d]\n", get_candidates_num(author_id_list, author_map));
  
  int merger_count = 1;
  int times = 0;
  // 4 check and merge result
  merge_result(author_id_list, author_map);

  // 5 write result
  FILE *fout = fopen("result.csv", "w");
#ifdef DEBUG
  memset(file_name_out, 0, sizeof(file_name_out[0])*128);
  sprintf(file_name_out, "%s/%s", DIR, "result.info");
  FILE *fout_info = fopen(file_name_out, "w");
#endif
  int candidates_sum = 0;
  int author_sum = 0;
  int transfer = 0;
  for (unsigned int i = 0; i < author_id_list->size(); ++i) {
    int author_id = (*author_id_list)[i];
    CAuthor *author = (*author_map)[author_id];
    vector<Candidate> &candidates = author->get_same_author_candidates();
    /*
      if (author->get_is_game_name()) {
        vector<CName*> game_names = author->get_game_names();
        for (unsigned int k = 0; k < game_names.size(); ++k) {
          delete_candidate(candidates, *author, 1+k);
        }
      }

      if (author->get_is_game_name()) {
        vector<CName*> game_names = author->get_game_names();
        for (unsigned int k = 0; k < game_names.size(); ++k) {
          valid(candidates, *author, 1+k);
        }
      }
      */
    if (candidates.size() > 1) {
      qsort(&candidates[0], candidates.size(), sizeof(Candidate), cmp);
    }
    fprintf(fout, "%d,%d", author_id, author_id);

    int diff = 0;
    for (unsigned int j = 0; j < candidates.size(); ++j) {
        fprintf(fout, " %d", candidates[j].author_id);
        CAuthor *one = (*author_map)[candidates[j].author_id];
        if (candidates.size() > one->get_same_author_candidates().size()) {
          diff = one->get_same_author_candidates().size() - candidates.size();
        }
    }
    if (candidates.size() > 0) {
#ifdef DEBUG
      fprintf(fout_info, "++:%d %s\n", author_id, author->get_cname()->ToString().c_str());
      for (unsigned int j = 0; j < candidates.size(); ++j) {
        CAuthor *one = (*author_map)[candidates[j].author_id];
        fprintf(fout_info, "---:%d\t%s\n", candidates[j].author_id, one->get_cname()->ToString().c_str());
      }
#endif
      ++author_sum;
    }

#ifdef DEBUG
    if (candidates.size() > 20) {
      printf("SOMUCH\n");
      for (unsigned int j = 0; j < candidates.size(); ++j) {
        CAuthor *one = (*author_map)[candidates[j].author_id];
        one->Print();
      }
    }
#endif
    transfer += diff;
    fprintf(fout, "\n");
  }
  int get_candidates_num_end = get_candidates_num(author_id_list, author_map);
  printf("candidates_sum:[%d] author_sum:[%d] transfer:[%d]\n",
      get_candidates_num_end, author_sum, transfer);
}
