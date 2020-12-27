/*
    Copyright (c) 2020 Hugo Melder and openTIDAL_ContentContainer contributors
    
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

#include "include/cJSON.h"
#include "include/openTIDAL.h"

/* Initialise pointer in openTIDAL_ContentContainer structure */
void openTIDAL_StructInit(openTIDAL_ContentContainer *o)
{
    o->albums = NULL;
    o->items = NULL;
    o->artists = NULL;
    o->playlists = NULL;
    o->mixes = NULL;
    o->contributors = NULL;
    o->credits = NULL;
    o->links = NULL;

    o->home.recentlyPlayed_ArrayTypes = NULL;
    o->home.recentlyPlayed_ArrayPosition = NULL;
    o->home.radioStationsForYou_Start = NULL;
    o->home.radioStationsForYou_Total = NULL;
    o->home.becauseYouListenedTo_Start = NULL;
    o->home.becauseYouListenedTo_Title = NULL;
    o->home.becauseYouListenedTo_Total = NULL;

    o->status = 0;
    o->json = NULL;
    o->jsonManifest = NULL;

    openTIDAL_ParseVerbose("Struct", "Initialise openTIDAL_ContentContainer struct", 2);
}

/* Allocate the dynamic array in heap with the initial capacity. 
 * Use the openTIDAL_ContentContainer array identifiers to allocate the corrent array.
 * To keep track of the allocated size and the items added to the array,
 * there are two static arrays (capacity and total).
 * The indentifiers are used to access the correct values. */
void openTIDAL_StructAlloc(openTIDAL_ContentContainer *o, int index)
{
    switch(index)
    {
        case 0:
            o->capacity[0] = STRUCT_INIT_CAPACITY;
            o->total[0] = 0;
            o->albums = malloc(sizeof(openTIDAL_AlbumContainer) * o->capacity[0]);
            break;

        case 1:
            o->capacity[1] = STRUCT_INIT_CAPACITY;
            o->total[1] = 0;
            o->items = malloc(sizeof(openTIDAL_ItemsContainer) * o->capacity[1]);
            break; 

        case 2:
            o->capacity[2] = STRUCT_INIT_CAPACITY;
            o->total[2] = 0;
            o->artists = malloc(sizeof(openTIDAL_ArtistContainer) * o->capacity[2]);
            break;

        case 3:
            o->capacity[3] = STRUCT_INIT_CAPACITY;
            o->total[3] = 0;
            o->playlists = malloc(sizeof(openTIDAL_PlaylistContainer) * o->capacity[3]);
            break;

        case 4:
            o->capacity[4] = STRUCT_INIT_CAPACITY;
            o->total[4] = 0;
            o->mixes = malloc(sizeof(openTIDAL_MixContainer) * o->capacity[4]);
         break;

        case 5:
            o->capacity[5] = STRUCT_INIT_CAPACITY;
            o->total[5] = 0;
            o->contributors = malloc(sizeof(openTIDAL_ContributorContainer) * o->capacity[5]);
         break;

        case 6:
            o->capacity[6] = STRUCT_INIT_CAPACITY;
            o->total[6] = 0;
            o->credits = malloc(sizeof(openTIDAL_CreditsContainer) * o->capacity[6]);
        break;

        case 7:
            o->capacity[7] = STRUCT_INIT_CAPACITY;
            o->total[7] = 0;
            o->links = malloc(sizeof(openTIDAL_LinkContainer) * o->capacity[7]);
         break;
    }

    char buffer[50];
    snprintf(buffer, sizeof(buffer), "Allocate array (identifier: %d) in heap", index);
    openTIDAL_ParseVerbose("Struct", buffer, 2);
}

/* Deallocates all dynamic arrays inside the openTIDAL_ContentContainer structure.
 * The allocated cJSON-Tree and all it's values will be deallocated
 * too. */
void openTIDAL_StructDelete(openTIDAL_ContentContainer *o)
{
    cJSON_Delete((cJSON *) o->json);
    cJSON_Delete((cJSON *) o->jsonManifest);

    if ( o->albums )
    {
        int i;
        for (i = 0; i < o->total[0]; ++i)
        {
            free(o->albums[i].artistId);
            free(o->albums[i].artistName);
        }
    }

    if ( o->items )
    {
        int i;
        for (i = 0; i < o->total[1]; ++i)
        {
            free(o->items[i].artistId);
            free(o->items[i].artistName);
        }

    }

    free(o->home.recentlyPlayed_ArrayTypes);
    free(o->home.recentlyPlayed_ArrayPosition);
    free(o->home.radioStationsForYou_Start);
    free(o->home.radioStationsForYou_Total);
    free(o->home.becauseYouListenedTo_Start);
    free(o->home.becauseYouListenedTo_Title);
    free(o->home.becauseYouListenedTo_Total);

    free(o->albums);
    free(o->items);
    free(o->artists);
    free(o->playlists);
    free(o->mixes);
    free(o->contributors);
    free(o->credits);
    free(o->links);

    openTIDAL_ParseVerbose("Struct", "Deallocate all arrays in structure", 2);
}

/* Resizes a specific dynamic array. Use the openTIDAL_ContentContainer array identifiers
 * to resize the correct array. The allocated array will grow by doubling
 * it's capacity (See openTIDAL_StructAdd). This safes system resources */
static void openTIDAL_StructResize(openTIDAL_ContentContainer *o, int capacity, int index)
{
    openTIDAL_AlbumContainer *albums;
    openTIDAL_ItemsContainer *items;
    openTIDAL_ArtistContainer *artists;
    openTIDAL_PlaylistContainer *playlists;
    openTIDAL_MixContainer *mixes;
    openTIDAL_ContributorContainer *contributors;
    openTIDAL_CreditsContainer *credits;
    openTIDAL_LinkContainer *links;

    switch(index)
    {
        case 0:
            albums = realloc(o->albums, sizeof(openTIDAL_AlbumContainer) * capacity);
            if( albums )
            {
                o->albums = albums;
                o->capacity[0] = capacity;
            }
            break;

        case 1:
            items = realloc(o->items, sizeof(openTIDAL_ItemsContainer) * capacity);
            if( items )
            {
                o->items = items;
                o->capacity[1] = capacity;
            }
         break; 

        case 2:
            artists = realloc(o->artists, sizeof(openTIDAL_ArtistContainer) * capacity);
            if( artists )
            {
                o->artists = artists;
                o->capacity[2] = capacity;
            }
            break;

        case 3:
            playlists = realloc(o->playlists, sizeof(openTIDAL_PlaylistContainer) * capacity);
            if( playlists )
            {
                o->playlists = playlists;
                o->capacity[3] = capacity;
            }
            break;

        case 4:
            mixes = realloc(o->mixes, sizeof(openTIDAL_MixContainer) * capacity);
            if( mixes )
            {
                o->mixes = mixes;
                o->capacity[4] = capacity;
            }
            break;

        case 5:
            contributors = realloc(o->contributors, sizeof(openTIDAL_ContributorContainer) * capacity);
            if( contributors )
            {
                o->contributors = contributors;
                o->capacity[5] = capacity;
            }
            break;

        case 6:
            credits = realloc(o->credits, sizeof(openTIDAL_CreditsContainer) * capacity);
            if( credits )
            {
                o->credits = credits;
                o->capacity[6] = capacity;
            }
            break;

        case 7:
            links = realloc(o->links, sizeof(openTIDAL_LinkContainer) * capacity);
            if( links )
            {
                o->links = links;
                o->capacity[7] = capacity;
            }
            break;
    }

    char buffer[50];
    snprintf(buffer, sizeof(buffer), "Realloc array (identifier %d) from %d to %d", index, capacity / 2, capacity);
    openTIDAL_ParseVerbose("Struct", buffer, 2);
}

void openTIDAL_StructAddAlbum(openTIDAL_ContentContainer *o, openTIDAL_AlbumContainer album)
{
    int index = 0;
    if ( o->capacity[index] == o->total[index] )
        openTIDAL_StructResize(o, o->capacity[index] * 2, index);
    o->albums[o->total[index]++] = album;

    openTIDAL_ParseVerbose("Struct", "Add item to array (identifier 0)", 2);
}

void openTIDAL_StructAddItem(openTIDAL_ContentContainer *o, openTIDAL_ItemsContainer item)
{
    int index = 1;
    if ( o->capacity[index] == o->total[index] )
        openTIDAL_StructResize(o, o->capacity[index] * 2, index);
    o->items[o->total[index]++] = item;

    openTIDAL_ParseVerbose("Struct", "Add item to array (identifier 1)", 2);
}

void openTIDAL_StructAddArtist(openTIDAL_ContentContainer *o, openTIDAL_ArtistContainer artist)
{
    int index = 2;
    if ( o->capacity[index] == o->total[index] )
        openTIDAL_StructResize(o, o->capacity[index] * 2, index);
    o->artists[o->total[index]++] = artist;

    openTIDAL_ParseVerbose("Struct", "Add item to array (identifier 2)", 2);
}

void openTIDAL_StructAddPlaylist(openTIDAL_ContentContainer *o, openTIDAL_PlaylistContainer playlist)
{
    int index = 3;
    if ( o->capacity[index] == o->total[index] )
        openTIDAL_StructResize(o, o->capacity[index] * 2, index);
    o->playlists[o->total[index]++] = playlist;

    openTIDAL_ParseVerbose("Struct", "Add item to array (identifier 3)", 2);
}

void openTIDAL_StructAddMix(openTIDAL_ContentContainer *o, openTIDAL_MixContainer mix)
{
    int index = 4;
    if ( o->capacity[index] == o->total[index] )
        openTIDAL_StructResize(o, o->capacity[index] * 2, index);
    o->mixes[o->total[index]++] = mix;

    openTIDAL_ParseVerbose("Struct", "Add item to array (identifier 4)", 2);
}

void openTIDAL_StructAddContributor(openTIDAL_ContentContainer *o, openTIDAL_ContributorContainer contributor)
{
    int index = 5;
    if ( o->capacity[index] == o->total[index] )
        openTIDAL_StructResize(o, o->capacity[index] * 2, index);
    o->contributors[o->total[index]++] = contributor;

    openTIDAL_ParseVerbose("Struct", "Add item to array (identifier 5)", 2);
}

void openTIDAL_StructAddCredit(openTIDAL_ContentContainer *o, openTIDAL_CreditsContainer credit)
{
    int index = 6;
    if ( o->capacity[index] == o->total[index] )
        openTIDAL_StructResize(o, o->capacity[index] * 2, index);
    o->credits[o->total[index]++] = credit;

    openTIDAL_ParseVerbose("Struct", "Add item to array (identifier 6)", 2);
}

void openTIDAL_StructAddLink(openTIDAL_ContentContainer *o, openTIDAL_LinkContainer link)
{
    int index = 7;
    if ( o->capacity[index] == o->total[index] )
        openTIDAL_StructResize(o, o->capacity[index] * 2, index);
    o->links[o->total[index]++] = link;

    openTIDAL_ParseVerbose("Struct", "Add item to array (identifier 7)", 2);
}