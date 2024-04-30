#include "jsonstreamevents.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int json_on_value (struct jsonstreamevents_status_struct* parserdata, enum parser_type_enum type, const void* value, size_t valuelen, void* userdata)
{
  //look for properties
  if (parserdata->parent && parserdata->property && (type == type_text || type == type_null)) {
    //first level from the top
    if (!parserdata->parent->parent /*&& parserdata->parent->name == NULL*/) {
      //first level from the top
      if (strcmp(parserdata->property, "name") == 0)
        printf("name: %.*s\n", (int)valuelen, (char*)value);
      else if (strcmp(parserdata->property, "homepage") == 0)
        printf("homepage: %.*s\n", (int)valuelen, (char*)value);
      else if (strcmp(parserdata->property, "html_url") == 0)
        printf("link: %.*s\n", (int)valuelen, (char*)value);
      else if (strcmp(parserdata->property, "description") == 0)
        printf("description: %.*s\n", (int)valuelen, (char*)value);
      else if (strcmp(parserdata->property, "releases_url") == 0)
        printf("releases URL: %.*s\n", (int)valuelen, (char*)value);
      else if (strcmp(parserdata->property, "tags_url") == 0)
        printf("tags URL: %.*s\n", (int)valuelen, (char*)value);
    } else if (parserdata->parent->parent && !parserdata->parent->parent->parent /*&& parserdata->parent->parent->name == NULL*/) {
      //second level from the top
      if (parserdata->parent->name) {
        //parent has a name
        if (strcmp(parserdata->parent->name, "owner") == 0) {
          if (strcmp(parserdata->property, "login") == 0)
            printf("owner login: %.*s\n", (int)valuelen, (char*)value);
        } else if (strcmp(parserdata->parent->name, "license") == 0) {
          if (strcmp(parserdata->property, "name") == 0)
            printf("license description: %.*s\n", (int)valuelen, (char*)value);
          else if (strcmp(parserdata->property, "spdx_id") == 0)
            printf("license type: %.*s\n", (int)valuelen, (char*)value);
        }
      } else if (parserdata->parent->parent->isarray) {
        //parent has no name and is an array entry
        if (strcmp(parserdata->property, "name") == 0)
          printf("file base name: %.*s\n", (int)valuelen, (char*)value);
        else if (strcmp(parserdata->property, "path") == 0)
          printf("file path: %.*s\n", (int)valuelen, (char*)value);
      }
    }
  }
  return 0;
}

int main ()
{
  char buf[1024];
  FILE* f;
  int len;
  jsonstreamevents_handle handle;
  const char* errmsg;

  //open handle
  handle = jsonstreamevents_create_parser(json_on_value, NULL, NULL, NULL);

  //read and parse data from file
  //if ((f = fopen("testdata/projectfiles.json", "rb")) != NULL) {
  if ((f = fopen("testdata/projectinfo.json", "rb")) != NULL) {
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
