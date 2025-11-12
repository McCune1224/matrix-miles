# Development Guide

## Hot Reload with Air

Air is configured for automatic hot reloading during development. The server will automatically rebuild and restart when you modify `.go` files.

### Quick Start

```bash
cd strava-server
air
```

You should see:
```
  __    _   ___  
 / /\  | | | |_) 
/_/--\ |_| |_| \_ v1.61.7

watching .
building...
running...
✓ Database connected successfully
🚀 Server starting on :8080
📍 OAuth login: http://localhost:8080/auth/login
```

### What Gets Watched

Air watches these directories for changes:
- `cmd/` - Server entry point
- `internal/` - Handlers, database, and Strava client
- `pkg/` - Configuration
- `db/queries/` - SQL queries (remember to run `sqlc generate` after changes)

**Excluded directories:**
- `db/migrations/` - Schema migrations (not watched)
- `tmp/` - Build output directory
- `vendor/` - Dependencies

### Configuration

Air configuration is in `.air.toml`. Key settings:

```toml
[build]
  cmd = "go build -o ./tmp/main ./cmd/server"
  bin = "./tmp/main"
  include_ext = ["go", "tpl", "tmpl", "html", "env"]
  exclude_dir = ["assets", "tmp", "vendor", "testdata", "db/migrations"]
  delay = 1000  # Wait 1s after detecting changes before rebuilding
```

### Development Workflow

1. **Start Air:**
   ```bash
   air
   ```

2. **Make code changes** - Air will detect and rebuild automatically

3. **If you modify SQL queries:**
   ```bash
   # In another terminal
   sqlc generate
   # Air will detect the generated Go files and rebuild
   ```

4. **View build errors:**
   - Build errors are logged to `build-errors.log`
   - Or check the Air output in your terminal

5. **Stop the server:**
   - Press `Ctrl+C`
   - Air will clean up the `tmp/` directory

### Common Commands

```bash
# Start with hot reload
air

# Build only (no run)
go build -o server ./cmd/server

# Run without Air
./server

# Run tests with watch
air -c .air.test.toml  # (if you create a test config)

# Regenerate database code after SQL changes
sqlc generate
```

### Tips

- **Port already in use?** Kill existing server processes:
  ```bash
  pkill -f "./server"
  lsof -ti :8080 | xargs kill -9
  ```

- **Air not rebuilding?** Check `build-errors.log` for compilation errors

- **Database changes?** Remember to run migrations before starting:
  ```bash
  psql "your_connection_string" -f db/migrations/001_initial_schema.sql
  ```

- **Environment changes?** Air watches `.env` files, but you may need to restart manually for some changes

### Directory Structure During Development

```
strava-server/
├── .air.toml              # Air configuration
├── tmp/                   # Build output (gitignored)
│   └── main              # Compiled binary (rebuilt on changes)
├── build-errors.log       # Build errors (gitignored)
├── cmd/
├── internal/
├── pkg/
└── db/
```

### Troubleshooting

**Air not installed?**
```bash
go install github.com/air-verse/air@latest
# Make sure ~/go/bin is in your PATH
export PATH=$PATH:~/go/bin
```

**Build errors not showing?**
```bash
tail -f build-errors.log
```

**Server keeps restarting?**
- Check for infinite loops or file watchers creating files
- Air excludes `tmp/` by default to prevent rebuild loops

**Want to exclude more files?**
Edit `.air.toml`:
```toml
[build]
  exclude_dir = ["assets", "tmp", "vendor", "testdata", "your_dir"]
  exclude_regex = ["_test.go", ".*\\.generated\\.go"]
```

### Alternative: Manual Development

If you prefer not to use Air:

```bash
# Terminal 1: Watch for changes and rebuild
while true; do
  inotifywait -r -e modify,create,delete cmd/ internal/ pkg/
  go build -o server ./cmd/server
done

# Terminal 2: Manually restart server
./server
```

Or use `go run` directly (slower but no build step):
```bash
go run ./cmd/server/main.go
```

---

## Testing During Development

```bash
# Test health endpoint
curl http://localhost:8080/health

# Test OAuth (in browser)
open http://localhost:8080/auth/login

# Test API with your API key
curl -H "X-API-Key: $(grep ESP32_API_KEY .env | cut -d= -f2)" \
  http://localhost:8080/api/stats/1

# Test admin endpoint
curl -u admin:$(grep ADMIN_PASSWORD .env | cut -d= -f2) \
  -X POST http://localhost:8080/admin/sync/1
```

---

## Database Development

### Apply New Migrations

```bash
psql "$(grep DB_HOST .env | cut -d= -f2 | xargs -I {} echo postgres://...)" \
  -f db/migrations/002_your_migration.sql
```

### Regenerate Database Code

After modifying SQL queries in `db/queries/`:

```bash
sqlc generate
# Air will detect the changes and rebuild
```

### Reset Database (Development Only!)

```bash
# Drop all tables
psql "connection_string" -c "DROP SCHEMA public CASCADE; CREATE SCHEMA public;"

# Reapply migrations
psql "connection_string" -f db/migrations/001_initial_schema.sql
```

---

Happy coding! 🚀
