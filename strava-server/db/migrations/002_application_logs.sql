-- Create application_logs table for storing structured logs
CREATE TABLE IF NOT EXISTS application_logs (
    id BIGSERIAL PRIMARY KEY,
    level VARCHAR(10) NOT NULL,  -- debug, info, warn, error, fatal
    message TEXT NOT NULL,
    timestamp TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    caller VARCHAR(255),  -- file:line
    stack_trace TEXT,  -- for errors
    fields JSONB,  -- structured fields (user_id, activity_id, etc.)
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Indexes for efficient querying
CREATE INDEX IF NOT EXISTS idx_logs_timestamp ON application_logs(timestamp DESC);
CREATE INDEX IF NOT EXISTS idx_logs_level ON application_logs(level);
CREATE INDEX IF NOT EXISTS idx_logs_fields ON application_logs USING GIN(fields);

-- Function to auto-delete logs older than 30 days
CREATE OR REPLACE FUNCTION cleanup_old_logs()
RETURNS void AS $$
BEGIN
    DELETE FROM application_logs 
    WHERE timestamp < NOW() - INTERVAL '30 days';
END;
$$ LANGUAGE plpgsql;
