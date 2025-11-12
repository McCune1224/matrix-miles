-- name: InsertLogBatch :copyfrom
INSERT INTO application_logs (
    level,
    message,
    timestamp,
    caller,
    stack_trace,
    fields
) VALUES (
    $1, $2, $3, $4, $5, $6
);

-- name: GetRecentLogs :many
SELECT * FROM application_logs
WHERE timestamp >= $1
ORDER BY timestamp DESC
LIMIT $2;

-- name: GetLogsByLevel :many
SELECT * FROM application_logs
WHERE level = $1
  AND timestamp >= $2
ORDER BY timestamp DESC
LIMIT $3;

-- name: GetLogsByUserID :many
SELECT * FROM application_logs
WHERE fields->>'user_id' = $1
  AND timestamp >= $2
ORDER BY timestamp DESC
LIMIT $3;

-- name: CleanupOldLogs :exec
DELETE FROM application_logs
WHERE timestamp < $1;
