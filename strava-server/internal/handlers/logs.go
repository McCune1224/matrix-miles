package handlers

import (
	"context"
	"net/http"
	"strconv"
	"time"

	"github.com/jackc/pgx/v5/pgtype"
	"github.com/labstack/echo/v4"
	"github.com/mckusa/strava-server/internal/database"
)

type LogHandler struct {
	queries *database.Queries
}

func NewLogHandler(queries *database.Queries) *LogHandler {
	return &LogHandler{
		queries: queries,
	}
}

// GetRecentLogs returns recent application logs
func (h *LogHandler) GetRecentLogs(c echo.Context) error {
	// Parse query parameters
	hoursStr := c.QueryParam("hours")
	if hoursStr == "" {
		hoursStr = "24" // default to last 24 hours
	}

	hours, err := strconv.Atoi(hoursStr)
	if err != nil {
		return echo.NewHTTPError(http.StatusBadRequest, "Invalid hours parameter")
	}

	limitStr := c.QueryParam("limit")
	if limitStr == "" {
		limitStr = "100"
	}

	limit, err := strconv.Atoi(limitStr)
	if err != nil {
		return echo.NewHTTPError(http.StatusBadRequest, "Invalid limit parameter")
	}

	// Calculate time threshold
	since := time.Now().Add(-time.Duration(hours) * time.Hour)

	ctx := context.Background()
	logs, err := h.queries.GetRecentLogs(ctx, database.GetRecentLogsParams{
		Timestamp: pgtype.Timestamp{Time: since, Valid: true},
		Limit:     int32(limit),
	})

	if err != nil {
		return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
	}

	return c.JSON(http.StatusOK, logs)
}

// GetLogsByLevel returns logs filtered by level
func (h *LogHandler) GetLogsByLevel(c echo.Context) error {
	level := c.Param("level")
	if level == "" {
		return echo.NewHTTPError(http.StatusBadRequest, "Level parameter required")
	}

	// Parse query parameters
	hoursStr := c.QueryParam("hours")
	if hoursStr == "" {
		hoursStr = "24"
	}

	hours, err := strconv.Atoi(hoursStr)
	if err != nil {
		return echo.NewHTTPError(http.StatusBadRequest, "Invalid hours parameter")
	}

	limitStr := c.QueryParam("limit")
	if limitStr == "" {
		limitStr = "100"
	}

	limit, err := strconv.Atoi(limitStr)
	if err != nil {
		return echo.NewHTTPError(http.StatusBadRequest, "Invalid limit parameter")
	}

	since := time.Now().Add(-time.Duration(hours) * time.Hour)

	ctx := context.Background()
	logs, err := h.queries.GetLogsByLevel(ctx, database.GetLogsByLevelParams{
		Level:     level,
		Timestamp: pgtype.Timestamp{Time: since, Valid: true},
		Limit:     int32(limit),
	})

	if err != nil {
		return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
	}

	return c.JSON(http.StatusOK, logs)
}

// GetLogsByUserID returns logs filtered by user_id field
func (h *LogHandler) GetLogsByUserID(c echo.Context) error {
	userID := c.Param("userId")
	if userID == "" {
		return echo.NewHTTPError(http.StatusBadRequest, "User ID parameter required")
	}

	hoursStr := c.QueryParam("hours")
	if hoursStr == "" {
		hoursStr = "24"
	}

	hours, err := strconv.Atoi(hoursStr)
	if err != nil {
		return echo.NewHTTPError(http.StatusBadRequest, "Invalid hours parameter")
	}

	limitStr := c.QueryParam("limit")
	if limitStr == "" {
		limitStr = "100"
	}

	limit, err := strconv.Atoi(limitStr)
	if err != nil {
		return echo.NewHTTPError(http.StatusBadRequest, "Invalid limit parameter")
	}

	since := time.Now().Add(-time.Duration(hours) * time.Hour)

	ctx := context.Background()
	logs, err := h.queries.GetLogsByUserID(ctx, database.GetLogsByUserIDParams{
		Fields:    []byte(userID),
		Timestamp: pgtype.Timestamp{Time: since, Valid: true},
		Limit:     int32(limit),
	})

	if err != nil {
		return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
	}

	return c.JSON(http.StatusOK, logs)
}
