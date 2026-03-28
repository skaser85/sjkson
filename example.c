#define SJKJSON_IMPLEMENTATION
#include "sjkjson.h"

JSON_Element* test_make_json() {
  Roots roots = {0};

  JSON_Element* r = make_json_array("");
  JSON_Elements* root = push_root(&roots, r->value.jarray);

  JSON_Element* o1 = make_json_object("");
  append_to_json(root, o1);
  root = push_root(&roots, o1->value.jobject);

  append_to_json(root, make_json_string("name", "France"));
  append_to_json(root, make_json_string("capital", "Paris"));
  append_to_json(root, make_json_num("population", 67364357));
  append_to_json(root,  make_json_num("area", 551695));
  append_to_json(root, make_json_string("currency", "Euro"));
  JSON_Element* lang = make_json_array("languages");
  append_to_json(root, lang);
  root = push_root(&roots, lang->value.jarray);
  append_to_json(root, make_json_string("", "French")); 
  root = pull_root(&roots);
  append_to_json(root, make_json_string("region", "Europe"));
  append_to_json(root, make_json_string("subregion", "Western Europe"));
  append_to_json(root, make_json_string("subregion", "https://upload.wikimedia.org/wikipedia/commons/c/c3/Flag_of_France.svg"));

  root = pull_root(&roots);

  return r;
}

int main(void) {
  //const char* src_file_path = "./products.json";
  
  //JSON_Element* json = parse_json_file(src_file_path); 
  
  JSON_Element* json = test_make_json();
  
  String_Builder sb = {0}; 
  dump_json(&sb, json, 0);

  write_sb_to_file("./test.json", sb);

  return 0;
}
