package handlers

import (
	"context"
	"database/sql"
	"net/http"
	"strconv"
	"time"

	"github.com/jackc/pgx/v5/pgtype"
	"github.com/labstack/echo/v4"
	"github.com/mckusa/strava-server/internal/database"
	"github.com/mckusa/strava-server/internal/strava"
)

type APIHandler struct {
	queries      *database.Queries
	stravaClient *strava.Client
}

func NewAPIHandler(queries *database.Queries, stravaClient *strava.Client) *APIHandler {
	return &APIHandler{
		queries:      queries,
		stravaClient: stravaClient,
	}
}

// GetRecentActivities returns recent activities for a user
func (h *APIHandler) GetRecentActivities(c echo.Context) error {
	userID, err := strconv.Atoi(c.Param("userId"))
	if err != nil {
		return echo.NewHTTPError(http.StatusBadRequest, "Invalid user ID")
	}

	ctx := context.Background()
	activities, err := h.queries.GetRecentActivities(ctx, database.GetRecentActivitiesParams{
		UserID: int32(userID),
		Limit:  10,
	})

	if err != nil {
		return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
	}

	return c.JSON(http.StatusOK, activities)
}

// GetCalendarData returns calendar data for a specific month
func (h *APIHandler) GetCalendarData(c echo.Context) error {
	userID, err := strconv.Atoi(c.Param("userId"))
	if err != nil {
		return echo.NewHTTPError(http.StatusBadRequest, "Invalid user ID")
	}

	year, err := strconv.Atoi(c.Param("year"))
	if err != nil {
		return echo.NewHTTPError(http.StatusBadRequest, "Invalid year")
	}

	month, err := strconv.Atoi(c.Param("month"))
	if err != nil {
		return echo.NewHTTPError(http.StatusBadRequest, "Invalid month")
	}

	// Calculate date range
	firstDay := time.Date(year, time.Month(month), 1, 0, 0, 0, 0, time.UTC)
	lastDay := firstDay.AddDate(0, 1, -1).Add(23*time.Hour + 59*time.Minute + 59*time.Second)

	ctx := context.Background()
	calendarData, err := h.queries.GetCalendarData(ctx, database.GetCalendarDataParams{
		UserID:      int32(userID),
		StartDate:   pgtype.Timestamp{Time: firstDay, Valid: true},
		StartDate_2: pgtype.Timestamp{Time: lastDay, Valid: true},
	})

	if err != nil {
		return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
	}

	return c.JSON(http.StatusOK, calendarData)
}

// GetUserStats returns statistics for a user
func (h *APIHandler) GetUserStats(c echo.Context) error {
	userID, err := strconv.Atoi(c.Param("userId"))
	if err != nil {
		return echo.NewHTTPError(http.StatusBadRequest, "Invalid user ID")
	}

	ctx := context.Background()
	stats, err := h.queries.GetActivityStats(ctx, int32(userID))

	if err != nil {
		return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
	}

	return c.JSON(http.StatusOK, stats)
}

// SyncActivities fetches and stores activities from Strava
func (h *APIHandler) SyncActivities(c echo.Context) error {
	userID, err := strconv.Atoi(c.Param("userId"))
	if err != nil {
		return echo.NewHTTPError(http.StatusBadRequest, "Invalid user ID")
	}
	queryMonth := c.QueryParam("month")
	queryYear := c.QueryParam("year")

	ctx := context.Background()

	// Get user from database
	user, err := h.queries.GetUserByID(ctx, int32(userID))
	if err != nil {
		if err == sql.ErrNoRows {
			return echo.NewHTTPError(http.StatusNotFound, "User not found")
		}
		return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
	}

	// Check if token needs refresh
	accessToken := user.AccessToken
	if user.TokenExpiresAt.Valid && time.Now().After(user.TokenExpiresAt.Time) {
		// Refresh token
		tokenResp, err := h.stravaClient.RefreshToken(user.RefreshToken)
		if err != nil {
			return echo.NewHTTPError(http.StatusInternalServerError, "Failed to refresh token: "+err.Error())
		}

		// Update tokens in database
		expiresAt := time.Unix(tokenResp.ExpiresAt, 0)
		_, err = h.queries.UpdateUserTokens(ctx, database.UpdateUserTokensParams{
			ID:             user.ID,
			AccessToken:    tokenResp.AccessToken,
			RefreshToken:   tokenResp.RefreshToken,
			TokenExpiresAt: pgtype.Timestamp{Time: expiresAt, Valid: true},
		})

		if err != nil {
			return echo.NewHTTPError(http.StatusInternalServerError, "Failed to update tokens: "+err.Error())
		}

		accessToken = tokenResp.AccessToken
	}

	var activities []strava.Activity

	if queryMonth != "" && queryYear != "" {
		// Fetch activities for specific month
		month, err := strconv.Atoi(queryMonth)
		if err != nil || month < 1 || month > 12 {
			return echo.NewHTTPError(http.StatusBadRequest, "Invalid month")
		}

		year, err := strconv.Atoi(queryYear)
		if err != nil || year < 2000 || year > time.Now().Year() {
			return echo.NewHTTPError(http.StatusBadRequest, "Invalid year")
		}

		// Calculate date range
		firstDay := time.Date(year, time.Month(month), 1, 0, 0, 0, 0, time.UTC)
		lastDay := firstDay.AddDate(0, 1, -1).Add(23*time.Hour + 59*time.Minute + 59*time.Second)

		activities, err = h.stravaClient.GetActivitiesInRange(accessToken, firstDay.Unix(), lastDay.Unix())
		if err != nil {
			return echo.NewHTTPError(http.StatusInternalServerError, "Failed to fetch activities: "+err.Error())
		}
	} else {
		// Fetch activities from Strava (last 30 days)
		thirtyDaysAgo := time.Now().AddDate(0, 0, -30).Unix()
		activities, err = h.stravaClient.GetActivities(accessToken, thirtyDaysAgo, 100)
		if err != nil {
			return echo.NewHTTPError(http.StatusInternalServerError, "Failed to fetch activities: "+err.Error())
		}
	}

	// Save activities to database
	saved := 0
	for _, activity := range activities {
		_, err := h.queries.UpsertActivity(ctx, database.UpsertActivityParams{
			UserID:           user.ID,
			StravaActivityID: activity.ID,
			Name:             pgtype.Text{String: activity.Name, Valid: true},
			Type:             pgtype.Text{String: activity.Type, Valid: true},
			Distance:         pgtype.Float8{Float64: activity.Distance, Valid: true},
			MovingTime:       pgtype.Int4{Int32: int32(activity.MovingTime), Valid: true},
			ElapsedTime:      pgtype.Int4{Int32: int32(activity.ElapsedTime), Valid: true},
			StartDate:        pgtype.Timestamp{Time: activity.StartDate, Valid: true},
			StartDateLocal:   pgtype.Timestamp{Time: activity.StartDateLocal, Valid: true},
		})

		if err == nil {
			saved++
		}
	}

	return c.JSON(http.StatusOK, map[string]any{
		"message": "Sync completed",
		"fetched": len(activities),
		"saved":   saved,
	})
}
