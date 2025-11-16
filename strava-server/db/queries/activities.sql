-- name: GetActivity :one
select *
from activities
where id = $1
limit 1
;

-- name: GetActivityByStravaID :one
select *
from activities
where strava_activity_id = $1
limit 1
;

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
select *
from activities
where user_id = $1
order by start_date desc
limit $2
offset $3
;

-- name: GetRecentActivities :many
select *
from activities
where user_id = $1
order by start_date desc
limit $2
;

-- name: GetActivitiesByDateRange :many
select *
from activities
where user_id = $1 and start_date >= $2 and start_date <= $3
order by start_date desc
;

-- name: GetActivityStats :one
select
    count(*) as total_activities,
    coalesce(sum(distance), 0) as total_distance,
    coalesce(sum(moving_time), 0) as total_time
from activities
where user_id = $1
;

-- name: GetCalendarData :many
select
    date(start_date) as activity_date,
    count(*) as count,
    round(sum(distance)::numeric, 2) as total_distance
from activities
where user_id = $1 and start_date >= $2 and start_date <= $3
group by date(start_date)
order by activity_date
;

-- name: DeleteActivity :exec
delete from activities
where id = $1
;

