package main

import (
	"context"
	"fmt"
	"os"
	"os/signal"
	"time"

	"github.com/jackc/pgx/v5/pgxpool"
	"github.com/labstack/echo/v4"
	"github.com/labstack/echo/v4/middleware"
	"go.uber.org/zap"

	"github.com/mckusa/strava-server/internal/database"
	"github.com/mckusa/strava-server/internal/handlers"
	"github.com/mckusa/strava-server/internal/services"
	"github.com/mckusa/strava-server/internal/strava"
	"github.com/mckusa/strava-server/pkg/config"
	"github.com/mckusa/strava-server/pkg/logger"
)

func main() {
	// Load configuration
	cfg, err := config.Load()
	if err != nil {
		fmt.Printf("Failed to load configuration: %v\n", err)
		os.Exit(1)
	}

	// Connect to database using pgxpool
	dbpool, err := pgxpool.New(context.Background(), cfg.Database.ConnectionString())
	if err != nil {
		fmt.Printf("Failed to connect to database: %v\n", err)
		os.Exit(1)
	}
	defer dbpool.Close()

	// Test database connection
	if err := dbpool.Ping(context.Background()); err != nil {
		fmt.Printf("Failed to ping database: %v\n", err)
		os.Exit(1)
	}

	// Initialize sqlc queries
	queries := database.New(dbpool)

	// Initialize Zap logger
	log, err := logger.NewLogger(queries, true) // true = development mode
	if err != nil {
		fmt.Printf("Failed to initialize logger: %v\n", err)
		os.Exit(1)
	}
	defer log.Sync()

	log.Info("Database connected successfully")

	// Start log flusher (every 5 minutes)
	logFlusher := services.NewLogFlusher(log)
	if err := logFlusher.Start("*/5 * * * *"); err != nil {
		log.Fatal("Failed to start log flusher", zap.Error(err))
	}
	defer logFlusher.Stop()

	// Initialize Strava client
	stravaClient := strava.NewClient(
		cfg.Strava.ClientID,
		cfg.Strava.ClientSecret,
		cfg.Strava.RedirectURI,
	)

	// Initialize handlers
	oauthHandler := handlers.NewOAuthHandler(queries, stravaClient)
	apiHandler := handlers.NewAPIHandler(queries, stravaClient)
	logHandler := handlers.NewLogHandler(queries)

	// Initialize Echo
	e := echo.New()
	e.HidePort = true
	e.HideBanner = true

	// Middleware
	e.Use(zapRequestLogger(log))
	e.Use(middleware.Recover())
	e.Use(middleware.CORS())

	// Routes

	// Health check
	e.GET("/health", func(c echo.Context) error {
		return c.JSON(200, map[string]string{
			"status": "ok",
			"time":   time.Now().Format(time.RFC3339),
		})
	})

	// OAuth routes (public)
	e.GET("/auth/login", oauthHandler.HandleLogin)
	e.GET("/auth/callback", oauthHandler.HandleCallback)

	// API routes (protected with API key)
	api := e.Group("/api")
	api.Use(apiKeyMiddleware(cfg.Security.ESP32APIKey))
	api.GET("/activities/recent/:userId", apiHandler.GetRecentActivities)
	api.GET("/activities/calendar/:userId/:year/:month", apiHandler.GetCalendarData)
	api.GET("/stats/:userId", apiHandler.GetUserStats)

	// Admin routes (protected with basic auth)
	admin := e.Group("/admin")
	admin.Use(middleware.BasicAuth(func(username, password string, c echo.Context) (bool, error) {
		return username == cfg.Security.AdminUsername && password == cfg.Security.AdminPassword, nil
	}))
	admin.POST("/sync/:userId", apiHandler.SyncActivities)

	// Log viewing routes
	admin.GET("/logs", logHandler.GetRecentLogs)
	admin.GET("/logs/level/:level", logHandler.GetLogsByLevel)
	admin.GET("/logs/user/:userId", logHandler.GetLogsByUserID)

	// Start server
	addr := fmt.Sprintf(":%s", cfg.Server.Port)
	log.Info("Server starting",
		zap.String("address", addr),
		zap.String("oauth_url", cfg.Server.Domain+"/auth/login"),
	)

	// Graceful shutdown
	go func() {
		if err := e.Start(addr); err != nil {
			log.Error("Server error", zap.Error(err))
		}
	}()

	// Wait for interrupt signal
	quit := make(chan os.Signal, 1)
	signal.Notify(quit, os.Interrupt)
	<-quit

	log.Info("Shutting down server...")
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()

	if err := e.Shutdown(ctx); err != nil {
		log.Error("Server shutdown error", zap.Error(err))
	}

	log.Info("Server stopped")
}

// apiKeyMiddleware validates the API key for ESP32 requests
func apiKeyMiddleware(expectedKey string) echo.MiddlewareFunc {
	return func(next echo.HandlerFunc) echo.HandlerFunc {
		return func(c echo.Context) error {
			apiKey := c.Request().Header.Get("X-API-Key")
			if apiKey == "" || apiKey != expectedKey {
				return echo.NewHTTPError(401, "Invalid or missing API key")
			}
			return next(c)
		}
	}
}

// zapRequestLogger is a middleware that logs HTTP requests using Zap
func zapRequestLogger(log *logger.Logger) echo.MiddlewareFunc {
	return func(next echo.HandlerFunc) echo.HandlerFunc {
		return func(c echo.Context) error {
			start := time.Now()

			err := next(c)

			req := c.Request()
			res := c.Response()

			fields := []zap.Field{
				zap.String("method", req.Method),
				zap.String("path", req.URL.Path),
				zap.Int("status", res.Status),
				zap.Duration("duration", time.Since(start)),
				zap.String("ip", c.RealIP()),
			}

			if err != nil {
				fields = append(fields, zap.Error(err))
				log.Error("Request failed", fields...)
			} else {
				log.Info("Request completed", fields...)
			}

			return err
		}
	}
}
