package config

import (
	"fmt"
	"os"

	"github.com/joho/godotenv"
)

type Config struct {
	Server   ServerConfig
	Strava   StravaConfig
	Database DatabaseConfig
	Security SecurityConfig
}

type ServerConfig struct {
	Port   string
	Domain string
}

type StravaConfig struct {
	ClientID     string
	ClientSecret string
	RedirectURI  string
}

type DatabaseConfig struct {
	Host     string
	Port     string
	User     string
	Password string
	DBName   string
	SSLMode  string
}

type SecurityConfig struct {
	ESP32APIKey   string
	AdminUsername string
	AdminPassword string
}

func Load() (*Config, error) {
	// Load .env file if it exists
	_ = godotenv.Load()

	cfg := &Config{
		Server: ServerConfig{
			Port:   getEnv("PORT", "8080"),
			Domain: getEnv("DOMAIN", "http://localhost:8080"),
		},
		Strava: StravaConfig{
			ClientID:     getEnv("STRAVA_CLIENT_ID", ""),
			ClientSecret: getEnv("STRAVA_CLIENT_SECRET", ""),
			RedirectURI:  getEnv("STRAVA_REDIRECT_URI", ""),
		},
		Database: DatabaseConfig{
			Host:     getEnv("DB_HOST", "localhost"),
			Port:     getEnv("DB_PORT", "5432"),
			User:     getEnv("DB_USER", "strava_user"),
			Password: getEnv("DB_PASSWORD", ""),
			DBName:   getEnv("DB_NAME", "strava_db"),
			SSLMode:  getEnv("DB_SSLMODE", "disable"),
		},
		Security: SecurityConfig{
			ESP32APIKey:   getEnv("ESP32_API_KEY", ""),
			AdminUsername: getEnv("ADMIN_USERNAME", "admin"),
			AdminPassword: getEnv("ADMIN_PASSWORD", ""),
		},
	}

	if err := cfg.Validate(); err != nil {
		return nil, err
	}

	return cfg, nil
}

func (c *Config) Validate() error {
	if c.Strava.ClientID == "" {
		return fmt.Errorf("STRAVA_CLIENT_ID is required")
	}
	if c.Strava.ClientSecret == "" {
		return fmt.Errorf("STRAVA_CLIENT_SECRET is required")
	}
	if c.Database.Password == "" {
		return fmt.Errorf("DB_PASSWORD is required")
	}
	if c.Security.ESP32APIKey == "" {
		return fmt.Errorf("ESP32_API_KEY is required")
	}
	return nil
}

func (c *DatabaseConfig) DSN() string {
	return fmt.Sprintf(
		"host=%s port=%s user=%s password=%s dbname=%s sslmode=%s",
		c.Host, c.Port, c.User, c.Password, c.DBName, c.SSLMode,
	)
}

// ConnectionString returns PostgreSQL connection string in URL format for pgx
func (c *DatabaseConfig) ConnectionString() string {
	return fmt.Sprintf(
		"postgres://%s:%s@%s:%s/%s?sslmode=%s",
		c.User, c.Password, c.Host, c.Port, c.DBName, c.SSLMode,
	)
}

func getEnv(key, defaultValue string) string {
	if value := os.Getenv(key); value != "" {
		return value
	}
	return defaultValue
}
