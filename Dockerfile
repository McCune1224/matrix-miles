# Multi-stage build for optimal image size
# This Dockerfile is at the root of matrix-miles and builds the strava-server subdirectory
FROM golang:1.23-alpine AS builder

# Install build dependencies
RUN apk add --no-cache git ca-certificates tzdata

# Set working directory to strava-server
WORKDIR /build/strava-server

# Copy go mod files first for better layer caching
COPY strava-server/go.mod strava-server/go.sum ./
RUN go mod download

# Copy strava-server source code
COPY strava-server/ ./

# Build the application
# CGO_ENABLED=0 for static binary, suitable for alpine
RUN CGO_ENABLED=0 GOOS=linux go build -ldflags="-w -s" -o server ./cmd/main.go

# Final stage - minimal runtime image
FROM alpine:latest

# Install runtime dependencies
RUN apk --no-cache add ca-certificates tzdata

# Create non-root user for security
RUN addgroup -g 1000 appuser && \
    adduser -D -u 1000 -G appuser appuser

WORKDIR /app

# Copy binary from builder
COPY --from=builder /build/strava-server/server .

# Copy database migrations (needed for potential runtime migrations)
COPY --from=builder /build/strava-server/db ./db

# Set ownership to non-root user
RUN chown -R appuser:appuser /app

# Switch to non-root user
USER appuser

# Expose port (Railway will use PORT environment variable)
EXPOSE 8080

# Health check
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD wget --no-verbose --tries=1 --spider http://localhost:${PORT:-8080}/health || exit 1

# Run the application
CMD ["./server"]
