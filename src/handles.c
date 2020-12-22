/*
  Copyright (c) 2020 Hugo Melder and openTIDAL contributors
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "include/handles.h"
#include "include/parse.h"
#include "include/openTIDAL.h"

/* struct used to parse raw libcurl response  */
struct MemoryStruct {
  char *memory;
  size_t size;
};

/* calculate size of response and realloc it  */
static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(ptr == NULL) {
    /* out of memory! */
    openTIDAL_ParseVerbose("cURL Handle", "Not enough memory (realloc returned NULL)", 1);
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

int curl_init = 0;
CURL *curl;

CURL *curl_session()
{
  curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();
  openTIDAL_ParseVerbose("cURL Handle", "Initialise baseUrl handle", 2);
  return curl;
}

void curl_exit()
{
  openTIDAL_ParseVerbose("cURL Handle", "Cleanup baseUrl handle", 2);
  curl_easy_cleanup(curl);
  curl_global_cleanup();
}

/* persistent cURL handle for API GET requests */
curl_model curl_get(char *endpoint, char *data)
{
  if(curl_init == 0)
  {
    curl = curl_session();
    curl_init = 1;
  }
  CURLcode res;
  curl_model model;
  struct MemoryStruct response;
  char *url = NULL;
  char *header = NULL;
  char *client_header = NULL;
  
  /* check if access_token has expired  */
  refresh_persistent();
  /* will be grown as needed by the realloc above */
  response.memory = malloc(1); 
  /* no data at this point */
  response.size = 0; 
  
  /* char concatenation of the url, endpoint and data
     allocate size of baseUrl, endpoint and data  */
  url = malloc(strlen(config.baseUrl)+1+strlen(endpoint)+1+strlen(data));
  strcpy(url, config.baseUrl);
  strcat(url, endpoint);
  strcat(url, "?");
  strcat(url, data);
  
 /* Specify Authorization Header or Demo Header  */ 
  if (config.demoEnabled != 1)
  {
    /* allocate size of access_token and header  */
    header = malloc(strlen(config.accessToken)+23+1);
    strcpy(header, "authorization: Bearer ");
    strcat(header, config.accessToken);
  }
  else
  {
    char client_header_key[] = "X-Tidal-Token: ";
    /* allocate size of client_id and header  */
    client_header = malloc(strlen(config.clientId)+strlen(client_header_key)+1);
    strcpy(client_header, client_header_key);
    strcat(client_header, config.clientId);
  }

  if(curl_init == 1)
  {
    /* create curl header list  */
    struct curl_slist *chunk = NULL;
    /* append the right header  */
    if (config.demoEnabled != 1)
    {
      /* append authentication header with user access_token  */
      chunk = curl_slist_append(chunk, header);
    }
    else
    {
      /* append client_id header */
      chunk = curl_slist_append(chunk, client_header);
    }
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    
    openTIDAL_ParseVerbose("cURL Handle", "Get Handle Trace", 3);
    if (openTIDAL_GetLogLevel() == 3)
      curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    res = curl_easy_perform(curl);
    
    /* cleanup */
    curl_slist_free_all(chunk);
    free(url);
    free(header);
    free(client_header);

    if (res != CURLE_OK)
    {
      model.status = -1;
      return model;
    }
    else
    {
      model.body = response.memory;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &model.responseCode);
      return model;
    }
  }
  else
  {
    model.status = -1;
    return model;
  }
}

/* persistent cURL handle for API POST requests */
curl_model curl_post(char *endpoint, char *data, char *optHeader)
{
  if(curl_init == 0)
  {
    curl = curl_session();
    curl_init = 1;
  }
  CURLcode res;
  curl_model model;

  struct MemoryStruct response;
  char *url = NULL;
  char *header = NULL;
  char *client_header = NULL;

  /* check if access_token has expired  */
  refresh_persistent();
  /* will be grown as needed by the realloc above */
  response.memory = malloc(1);
  /* no data at this point */
  response.size = 0; 

  /* char concatenation */
  url = malloc(strlen(config.baseUrl)+1+strlen(endpoint));
  strcpy(url, config.baseUrl);
  strcat(url, endpoint);
  /* specify Authorization Header or Demo Header  */
  if (config.demoEnabled != 1)
  {
    /* allocate size of access_token and header  */
    header = malloc(strlen(config.accessToken)+23+1);
    strcpy(header, "authorization: Bearer ");
    strcat(header, config.accessToken);
  }
  else
  {
    /* create client_id authorization header  */
    char client_header_key[] = "x-tidal-token: ";
    /* allocate size of client_id and header  */
    client_header = malloc(strlen(config.clientId)+strlen(client_header_key)+1);
    strcpy(client_header, client_header_key);
    strcat(client_header, config.clientId);
  }
  
  if(curl_init == 1)
  {
    struct curl_slist *chunk = NULL;
    /* append the right header  */
    if (config.demoEnabled != 1)
    {
      /* append authentication header with user access_token  */
      chunk = curl_slist_append(chunk, header);
    }
    else
    {
      /* append client_id header  */
      //chunk = curl_slist_append(chunk, client_header);
    }
    /* append optional header (like If-None-Match)  */
    chunk = curl_slist_append(chunk, optHeader);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, NULL);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    
    openTIDAL_ParseVerbose("cURL Handle", "Post Request Trace", 3);
    if (openTIDAL_GetLogLevel() == 3)
      curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    
    res = curl_easy_perform(curl);

    /* cleanup */
    curl_slist_free_all(chunk);
    free(url);
    free(header);
    free(client_header);

    if (res != CURLE_OK)
    {
      model.status = -1;
      return model;
    }
    else
    {
      model.body = response.memory;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &model.responseCode);
      return model;
    }
  }
  else
  {
    model.status = -1;
    return model;
  }
}

/* persistent cURL handle for API DELETE requests */
curl_model curl_delete(char *endpoint, char *data, char *optHeader)
{
  if(curl_init == 0)
  {
    curl = curl_session();
    curl_init = 1;
  }
  CURLcode res;
  curl_model model;

  struct MemoryStruct response;
  char *url = NULL;
  char *header = NULL;
  char *client_header = NULL;

  refresh_persistent();

  /* will be grown as needed by the realloc above */
  response.memory = malloc(1);
  /* no data at this point */ 
  response.size = 0;

  /*char concatenation*/
  url = malloc(strlen(config.baseUrl)+1+strlen(endpoint)+1+strlen(data));
  strcpy(url, config.baseUrl);
  strcat(url, endpoint);

  /* Specify Authorization Header or Demo Header  */
  if (config.demoEnabled != 1)
  {
    /* allocate size of access_token and header  */
    header = malloc(strlen(config.accessToken)+23+1);
    strcpy(header, "authorization: Bearer ");
    strcat(header, config.accessToken);
  }
  else
  {
    char client_header_key[] = "x-tidal-token: ";
    /* allocate size of client_id and header  */
    client_header = malloc(strlen(config.clientId)+strlen(client_header_key)+1);
    strcpy(client_header, client_header_key);
    strcat(client_header, config.clientId);
  }

  if(curl_init == 1) {
    struct curl_slist *chunk = NULL;
    /* append the right header  */
    if (config.demoEnabled != 1)
    {
      /* append authentication header with user access_token  */
      chunk = curl_slist_append(chunk, header);
    }
    else
    {
      /* append client_id header  */
      chunk = curl_slist_append(chunk, client_header);
    }
    chunk = curl_slist_append(chunk, optHeader);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    
    openTIDAL_ParseVerbose("cURL Handle", "Delete Request Trace", 3);
    if (openTIDAL_GetLogLevel() == 3)
      curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    
    res = curl_easy_perform(curl);
    
    /* cleanup */
    curl_slist_free_all(chunk);
    free(url);
    free(header);
    free(client_header);

    if (res != CURLE_OK)
    {
      model.status = -1;
      return model;
    }
    else
    {
      model.body = response.memory;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &model.responseCode);
      return model;
    }
  }
  else
  {
    model.status = -1;
    return model;
  }
}

/* persistent cURL handle for API HEAD requests */
curl_model curl_head(char *endpoint, char *data)
{
  /* Create a new cURL handle for the head request
   * because of a cURL bug. If a head request is
   * performed with a persistent cURL handle a
   * Segmentation Fault occurs. Other requests
   * perform flawlessly -_- */
  CURL *curlHead = curl_easy_init();
  CURLcode res;
  curl_model model;
  struct MemoryStruct response;
  char *url;
  char *header;
  char *client_header;
   
  /* check if access_token has expired  */
  refresh_persistent();
  /* will be grown as needed by the realloc above */ 
  response.memory = malloc(1);
  /* no data at this point */
  response.size = 0;

  /*char concatenation*/
  url = malloc(strlen(config.baseUrl)+1+strlen(endpoint)+1+strlen(data));
  strcpy(url, config.baseUrl);
  strcat(url, endpoint);
  strcat(url, "?");
  strcat(url, data);

  /* Specify Authorization Header or Demo Header  */
  if (config.demoEnabled != 1)
  {
    /* allocate size of access_token and header  */
    header = malloc(strlen(config.accessToken)+23+1);
    strcpy(header, "authorization: Bearer ");
    strcat(header, config.accessToken);
  }
  else
  {
    char client_header_key[] = "x-tidal-token: ";
    /* allocate size of client_id and header  */
    client_header = malloc(strlen(config.clientId)+strlen(client_header_key)+1);
    strcpy(client_header, client_header_key);
    strcat(client_header, config.clientId);
  }

  if(curlHead) {
    struct curl_slist *chunk = NULL;
    /* append the right header  */
    if (config.demoEnabled != 1)
    {
      /* append authentication header with user access_token  */
      chunk = curl_slist_append(chunk, header);
    }
    else
    {
      /* append client_id header  */
      chunk = curl_slist_append(chunk, client_header);
    }
    curl_easy_setopt(curlHead, CURLOPT_URL, url);
    curl_easy_setopt(curlHead, CURLOPT_HTTPHEADER, chunk);
    curl_easy_setopt(curlHead, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curlHead, CURLOPT_HEADERFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curlHead, CURLOPT_HEADERDATA, &response); 
    
    openTIDAL_ParseVerbose("cURL Handle", "Head Request Trace", 3);
    if (openTIDAL_GetLogLevel() == 3)
      curl_easy_setopt(curlHead, CURLOPT_VERBOSE, 1L);
    
    res = curl_easy_perform(curlHead);
    
    free(header);
    free(client_header);

    if (res != CURLE_OK)
    {
      model.status = -1;
    }
    else
    {
      model.header = response.memory;
      curl_easy_getinfo(curlHead, CURLINFO_RESPONSE_CODE, &model.responseCode);
    }

    /* cleanup  */
    curl_slist_free_all(chunk);
    free(url);
    curl_easy_cleanup(curlHead);
    return model;
  }
  else
  {
    model.status = -1;
    return model;
  }
}

/* auth handle */

int curl_init_auth = 0;
CURL *curl_auth;

CURL *curl_session_auth()
{
  curl_auth = curl_easy_init();
  return curl_auth;
}

void curl_exit_auth()
{
  openTIDAL_ParseVerbose("cURL Handle", "Cleanup auth handle", 2); 
  curl_easy_cleanup(curl_auth);
}

curl_model curl_post_auth(char *endpoint, char *data)
{
  if(curl_init_auth == 0)
    {
      curl_auth = curl_session_auth();
      curl_init_auth = 1;
    }
  CURLcode res;
  curl_model model;
  struct MemoryStruct response;

  /* will be grown as needed by the realloc above */
  response.memory = malloc(1);
  /* no data at this point */
  response.size = 0;

  /*char concatenation*/
  char *url;
  url = malloc(strlen(config.authUrl)+1+strlen(endpoint));
  strcpy(url, config.authUrl);
  strcat(url, endpoint);

  if(curl_init_auth == 1) {
    curl_easy_setopt(curl_auth, CURLOPT_URL, url);
    curl_easy_setopt(curl_auth, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    /* append (x-www-form-urlencoded) parameter */
    curl_easy_setopt(curl_auth, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(curl_auth, CURLOPT_WRITEDATA, &response);
    /* enable basic authentication (clientid:client_secret) */
    curl_easy_setopt(curl_auth, CURLOPT_USERNAME, config.clientId);
    curl_easy_setopt(curl_auth, CURLOPT_PASSWORD, config.clientSecret);
    
    openTIDAL_ParseVerbose("cURL Handle", "Post Auth Request Trace", 3);
    if (openTIDAL_GetLogLevel() == 3)
      curl_easy_setopt(curl_auth, CURLOPT_VERBOSE, 1L);

    res = curl_easy_perform(curl_auth);
    
    free(url);
    
    if (res != CURLE_OK)
    {
      model.status = -1;
      return model;
    }
    else
    {
      model.body = response.memory;
      curl_easy_getinfo(curl_auth, CURLINFO_RESPONSE_CODE, &model.responseCode);
      return model;
    }
  }
  else
  {
    model.status = -1;
    return model;
  }
}
