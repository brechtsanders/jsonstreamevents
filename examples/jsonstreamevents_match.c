#include "jsonstreamevents.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int json_on_value (struct jsonstreamevents_status_struct* parserdata, enum parser_type_enum type, const void* value, size_t valuelen, void* userdata)
{
  //look for properties
  if (jsonstreamevents_parent_matches(parserdata, NULL, "owner") == 0) {
    //owner object
    //match full path to the top
    if (jsonstreamevents_property_matches(parserdata, NULL, "owner", "login") == 0)
      printf("owner login: %.*s\n", (int)valuelen, (char*)value);
  } else if (jsonstreamevents_parent_matches(parserdata, NULL, "license") == 0) {
    //license object
    //match only propery name
    if (jsonstreamevents_property_matches(parserdata, "name") == 0)
      printf("license description: %.*s\n", (int)valuelen, (char*)value);
    else if (jsonstreamevents_property_matches(parserdata, "spdx_id") == 0)
      printf("license type: %.*s\n", (int)valuelen, (char*)value);
  } else {
    if (jsonstreamevents_property_matches(parserdata, NULL, "name") == 0)
      printf("name: %.*s\n", (int)valuelen, (char*)value);
    else if (jsonstreamevents_property_matches(parserdata, NULL, "homepage") == 0)
      printf("homepage: %.*s\n", (int)valuelen, (char*)value);
    else if (jsonstreamevents_property_matches(parserdata, NULL, "html_url") == 0)
      printf("link: %.*s\n", (int)valuelen, (char*)value);
    else if (jsonstreamevents_property_matches(parserdata, NULL, "description") == 0)
      printf("description: %.*s\n", (int)valuelen, (char*)value);
    else if (jsonstreamevents_property_matches(parserdata, NULL, "releases_url") == 0)
      printf("releases URL: %.*s\n", (int)valuelen, (char*)value);
    else if (jsonstreamevents_property_matches(parserdata, NULL, "tags_url") == 0)
      printf("tags URL: %.*s\n", (int)valuelen, (char*)value);
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
