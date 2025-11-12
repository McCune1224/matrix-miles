-- name: GetUserByID :one
SELECT * FROM users
WHERE id = $1 LIMIT 1;

-- name: GetUserByStravaID :one
SELECT * FROM users
WHERE strava_user_id = $1 LIMIT 1;

-- name: CreateUser :one
INSERT INTO users (
    strava_user_id,
    username,
    access_token,
    refresh_token,
    token_expires_at
) VALUES (
    $1, $2, $3, $4, $5
) RETURNING *;

-- name: UpdateUserTokens :one
UPDATE users
SET access_token = $2,
    refresh_token = $3,
    token_expires_at = $4,
    updated_at = CURRENT_TIMESTAMP
WHERE id = $1
RETURNING *;

-- name: UpsertUser :one
INSERT INTO users (
    strava_user_id,
    username,
    access_token,
    refresh_token,
    token_expires_at
) VALUES (
    $1, $2, $3, $4, $5
)
ON CONFLICT (strava_user_id)
DO UPDATE SET
    username = EXCLUDED.username,
    access_token = EXCLUDED.access_token,
    refresh_token = EXCLUDED.refresh_token,
    token_expires_at = EXCLUDED.token_expires_at,
    updated_at = CURRENT_TIMESTAMP
RETURNING *;

-- name: ListUsers :many
SELECT * FROM users
ORDER BY created_at DESC;

-- name: DeleteUser :exec
DELETE FROM users
WHERE id = $1;
