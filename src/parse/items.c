#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/openTIDAL.h"

items_model parse_tracks(cJSON *input_json) /* TODO: Add bool types & replayGain/peak  */
{
  items_model Value;
  const cJSON *artistItem = NULL;
  int artistCounter = 0;
  
  cJSON *id = cJSON_GetObjectItem(input_json, "id");
  cJSON *title = cJSON_GetObjectItemCaseSensitive(input_json, "title");
  cJSON *duration = cJSON_GetObjectItem(input_json, "duration");
  cJSON *popularity = cJSON_GetObjectItem(input_json, "popularity");
  cJSON *trackNumber = cJSON_GetObjectItem(input_json, "trackNumber");
  cJSON *volumeNumber = cJSON_GetObjectItem(input_json, "volumeNumber");
  cJSON *version = cJSON_GetObjectItemCaseSensitive(input_json, "version");
  cJSON *artist = cJSON_GetObjectItemCaseSensitive(input_json, "artists");
  cJSON *album = cJSON_GetObjectItemCaseSensitive(input_json, "album");
  cJSON *albumTitle = cJSON_GetObjectItemCaseSensitive(album, "title");
  cJSON *albumCover = cJSON_GetObjectItemCaseSensitive(album, "cover"); 

  Value.id[0] = id->valueint;
  strcpy(Value.title[0], title->valuestring);
  Value.duration[0] = duration->valueint;
  Value.popularity[0] = popularity->valueint;
  Value.trackNumber[0] = trackNumber->valueint;
  Value.volumeNumber[0] = volumeNumber->valueint;
  
  if (cJSON_IsNull(version) != 1)
  {
    strcpy(Value.version[0], version->valuestring);
  }
  
  Value.subArraySize[0] = cJSON_GetArraySize(artist);
  cJSON_ArrayForEach(artistItem, artist)
  {
    cJSON *artistId = cJSON_GetObjectItem(artistItem, "id");
    cJSON *artistName = cJSON_GetObjectItemCaseSensitive(artistItem, "name");

    Value.artistId[0][artistCounter] = artistId->valueint;
    strcpy(Value.artistName[0][artistCounter], artistName->valuestring);
    artistCounter = artistCounter + 1;
  }
  
  Value.albumId[0] = cJSON_GetObjectItem(album, "id")->valueint;
  strcpy(Value.albumTitle[0], albumTitle->valuestring);
  strcpy(Value.cover[0], albumCover->valuestring);

  return Value;
}

items_model parse_items(cJSON *input_json, int version, int video) /* TODO: Add bool types */
{
  items_model Value;
  const cJSON *version_json = NULL;
  const cJSON *item = NULL;
  const cJSON *items = NULL;
  const cJSON *artistItems = NULL;
  const cJSON *artistItem = NULL;
  int i = 0;

  items = cJSON_GetObjectItemCaseSensitive(input_json, "items");
  Value.arraySize = cJSON_GetArraySize(items);
  Value.offset = cJSON_GetObjectItemCaseSensitive(input_json, "offset")->valueint;
  Value.totalNumberOfItems = cJSON_GetObjectItemCaseSensitive(input_json, "totalNumberOfItems")->valueint;
  if (Value.totalNumberOfItems != 0)
  {
    cJSON_ArrayForEach(item, items)
    {
      int artistCounter = 0;
      if (version == 1) /* Sometimes the JSON response varies */
      {
      version_json = item;
      }
      else
      {
        version_json = cJSON_GetObjectItemCaseSensitive(item, "item");
      }

      cJSON *id = cJSON_GetObjectItem(version_json, "id");
      cJSON *title = cJSON_GetObjectItemCaseSensitive(version_json, "title");
      cJSON *duration = cJSON_GetObjectItem(version_json, "duration");
      cJSON *popularity = cJSON_GetObjectItem(version_json, "popularity");
      cJSON *trackNumber = cJSON_GetObjectItem(version_json, "trackNumber");
      cJSON *volumeNumber = cJSON_GetObjectItem(version_json, "volumeNumber");
      cJSON *version = cJSON_GetObjectItemCaseSensitive(version_json, "version");
      cJSON *artist = cJSON_GetObjectItemCaseSensitive(version_json, "artists");

      Value.id[i] = id->valueint;
      strcpy(Value.title[i], title->valuestring);
      Value.duration[i] = duration->valueint;
      Value.popularity[i] = popularity->valueint;
      Value.trackNumber[i] = trackNumber->valueint;
      Value.volumeNumber[i] = volumeNumber->valueint;
      if (cJSON_IsNull(version) != 1)
      {
        strcpy(Value.version[i], version->valuestring);
      }

      Value.subArraySize[i] = cJSON_GetArraySize(artist);
      cJSON_ArrayForEach(artistItem, artist)
      {
        cJSON *artistId = cJSON_GetObjectItemCaseSensitive(artistItem, "id");
	cJSON *artistName = cJSON_GetObjectItemCaseSensitive(artistItem, "name");

        Value.artistId[i][artistCounter] = artistId->valueint;
        strcpy(Value.artistName[i][artistCounter], artistName->valuestring);
        artistCounter = artistCounter + 1;
      }

      if (video == 0) /* Videos don't have an album section */
        {
          cJSON *album = cJSON_GetObjectItemCaseSensitive(version_json, "album");
	  cJSON *albumId = cJSON_GetObjectItemCaseSensitive(album, "id");
          cJSON *albumTitle = cJSON_GetObjectItemCaseSensitive(album, "title");
          cJSON *albumCover = cJSON_GetObjectItemCaseSensitive(album, "cover");
          
	  Value.albumId[i] = albumId->valueint;
          strcpy(Value.albumTitle[i], albumTitle->valuestring);
          strcpy(Value.cover[i], albumCover->valuestring);
        }
      i = i + 1;
    }
  }
  else
  {
    printf("%s\n", "No Items in Response");
  }
   return Value;
}
