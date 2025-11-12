package logger

import (
	"context"
	"encoding/json"
	"os"
	"sync"
	"time"

	"github.com/jackc/pgx/v5/pgtype"
	"github.com/mckusa/strava-server/internal/database"
	"go.uber.org/zap"
	"go.uber.org/zap/zapcore"
)

// LogEntry represents a buffered log entry
type LogEntry struct {
	Level      string
	Message    string
	Timestamp  time.Time
	Caller     string
	StackTrace string
	Fields     map[string]any
}

// DatabaseSink is a custom Zap sink that buffers logs for batch DB writes
type DatabaseSink struct {
	queries *database.Queries
	buffer  []LogEntry
	mu      sync.Mutex
	maxSize int
}

// NewDatabaseSink creates a new database sink
func NewDatabaseSink(queries *database.Queries, maxSize int) *DatabaseSink {
	return &DatabaseSink{
		queries: queries,
		buffer:  make([]LogEntry, 0, maxSize),
		maxSize: maxSize,
	}
}

// Write implements zapcore.WriteSyncer
func (ds *DatabaseSink) Write(p []byte) (n int, err error) {
	var entry map[string]any
	if err := json.Unmarshal(p, &entry); err != nil {
		return 0, err
	}

	logEntry := LogEntry{
		Level:     getString(entry, "level"),
		Message:   getString(entry, "msg"),
		Timestamp: time.Now(),
		Caller:    getString(entry, "caller"),
		Fields:    entry,
	}

	if stacktrace, ok := entry["stacktrace"].(string); ok {
		logEntry.StackTrace = stacktrace
	}

	ds.mu.Lock()
	ds.buffer = append(ds.buffer, logEntry)
	needsFlush := len(ds.buffer) >= ds.maxSize
	ds.mu.Unlock()

	if needsFlush {
		ds.Flush()
	}

	return len(p), nil
}

// Sync implements zapcore.WriteSyncer
func (ds *DatabaseSink) Sync() error {
	return ds.Flush()
}

// Flush writes all buffered logs to the database
func (ds *DatabaseSink) Flush() error {
	ds.mu.Lock()
	if len(ds.buffer) == 0 {
		ds.mu.Unlock()
		return nil
	}

	entries := make([]LogEntry, len(ds.buffer))
	copy(entries, ds.buffer)
	ds.buffer = ds.buffer[:0]
	ds.mu.Unlock()

	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()

	rows := make([]database.InsertLogBatchParams, len(entries))

	for i, entry := range entries {
		fieldsJSON, _ := json.Marshal(entry.Fields)
		rows[i] = database.InsertLogBatchParams{
			Level:      entry.Level,
			Message:    entry.Message,
			Timestamp:  pgtype.Timestamp{Time: entry.Timestamp, Valid: true},
			Caller:     pgtype.Text{String: entry.Caller, Valid: entry.Caller != ""},
			StackTrace: pgtype.Text{String: entry.StackTrace, Valid: entry.StackTrace != ""},
			Fields:     fieldsJSON,
		}
	}

	_, err := ds.queries.InsertLogBatch(ctx, rows)
	return err
}

func getString(m map[string]any, key string) string {
	if v, ok := m[key]; ok {
		if s, ok := v.(string); ok {
			return s
		}
	}
	return ""
}

// Logger wraps zap.Logger with database sink
type Logger struct {
	*zap.Logger
	dbSink *DatabaseSink
}

// NewLogger creates a new logger with console and database outputs
func NewLogger(queries *database.Queries, isDevelopment bool) (*Logger, error) {
	consoleEncoderConfig := zapcore.EncoderConfig{
		TimeKey:        "time",
		LevelKey:       "level",
		NameKey:        "logger",
		CallerKey:      "caller",
		MessageKey:     "msg",
		StacktraceKey:  "stacktrace",
		LineEnding:     zapcore.DefaultLineEnding,
		EncodeLevel:    zapcore.CapitalColorLevelEncoder,
		EncodeTime:     zapcore.ISO8601TimeEncoder,
		EncodeDuration: zapcore.SecondsDurationEncoder,
		EncodeCaller:   zapcore.ShortCallerEncoder,
	}

	dbEncoderConfig := zapcore.EncoderConfig{
		TimeKey:        "time",
		LevelKey:       "level",
		NameKey:        "logger",
		CallerKey:      "caller",
		MessageKey:     "msg",
		StacktraceKey:  "stacktrace",
		LineEnding:     zapcore.DefaultLineEnding,
		EncodeLevel:    zapcore.LowercaseLevelEncoder,
		EncodeTime:     zapcore.ISO8601TimeEncoder,
		EncodeDuration: zapcore.SecondsDurationEncoder,
		EncodeCaller:   zapcore.ShortCallerEncoder,
	}

	dbSink := NewDatabaseSink(queries, 100)

	level := zapcore.InfoLevel
	if isDevelopment {
		level = zapcore.DebugLevel
	}

	consoleCore := zapcore.NewCore(
		zapcore.NewConsoleEncoder(consoleEncoderConfig),
		zapcore.AddSync(os.Stdout),
		level,
	)

	dbCore := zapcore.NewCore(
		zapcore.NewJSONEncoder(dbEncoderConfig),
		zapcore.AddSync(dbSink),
		level,
	)

	core := zapcore.NewTee(consoleCore, dbCore)
	zapLogger := zap.New(core, zap.AddCaller(), zap.AddStacktrace(zapcore.ErrorLevel))

	return &Logger{
		Logger: zapLogger,
		dbSink: dbSink,
	}, nil
}

// Flush flushes the database sink
func (l *Logger) Flush() error {
	return l.dbSink.Flush()
}
