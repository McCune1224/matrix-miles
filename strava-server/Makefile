.PHONY: help build run test clean docker-build docker-run docker-up docker-down test-api test-health dev

help: ## Show this help message
	@echo 'Usage: make [target]'
	@echo ''
	@echo 'Available targets:'
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "  %-20s %s\n", $$1, $$2}' $(MAKEFILE_LIST)

build: ## Build the Go binary
	go build -o server ./cmd/main.go

run: ## Run the application locally
	go run ./cmd/main.go

dev: ## Run with hot reload using Air
	air

test: ## Run Go tests
	go test -v ./...

clean: ## Clean build artifacts
	rm -f server
	rm -rf tmp/

# Docker commands
docker-build: ## Build Docker image (run from project root)
	cd .. && docker build -t strava-server:latest .

docker-run: ## Run Docker container (requires .env file in root)
	docker run --rm -p 8080:8080 --env-file ../.env strava-server:latest

docker-up: ## Start services with docker-compose (run from project root)
	cd .. && docker-compose up --build

docker-down: ## Stop docker-compose services (run from project root)
	cd .. && docker-compose down

docker-logs: ## View docker-compose logs
	cd .. && docker-compose logs -f

# API Testing
test-health: ## Quick health check
	@echo "Testing health endpoint..."
	@curl -s http://localhost:8080/health | jq '.' || curl -s http://localhost:8080/health

test-api: ## Run full API test suite
	@chmod +x test-api.sh
	@./test-api.sh

test-api-remote: ## Test API on production/remote server (usage: make test-api-remote URL=https://your-server.com)
	@chmod +x test-api.sh
	@./test-api.sh $(URL)

# Quick endpoint tests
test-activities: ## Test activities endpoint (requires USER_ID env var)
	@curl -s -H "X-API-Key: $(ESP32_API_KEY)" \
		http://localhost:8080/api/activities/recent/$(USER_ID) | jq '.'

test-stats: ## Test stats endpoint (requires USER_ID env var)
	@curl -s -H "X-API-Key: $(ESP32_API_KEY)" \
		http://localhost:8080/api/stats/$(USER_ID) | jq '.'

test-logs: ## Test admin logs endpoint
	@curl -s -u $(ADMIN_USERNAME):$(ADMIN_PASSWORD) \
		http://localhost:8080/admin/logs | jq '.'

test-sync: ## Trigger activity sync (requires USER_ID env var)
	@curl -s -X POST -u $(ADMIN_USERNAME):$(ADMIN_PASSWORD) \
		http://localhost:8080/admin/sync/$(USER_ID) | jq '.'

# Database
migrate-up: ## Run database migrations up
	@echo "Running migrations..."
	@if [ -z "$$DATABASE_URL" ]; then \
		echo "Error: DATABASE_URL not set"; \
		exit 1; \
	fi
	@psql $$DATABASE_URL -f db/migrations/001_initial_schema.sql
	@psql $$DATABASE_URL -f db/migrations/002_application_logs.sql
	@echo "Migrations completed"

generate: ## Generate sqlc code
	sqlc generate

deps: ## Download Go dependencies
	go mod download
	go mod tidy

lint: ## Run linter (requires golangci-lint)
	golangci-lint run

# Railway
railway-logs: ## View Railway logs (requires Railway CLI)
	railway logs

railway-shell: ## Open shell in Railway container
	railway run bash

railway-deploy: ## Deploy to Railway
	git push railway main

.DEFAULT_GOAL := help
