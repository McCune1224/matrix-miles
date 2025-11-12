-- name: GetActivity :one
SELECT * FROM activities
WHERE id = $1 LIMIT 1;

-- name: GetActivityByStravaID :one
SELECT * FROM activities
WHERE strava_activity_id = $1 LIMIT 1;

-- name: CreateActivity :one
INSERT INTO activities (
    user_id,
    strava_activity_id,
    name,
    type,
    distance,
    moving_time,
    elapsed_time,
    start_date,
    start_date_local
) VALUES (
    $1, $2, $3, $4, $5, $6, $7, $8, $9
) RETURNING *;

-- name: UpsertActivity :one
INSERT INTO activities (
    user_id,
    strava_activity_id,
    name,
    type,
    distance,
    moving_time,
    elapsed_time,
    start_date,
    start_date_local
) VALUES (
    $1, $2, $3, $4, $5, $6, $7, $8, $9
)
ON CONFLICT (strava_activity_id)
DO UPDATE SET
    name = EXCLUDED.name,
    type = EXCLUDED.type,
    distance = EXCLUDED.distance,
    moving_time = EXCLUDED.moving_time,
    elapsed_time = EXCLUDED.elapsed_time,
    start_date = EXCLUDED.start_date,
    start_date_local = EXCLUDED.start_date_local
RETURNING *;

-- name: ListActivitiesByUser :many
SELECT * FROM activities
WHERE user_id = $1
ORDER BY start_date DESC
LIMIT $2 OFFSET $3;

-- name: GetRecentActivities :many
SELECT * FROM activities
WHERE user_id = $1
ORDER BY start_date DESC
LIMIT $2;

-- name: GetActivitiesByDateRange :many
SELECT * FROM activities
WHERE user_id = $1
  AND start_date >= $2
  AND start_date <= $3
ORDER BY start_date DESC;

-- name: GetActivityStats :one
SELECT 
    COUNT(*) as total_activities,
    COALESCE(SUM(distance), 0) as total_distance,
    COALESCE(SUM(moving_time), 0) as total_time
FROM activities
WHERE user_id = $1;

-- name: GetCalendarData :many
SELECT 
    DATE(start_date) as activity_date,
    COUNT(*) as count,
    SUM(distance) as total_distance
FROM activities
WHERE user_id = $1
  AND start_date >= $2
  AND start_date <= $3
GROUP BY DATE(start_date)
ORDER BY activity_date;

-- name: DeleteActivity :exec
DELETE FROM activities
WHERE id = $1;
