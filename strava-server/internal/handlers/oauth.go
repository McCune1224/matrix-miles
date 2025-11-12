package handlers

import (
	"context"
	"fmt"
	"net/http"
	"time"

	"github.com/jackc/pgx/v5/pgtype"
	"github.com/labstack/echo/v4"
	"github.com/mckusa/strava-server/internal/database"
	"github.com/mckusa/strava-server/internal/strava"
)

type OAuthHandler struct {
	queries      *database.Queries
	stravaClient *strava.Client
}

func NewOAuthHandler(queries *database.Queries, stravaClient *strava.Client) *OAuthHandler {
	return &OAuthHandler{
		queries:      queries,
		stravaClient: stravaClient,
	}
}

// HandleLogin initiates the OAuth flow
func (h *OAuthHandler) HandleLogin(c echo.Context) error {
	// Optional: generate and store state for CSRF protection
	state := "random_state_string" // In production, generate a secure random state
	authURL := h.stravaClient.GetAuthURL(state)

	return c.Redirect(http.StatusTemporaryRedirect, authURL)
}

// HandleCallback handles the OAuth callback from Strava
func (h *OAuthHandler) HandleCallback(c echo.Context) error {
	code := c.QueryParam("code")
	if code == "" {
		return echo.NewHTTPError(http.StatusBadRequest, "Missing authorization code")
	}

	// Optional: verify state parameter
	// state := c.QueryParam("state")

	// Exchange authorization code for tokens
	tokenResp, err := h.stravaClient.ExchangeToken(code)
	if err != nil {
		return echo.NewHTTPError(http.StatusInternalServerError, fmt.Sprintf("Failed to exchange token: %v", err))
	}

	// Save or update user in database
	ctx := context.Background()
	expiresAt := time.Unix(tokenResp.ExpiresAt, 0)

	username := tokenResp.Athlete.Username
	if username == "" {
		username = fmt.Sprintf("%s %s", tokenResp.Athlete.Firstname, tokenResp.Athlete.Lastname)
	}

	user, err := h.queries.UpsertUser(ctx, database.UpsertUserParams{
		StravaUserID:   tokenResp.Athlete.ID,
		Username:       pgtype.Text{String: username, Valid: username != ""},
		AccessToken:    tokenResp.AccessToken,
		RefreshToken:   tokenResp.RefreshToken,
		TokenExpiresAt: pgtype.Timestamp{Time: expiresAt, Valid: true},
	})

	if err != nil {
		return echo.NewHTTPError(http.StatusInternalServerError, fmt.Sprintf("Failed to save user: %v", err))
	}

	// Return success page
	html := fmt.Sprintf(`
		<!DOCTYPE html>
		<html>
		<head>
			<title>Authorization Successful</title>
			<style>
				body {
					font-family: Arial, sans-serif;
					max-width: 600px;
					margin: 50px auto;
					padding: 20px;
					text-align: center;
				}
				.success {
					color: #28a745;
					font-size: 24px;
					margin-bottom: 20px;
				}
				.info {
					background: #f8f9fa;
					padding: 20px;
					border-radius: 8px;
					margin: 20px 0;
				}
				.user-id {
					font-size: 20px;
					font-weight: bold;
					color: #007bff;
				}
			</style>
		</head>
		<body>
			<div class="success">✓ Authorization Successful!</div>
			<p>Your Strava account has been connected successfully.</p>
			<div class="info">
				<p>Your User ID:</p>
				<p class="user-id">%d</p>
				<p><small>Use this ID to configure your ESP32 device</small></p>
			</div>
			<p>You can now close this window and configure your device.</p>
		</body>
		</html>
	`, user.ID)

	return c.HTML(http.StatusOK, html)
}
