#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/parse.h"
#include "../include/handles.h"
#include "../include/openTIDAL.h"

playlist_model get_playlist(char *playlistid)
{
  playlist_model Value;
  char *endpoint;
  char baseparams[20];
  
  endpoint = url_cat_str("playlists/", playlistid, "");
  snprintf(baseparams, 20, "countryCode=%s", countryCode);
  curl_model req = curl_get(endpoint, baseparams);
  free(endpoint);
  
  if (req.status != -1)
  {
    cJSON *input_json = json_parse(req.body);
    if (req.responseCode == 200)
    {
      json_playlist_model processed_json = json_parse_playlist(input_json);
      Value = parse_playlist_values(processed_json, 0); 
      Value.status = 1;
      Value.arraySize = 1;

      /* Cleanup  */
      cJSON_Delete(input_json);
      free(req.body);
      return Value;
    }
    else
    {
      Value.status = parse_status(input_json, req, 0, playlistid);
      /* Cleanup  */
      cJSON_Delete(input_json);
      free(req.body);
      return Value;
    }
  }
  else
  {
    free(req.body);
    Value.status = -1;
    fprintf(stderr, "[Request Error] Playlist %s: CURLE_OK Check failed.\n", playlistid);
    return Value;
  }
}

items_model get_playlist_items(char *playlistid, size_t limit, size_t offset)
{
  items_model Value;
  char *endpoint;
  char baseparams[50];
  
  endpoint = url_cat_str("playlists/", playlistid, "/items");
  snprintf(baseparams, 50, "countryCode=%s&limit%zu&offset=%zu", countryCode,
            limit, offset);
  curl_model req = curl_get(endpoint, baseparams);
  free(endpoint);
  
  if (req.status != -1)
  {
    cJSON *input_json = json_parse(req.body);
    if (req.responseCode == 200)
    {
      cJSON *limit = cJSON_GetObjectItem(input_json, "limit");
      cJSON *offset = cJSON_GetObjectItem(input_json, "offset");
      cJSON *totalNumberOfItems = cJSON_GetObjectItem(input_json, "totalNumberOfItems");
      cJSON *items = cJSON_GetObjectItem(input_json, "items");
      cJSON *item = NULL;
      size_t i = 0;
            
      if (cJSON_IsArray(items))
      {
	cJSON_ArrayForEach(item, items)
        {
          cJSON *innerItem = cJSON_GetObjectItem(item, "item");
          json_items_model processed_json = json_parse_items(innerItem);
	  Value = parse_items_values(processed_json, i);
	  i += 1;
	}
      }
 
      parse_number(limit, &Value.limit); 
      parse_number(offset, &Value.offset);
      parse_number(totalNumberOfItems, &Value.totalNumberOfItems);
      Value.status = 1;
      Value.arraySize = cJSON_GetArraySize(items); 
      
      /* Cleanup  */
      cJSON_Delete(input_json);
      free(req.body);
      return Value;
    }
    else
    {
      Value.status = parse_status(input_json, req, 0, playlistid);
      /* Cleanup  */
      cJSON_Delete(input_json);
      free(req.body);
      return Value;
    }
  }
  else
  {
    free(req.body);
    Value.status = -1;
    fprintf(stderr, "[Request Error] Playlist %s: CURLE_OK Check failed.\n", playlistid);
    return Value;
  }
}

playlist_model
get_user_playlist(size_t limit, size_t offset, char *order, char *orderDirection)
{
  playlist_model Value;
  char *endpoint = url_cat("users/", userId, "/playlistsAndFavoritePlaylists", 0);
  
  char baseparams[150];
  snprintf(baseparams, 150, "countryCode=%s&limit=%zu&offset%zu&order%s&orderDirection=%s",
             countryCode, limit, offset, order, orderDirection);

  curl_model req = curl_get(endpoint, baseparams);
  free(endpoint);
  if (req.status != -1)
  {
    cJSON *input_json = json_parse(req.body);
    if (req.responseCode == 200)
    {
      cJSON *items = cJSON_GetObjectItem(input_json, "items");
      cJSON *item = NULL;
      cJSON *limit = cJSON_GetObjectItem(input_json, "limit");
      cJSON *offset = cJSON_GetObjectItem(input_json, "offset");
      cJSON *totalNumberOfItems = cJSON_GetObjectItem(input_json, "totalNumberOfItems");

      size_t i = 0;

      if (cJSON_IsArray(items))
      {
        cJSON_ArrayForEach(item, items)
        {
          cJSON *innerItem = cJSON_GetObjectItem(item, "playlist");

          json_playlist_model processed_json = json_parse_playlist(innerItem);
          Value = parse_playlist_values(processed_json, i);
  	  i += 1;
        }
      }
      
      parse_number(limit, &Value.limit);
      parse_number(offset, &Value.offset);
      parse_number(totalNumberOfItems, &Value.totalNumberOfItems);
      Value.arraySize = cJSON_GetArraySize(items);
      Value.status = 1;
 
      cJSON_Delete(input_json);
      free(req.body);
      return Value;
    }
    else
    {
      Value.status = parse_status(input_json, req, userId, NULL);
      free(req.body);
      cJSON_Delete(input_json);
      return Value;
    }
  }
else
  {
    Value.status = -1;
    free(req.body);
    fprintf(stderr, "[Request Error] User %zu: CURLE_OK Check failed.", userId);
    return Value;
  }
}


char *get_playlist_etag(char *playlistid)
{
  /* Request playlist endpoint to scrape eTag Header  */
  char *endpoint;
  char baseparams[20];
  char *eTagHeader = malloc(30);

  endpoint = url_cat_str("playlists/", playlistid, "");
  snprintf(baseparams, 20, "countryCode=%s", countryCode);
  curl_model req = curl_head(endpoint, baseparams); /* Returns Header with eTag */
  free(endpoint);
  
  if (req.status != -1)
  {
    if (req.responseCode == 200)
    {
      size_t i = 0; /* Counter for Buffer (Header) Splitter  */
      size_t e; /* Counter for ETag-String Extraction  */
      char *p = strtok (req.header, "\n");
      char eTag[20];
      /* Buffer for splitted HTTP-Header  */ 
      char *array[30];
      while (p != NULL)
      {
        array[i++] = p;
	/* Split if char is \n  */
        p = strtok (NULL, "\n");
      }
      for (e = 0; e < i; ++e)
      {
        /* If String begins with ETag  */
        if (strncmp(array[e], "ETag", 4) == 0)
        {
          size_t charCounter;
          size_t eTagCounter = 0;
	  /* Extract ETag Values */
          for (charCounter = 7; charCounter < strlen(array[e]); ++charCounter)
          {
            if (array[e][charCounter] != '"')
            {
	      strcpy(&eTag[eTagCounter], &array[e][charCounter]);
	      eTagCounter = eTagCounter + 1;
            }
          }
          strcat(&eTag[eTagCounter + 1], "\0");
        }
      }
      snprintf(eTagHeader, 30, "if-none-match: %s", eTag);
      free(req.header);
      return eTagHeader;
    }
    else
    {
      fprintf(stderr, "[Request Error] Could not parse eTag-Header. Not a 200 Response.\n");
      free(req.header);
      return 0;
    }
  }
  else
  {
    free(req.header);
    fprintf(stderr, "[Request Error] Playlist %s: CURLE_OK Check failed.\n", playlistid);
    return 0;
  }
}

int delete_playlist(char *playlistid)
{
  char buffer[80];
  snprintf(buffer, 80, "playlists/%s?countryCode=%s", playlistid, countryCode);

  curl_model req = curl_delete(buffer, "", "");
  free(req.body); 
  if (req.status != -1)
  {
    if (req.responseCode == 200 || req.responseCode == 204)
    {
      return 1;
    }
    else if (req.responseCode == 400)
    {
      fprintf(stderr, "[400] Bad Request (Invalid Indices)\n");
      return -9;
    }
    else if (req.responseCode == 401)
    {
      fprintf(stderr, "[401] Unauthorized\n");
      return -8;
    }
    else if (req.responseCode == 404)
    {
      fprintf(stderr, "[404] Resource %s not found\n", playlistid);
      return -2;
    }
    else if (req.responseCode == 412)
    {
      fprintf(stderr, "[412] Resource %s eTag invalid\n", playlistid);
      return -4;
    }
    else
    {
      return 0;
    }
  }
  else
  {
    fprintf(stderr, "[Request Error] Playlist %s: CURLE_OK Check failed.\n", playlistid);
    return -1;
  }  
}

int delete_playlist_item(char *playlistid, size_t index, char *eTagHeader)
{
  char buffer[80];
  snprintf(buffer, 80, "playlists/%s/items/%zu?countryCode=%s", playlistid, index, countryCode);
  curl_model req = curl_delete(buffer, "", eTagHeader);
  
  /* cleanup */
  free(req.body);
  
  if (req.status != -1)
  {
    if (req.responseCode == 200)
    {
      return 1;
    }
    else if (req.responseCode == 400)
    {
      fprintf(stderr, "[400] Bad Request (Invalid Indices)\n");
      return -9;
    }
    else if (req.responseCode == 401)
    {
      fprintf(stderr, "[401] Unauthorized\n");
      return -8;
    }
    else if (req.responseCode == 404)
    {
      fprintf(stderr, "[404] Resource %s not found\n", playlistid);
      return -2;
    }
    else if (req.responseCode == 412)
    {
      fprintf(stderr, "[412] Resource %s eTag invalid\n", playlistid);
      return -4;
    }
    else
    {
      return 0;
    }
  }
  else
  {
    fprintf(stderr, "[Request Error] Playlist %s: CURLE_OK Check failed.\n", playlistid);
    return -1;
  }
}

/* TODO: FIX THIS SHIT!  */
int move_playlist_item(char *playlistid, size_t index, size_t toIndex, char *eTagHeader)
{
  return 0;
}

int add_playlist_item(char *playlistid, size_t trackid, char *onDupes, char *eTagHeader)
{
  return 0;
}

int add_playlist_items(char *playlistid, char *trackids, char *onDupes, char *eTagHeader)
{
  return 0;
}

/* create & delete favourites */

playlist_model create_user_playlist(char *title, char *description)
{
  playlist_model Value;
  char *endpoint = url_cat("users/", userId, "/playlists", 1);

  char *data = malloc(strlen(title)+strlen(description)+7+14+1);
  strcpy(data, "title=");
  strcat(data, title);
  strcat(data, "&description=");
  strcat(data, description);

  curl_model req = curl_post(endpoint, data, "");
  /*Cleanup*/
  free(endpoint);
  free(data);

  if (req.status != -1)
  {
    cJSON *input_json = json_parse(req.body);
    if (req.responseCode == 201 || req.responseCode == 200)
    {
      json_playlist_model processed_json = json_parse_playlist(input_json);
      Value = parse_playlist_values(processed_json, 0);
      Value.status = 1;
      Value.arraySize = 1;

      free(req.body);
      cJSON_Delete(input_json);
      return Value;
    }
    else
    {
      Value.status = parse_status(input_json, req, userId, NULL);
      free(req.body);
      cJSON_Delete(input_json);
      return Value;
    }
  }
  else
  {
    printf("[Request Error] CURLE_OK Check failed.\n");
    playlist_model Value;
    Value.status = -1;
    free(req.body);
    return Value;
  }
}


int add_user_playlist(char *playlistid)
{
  char *endpoint = url_cat("users/", userId, "/favorites/playlists", 1);
  int status;
  char *data = malloc(strlen(playlistid)+10+25+1);
  strcpy(data, "uuids=");
  strcat(data, playlistid);
  strcat(data, "&onArtifactNotFound=FAIL");

  curl_model req = curl_post(endpoint, data, "");
  free(endpoint);
  free(data);
  free(req.body);
  if (req.status != -1)
  {
    if (req.responseCode == 200)
    {
      return 1;
    }
    else if (req.responseCode == 400)
    {
      status = -11;
      return status;
    }
    else if (req.responseCode == 401)
    {
      status = -8;
      return status;
    }
    else if (req.responseCode == 404)
    {
      status = -2;
      return status;
    }
    else
    {
      status = 0;
      return status;
    }
  }
  else
  {
    return -1;
  }
}

int delete_user_playlist(char *playlistid)
{
  int status;
  char buffer[80];
  snprintf(buffer, 80, "users/%zu/favorites/playlists/%s?countryCode=%s", userId, playlistid, countryCode);

  curl_model req = curl_delete(buffer, "", "");
  /*Cleanup*/
  free(req.body);

  if (req.status != -1)
  {
    if (req.responseCode == 200)
    {
      return 1;
    }
    else if (req.responseCode == 400)
    {
      status = -11;
      return status;
    }
    else if (req.responseCode == 401)
    {
      status = -8;
      return status;
    }
    else if (req.responseCode == 404)
    {
      status = -2;
      return status;
    }
    else
    {
      status = 0;
      return status;
    }
  }
  else
  {
    return -1;
  }
}

