#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

#define STRAVA_BASE_URL "https://www.strava.com/oauth/token"

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) return 0;
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

char* get_env_value(const char* key) {
    FILE* file = fopen(".env", "r");
    if (!file) return NULL;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char* eq = strchr(line, '=');
        if (eq) {
            *eq = '\0';
            if (strcmp(line, key) == 0) {
                char* value = strdup(eq + 1);
                value[strcspn(value, "\n")] = '\0';
                fclose(file);
                return value;
            }
        }
    }
    fclose(file);
    return NULL;
}

char *refresh_access_token(const char *client_id, const char *client_secret,
                           const char *refresh_token) {
    CURL *curl_client = curl_easy_init();
    if (!curl_client) return NULL;

    CURLcode response;
    curl_easy_setopt(curl_client, CURLOPT_URL, STRAVA_BASE_URL);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    curl_easy_setopt(curl_client, CURLOPT_HTTPHEADER, headers);

    char formatted_post_fields[512];  // Increased buffer size
    sprintf(formatted_post_fields,
            "grant_type=refresh_token&client_id=%s&client_secret=%s&refresh_token=%s",
            client_id, client_secret, refresh_token);

    curl_easy_setopt(curl_client, CURLOPT_POST, 1L);
    curl_easy_setopt(curl_client, CURLOPT_POSTFIELDS, formatted_post_fields);

    struct MemoryStruct chunk = {0};
    curl_easy_setopt(curl_client, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_client, CURLOPT_WRITEDATA, (void *)&chunk);

    response = curl_easy_perform(curl_client);
    if (response != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(response));
        free(chunk.memory);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl_client);
        return NULL;
    }

    cJSON *json_response = cJSON_Parse(chunk.memory);
    if (!json_response) {
        fprintf(stderr, "JSON parse error\n");
        free(chunk.memory);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl_client);
        return NULL;
    }

    cJSON *token = cJSON_GetObjectItem(json_response, "access_token");
    char *access_token = NULL;
    if (token && cJSON_IsString(token)) {
        access_token = strdup(token->valuestring);
    } else {
        fprintf(stderr, "Access token not found or invalid\n");
    }

    cJSON_Delete(json_response);
    free(chunk.memory);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl_client);
    return access_token;
}

int main() {
    char* client_id = get_env_value("CLIENT_ID");
    char* client_secret = get_env_value("CLIENT_SECRET");
    char* refresh_token_val = get_env_value("REFRESH_TOKEN");

    if (!client_id || !client_secret || !refresh_token_val) {
        fprintf(stderr, "Failed to load environment variables\n");
        return 1;
    }

    char* access_token = refresh_access_token(client_id, client_secret, refresh_token_val);
    if (access_token) {
        printf("Access token: %s\n", access_token);
        free(access_token);
    } else {
        fprintf(stderr, "Failed to refresh access token\n");
    }

    free(client_id);
    free(client_secret);
    free(refresh_token_val);

    return 0;
}