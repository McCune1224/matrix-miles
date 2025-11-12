package services

import (
	"github.com/mckusa/strava-server/pkg/logger"
	"github.com/robfig/cron/v3"
	"go.uber.org/zap"
)

type LogFlusher struct {
	logger *logger.Logger
	cron   *cron.Cron
}

func NewLogFlusher(log *logger.Logger) *LogFlusher {
	return &LogFlusher{
		logger: log,
		cron:   cron.New(),
	}
}

// Start begins the periodic log flushing
func (lf *LogFlusher) Start(schedule string) error {
	// Default: every 5 minutes
	if schedule == "" {
		schedule = "*/5 * * * *"
	}

	_, err := lf.cron.AddFunc(schedule, func() {
		lf.logger.Info("Flushing logs to database")
		if err := lf.logger.Flush(); err != nil {
			lf.logger.Error("Failed to flush logs", zap.Error(err))
		}
	})

	if err != nil {
		return err
	}

	lf.cron.Start()
	lf.logger.Info("Log flusher started", zap.String("schedule", schedule))
	return nil
}

// Stop stops the log flusher and flushes remaining logs
func (lf *LogFlusher) Stop() error {
	lf.logger.Info("Stopping log flusher")
	lf.cron.Stop()
	return lf.logger.Flush()
}
