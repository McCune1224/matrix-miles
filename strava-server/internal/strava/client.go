package strava

import (
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"net/url"
	"strings"
	"time"
)

const (
	stravaAPIBase = "https://www.strava.com"
	tokenEndpoint = "/oauth/token"
	authEndpoint  = "/oauth/authorize"
)

type Client struct {
	clientID     string
	clientSecret string
	redirectURI  string
	httpClient   *http.Client
}

type TokenResponse struct {
	AccessToken  string  `json:"access_token"`
	RefreshToken string  `json:"refresh_token"`
	ExpiresAt    int64   `json:"expires_at"`
	ExpiresIn    int     `json:"expires_in"`
	TokenType    string  `json:"token_type"`
	Athlete      Athlete `json:"athlete"`
}

type Athlete struct {
	ID        int64  `json:"id"`
	Username  string `json:"username"`
	Firstname string `json:"firstname"`
	Lastname  string `json:"lastname"`
}

type Activity struct {
	ID             int64     `json:"id"`
	Name           string    `json:"name"`
	Type           string    `json:"type"`
	Distance       float64   `json:"distance"`
	MovingTime     int       `json:"moving_time"`
	ElapsedTime    int       `json:"elapsed_time"`
	StartDate      time.Time `json:"start_date"`
	StartDateLocal time.Time `json:"start_date_local"`
}

func NewClient(clientID, clientSecret, redirectURI string) *Client {
	return &Client{
		clientID:     clientID,
		clientSecret: clientSecret,
		redirectURI:  redirectURI,
		httpClient: &http.Client{
			Timeout: 30 * time.Second,
		},
	}
}

// GetAuthURL returns the OAuth authorization URL
func (c *Client) GetAuthURL(state string) string {
	params := url.Values{}
	params.Add("client_id", c.clientID)
	params.Add("redirect_uri", c.redirectURI)
	params.Add("response_type", "code")
	params.Add("scope", "activity:read_all,profile:read_all")
	if state != "" {
		params.Add("state", state)
	}

	return fmt.Sprintf("%s%s?%s", stravaAPIBase, authEndpoint, params.Encode())
}

// ExchangeToken exchanges an authorization code for access tokens
func (c *Client) ExchangeToken(code string) (*TokenResponse, error) {
	data := url.Values{}
	data.Set("client_id", c.clientID)
	data.Set("client_secret", c.clientSecret)
	data.Set("code", code)
	data.Set("grant_type", "authorization_code")

	return c.requestToken(data)
}

// RefreshToken exchanges a refresh token for new access tokens
func (c *Client) RefreshToken(refreshToken string) (*TokenResponse, error) {
	data := url.Values{}
	data.Set("client_id", c.clientID)
	data.Set("client_secret", c.clientSecret)
	data.Set("refresh_token", refreshToken)
	data.Set("grant_type", "refresh_token")

	return c.requestToken(data)
}

func (c *Client) requestToken(data url.Values) (*TokenResponse, error) {
	endpoint := stravaAPIBase + tokenEndpoint

	req, err := http.NewRequest("POST", endpoint, strings.NewReader(data.Encode()))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Content-Type", "application/x-www-form-urlencoded")

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to make request: %w", err)
	}
	defer resp.Body.Close()

	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, fmt.Errorf("failed to read response: %w", err)
	}

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("strava API error (status %d): %s", resp.StatusCode, string(body))
	}

	var tokenResp TokenResponse
	if err := json.Unmarshal(body, &tokenResp); err != nil {
		return nil, fmt.Errorf("failed to parse response: %w", err)
	}

	return &tokenResp, nil
}

// GetActivities fetches activities from Strava API
func (c *Client) GetActivities(accessToken string, after int64, perPage int) ([]Activity, error) {
	if perPage == 0 {
		perPage = 30
	}
	if perPage > 200 {
		perPage = 200
	}

	endpoint := fmt.Sprintf("%s/api/v3/athlete/activities", stravaAPIBase)
	params := url.Values{}
	params.Add("per_page", fmt.Sprintf("%d", perPage))
	if after > 0 {
		params.Add("after", fmt.Sprintf("%d", after))
	}

	reqURL := fmt.Sprintf("%s?%s", endpoint, params.Encode())
	req, err := http.NewRequest("GET", reqURL, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+accessToken)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to make request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		body, _ := io.ReadAll(resp.Body)
		return nil, fmt.Errorf("strava API error (status %d): %s", resp.StatusCode, string(body))
	}

	var activities []Activity
	if err := json.NewDecoder(resp.Body).Decode(&activities); err != nil {
		return nil, fmt.Errorf("failed to parse activities: %w", err)
	}

	return activities, nil
}

func (c *Client) GetActivitiesInRange(accessToken string, startDayUnix, endDayUnix int64) ([]Activity, error) {
	allActivities := []Activity{}
	page := 1
	perPage := 100
	for {
		endpoint := fmt.Sprintf("%s/api/v3/athlete/activities", stravaAPIBase)
		params := url.Values{}
		params.Add("per_page", fmt.Sprintf("%d", perPage))
		params.Add("page", fmt.Sprintf("%d", page))

		reqURL := fmt.Sprintf("%s?%s", endpoint, params.Encode())
		req, err := http.NewRequest("GET", reqURL, nil)
		if err != nil {
			return nil, fmt.Errorf("failed to create request: %w", err)
		}

		req.Header.Set("Authorization", "Bearer "+accessToken)

		resp, err := c.httpClient.Do(req)
		if err != nil {
			return nil, fmt.Errorf("failed to make request: %w", err)
		}
		defer resp.Body.Close()

		if resp.StatusCode != http.StatusOK {
			body, _ := io.ReadAll(resp.Body)
			return nil, fmt.Errorf("strava API error (status %d): %s", resp.StatusCode, string(body))
		}

		var activities []Activity
		if err := json.NewDecoder(resp.Body).Decode(&activities); err != nil {
			return nil, fmt.Errorf("failed to parse activities: %w", err)
		}

		if len(activities) == 0 {
			break
		}

		for _, activity := range activities {
			startUnix := activity.StartDate.Unix()
			if startUnix >= startDayUnix && startUnix <= endDayUnix {
				allActivities = append(allActivities, activity)
			}
		}
		page++
	}

	return allActivities, nil
}
