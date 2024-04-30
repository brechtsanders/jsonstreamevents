#include "jsonstreamevents.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct user_data_struct {
  unsigned int indent;
};

int json_on_value (struct jsonstreamevents_status_struct* parserdata, enum parser_type_enum type, const void* value, size_t valuelen, void* userdata)
{
  printf("%*s%s:%s=", ((struct user_data_struct*)userdata)->indent * 2, "", (parserdata->property ? parserdata->property : "(no name)"), parser_type_get_name(type));
  switch (type) {
    case type_null:    printf("(null)"); break;
    case type_text:    printf("%.*s", (int)valuelen, (char*)value); break;
    case type_number:  printf("%g", *(double*)value); break;
    case type_boolean: printf("%s", (valuelen ? "true" : "false")); break;
    default:           break;
  }
  printf("\n");
/*
  //if (jsonstreamevents_parent_matches(parserdata, NULL, "owner") == 0)
  if (jsonstreamevents_property_matches(parserdata, NULL, "owner", "login") == 0)
    printf("_");
*/
  //look for properties
  if (parserdata->parent && parserdata->property && (type == type_text || type == type_null)) {
    //first level from the top
    if (!parserdata->parent->parent /*&& parserdata->parent->name == NULL*/) {
      //first level from the top
      if (strcmp(parserdata->property, "name") == 0)
        printf("*name\n");
      else if (strcmp(parserdata->property, "homepage") == 0)
        printf("*homepage\n");
      else if (strcmp(parserdata->property, "html_url") == 0)
        printf("*link\n");
      else if (strcmp(parserdata->property, "description") == 0)
        printf("*description\n");
      else if (strcmp(parserdata->property, "releases_url") == 0)
        printf("*releases URL\n");
      else if (strcmp(parserdata->property, "tags_url") == 0)
        printf("*tags URL\n");
    } else if (parserdata->parent->parent && !parserdata->parent->parent->parent /*&& parserdata->parent->parent->name == NULL*/) {
      //second level from the top
      if (parserdata->parent->name) {
        //parent has a name
        if (strcmp(parserdata->parent->name, "owner") == 0) {
          if (strcmp(parserdata->property, "login") == 0)
            printf("*owner login\n");
        } else if (strcmp(parserdata->parent->name, "license") == 0) {
          if (strcmp(parserdata->property, "name") == 0)
            printf("*license description\n");
          else if (strcmp(parserdata->property, "spdx_id") == 0)
            printf("*license type\n");
        }
      } else if (parserdata->parent->parent->isarray) {
        //parent has no name and is an array entry
        if (strcmp(parserdata->property, "name") == 0)
          printf("*file base name\n");
        else if (strcmp(parserdata->property, "path") == 0)
          printf("*file path\n");
      }
    }
  }
  return 0;
}

int json_on_begin (struct jsonstreamevents_status_struct* parserdata, int isarray, void* userdata)
{
  printf("%*sbegin %s %s\n", ((struct user_data_struct*)userdata)->indent * 2, "", (isarray ? "array" : "object"), (parserdata->property ? parserdata->property : "(no name)"));
  ((struct user_data_struct*)userdata)->indent++;
  return 0;
}

int json_on_end (struct jsonstreamevents_status_struct* parserdata, int isarray, void* userdata)
{
  ((struct user_data_struct*)userdata)->indent--;
  printf("%*send %s %s\n", ((struct user_data_struct*)userdata)->indent * 2, "", (isarray ? "array" : "object"), (parserdata->property ? parserdata->property : "(no name)"));
  return 0;
}

int main ()
{
  char buf[1024];
  FILE* f;
  int len;
  struct user_data_struct userdata;
  jsonstreamevents_handle handle;
  const char* errmsg;

  //initialize user data
  userdata.indent = 0;

  //open handle
  handle = jsonstreamevents_create_parser(json_on_value, json_on_begin, json_on_end, &userdata);

  //read and parse data from file
  if ((f = fopen("testdata/projectfiles.json", "rb")) != NULL) {
  //if ((f = fopen("testdata/projectinfo.json", "rb")) != NULL) {
    while ((len = fread(buf, 1, sizeof(buf), f))) {
      if ((errmsg = jsonstreamevents_parse_data(handle, buf, len)) != NULL)
        break;
    }
    fclose(f);
    if (!errmsg)
      errmsg = jsonstreamevents_parse_done(handle);
    if (errmsg) {
      fprintf(stderr, "%s", errmsg);
    }
  }

  //close handle
  jsonstreamevents_destroy_parser(handle);

  return 0;
}
