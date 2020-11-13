#include <stdio.h>
#include <stdlib.h>
#include "../include/parse.h"
#include "../include/handles.h"
#include "../include/openTIDAL.h"

/* Search */

search_model get_search(char *term, size_t limit)
{
  search_model Value;
  char *encodedTerm = url_encode(term);
  char *endpoint = "search/";
  char baseparams[128];
  
  snprintf(baseparams, 128, "countryCode=%s&limit=%zu&query=%s", countryCode,
            limit, encodedTerm);
  curl_model req = curl_get(endpoint, baseparams);
  free(encodedTerm);

  if (req.status != -1)
  {
    cJSON *input_json = json_parse(req.body);
    if (req.responseCode == 200)
    {
      search_model parse = parse_search(input_json);
      free(req.body);
      cJSON_Delete(input_json);
      return parse;
    }
    else
    {
      Value.status = parse_status(input_json, req, 0, "Search");
      cJSON_Delete(input_json);
      free(req.body);
      return Value;
    }
  }
  else
  {
    Value.status = -1;
    free(req.body);
    fprintf(stderr, "[Request Error] CURLE_OK Check failed.\n");
    return Value;
  }
}
