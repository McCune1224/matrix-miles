#include "./cJSON.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/typecheck-gcc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *STRAVA_BASE_URL = "https://www.strava.com/api/v3";

struct MemoryStruct {
  char *memory;
  size_t size;
};

// more or less an interface to handle fwrite() calls from libcurl
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb,
                                  void *userp) {
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if (!ptr)
    return 0; // Out of memory :[
  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
  return realsize;
}

// Creates CURL client with predefined headers Content-Type:
// application/x-www-form-urlencoded
// CURL *create_strava_curl_client() {
//   CURL *client;
//   struct curl_slist *headers = NULL;
//   headers = curl_slist_append(
//       headers, "Content-Type: application/x-www-form-urlencoded");
//   curl_easy_setopt(client, CURLOPT_HTTPHEADER, headers);
//
//   return client;
// }

// Function to read .env file and get value by key
char *get_env_value(const char *key) {
  FILE *file = fopen(".env", "r");
  if (!file) {
    return NULL;
  }

  char line[256];

  while (fgets(line, sizeof(line), file)) {
    char *eq = strchr(line, '=');
    if (eq) {
      *eq = '\0';
      if (strcmp(line, key) == 0) {
        char *value = strdup(eq + 1);
        value[strcspn(value, "\n")] = '\0'; // Remove newline
        fclose(file);
        return value;
      }
    }
  }

  fclose(file);
  return NULL;
}

// Function to refresh access token
char *refresh_access_token(const char *client_id, const char *client_secret,
                           const char *refresh_token) {
  // Use libcurl to POST to Strava token endpoint
  CURL *curl_client = curl_easy_init();
  CURLcode response;
  curl_easy_setopt(curl_client, CURLOPT_URL, STRAVA_BASE_URL);

  struct curl_slist *headers = NULL;
  headers = curl_slist_append(
      headers, "Content-Type: application/x-www-form-urlencoded");
  curl_easy_setopt(curl_client, CURLOPT_HTTPHEADER, headers);

  char formatted_post_fields[256];
  sprintf(
      formatted_post_fields,
      "grant_type=refresh_token&client_id=%s&client_secret=%s&refresh_token=%s",
      client_id, client_secret, refresh_token);

  curl_easy_setopt(curl_client, CURLOPT_POST, 1L);
  curl_easy_setopt(curl_client, CURLOPT_POSTFIELDS, formatted_post_fields);

  struct MemoryStruct chunk = {0};
  curl_easy_setopt(curl_client, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(curl_client, CURLOPT_WRITEDATA, (void *)&chunk);

  response = curl_easy_perform(curl_client);
  if (response != CURLE_OK) {
    // Handle curl error
    free(chunk.memory);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl_client);
    return NULL;
  }
  printf("Response: %s\n", chunk.memory);

  cJSON *json_response = cJSON_Parse(chunk.memory);
  if (!json_response) {
    free(chunk.memory);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl_client);
    return NULL;
  }

  cJSON *token = cJSON_GetObjectItem(json_response, "access_token");
  int errorCount =
      cJSON_GetArraySize(cJSON_GetObjectItem(json_response, "errors"));

  printf("Error count: %d\n", errorCount);
  for (int i = 0; i < errorCount; i++) {
    cJSON *error =
        cJSON_GetArrayItem(cJSON_GetObjectItem(json_response, "errors"), i);
    cJSON *error_msg = cJSON_GetObjectItem(error, "message");
    if (error_msg && cJSON_IsString(error_msg)) {
      printf("Error: %s\n", error_msg->valuestring);
    }
  }

  char *access_token = NULL;
  if (token && cJSON_IsString(token)) {
    access_token = strdup(token->valuestring);
  }

  // Parse JSON response for access_token

  cJSON_Delete(json_response);
  free(chunk.memory);
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl_client);

  return access_token;
}

// Function to make API calls
void get_activities(const char *access_token) {
  // Use libcurl to GET /athlete/activities
  // Parse and print JSON response
}

int main() {
  // Load env vars
  char *client_id = get_env_value("STRAVA_CLIENT_ID");
  char *client_secret = get_env_value("STRAVA_CLIENT_SECRET");
  char *refresh_token_val = get_env_value("STRAVA_REFRESH_TOKEN");

  // Get access token
  char *access_token =
      refresh_access_token(client_id, client_secret, refresh_token_val);
  printf("GOT %s\n", access_token);

  // Fetch activities
  // get_activities(access_token);

  // // Cleanup
  // free(client_id);
  // free(client_secret);
  // free(refresh_token_val);
  // free(access_token);
  // printf("GOT %s\n", client_id);
  // printf("GOT %s\n", client_secret);
  // printf("GOT %s\n", refresh_token_val);
  return 0;
}
