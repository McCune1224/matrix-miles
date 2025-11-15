#!/bin/bash
# Cleanup duplicate Docker files from strava-server subdirectory
# These files are now at the root level

echo "🧹 Cleaning up duplicate Docker files from strava-server/"
echo ""

cd /home/mckusa/Code/matrix-miles/strava-server

FILES_TO_REMOVE=(
  "Dockerfile"
  "docker-compose.yml"
  "railway.json"
  ".dockerignore"
  "Makefile.docker"
)

for file in "${FILES_TO_REMOVE[@]}"; do
  if [ -f "$file" ]; then
    rm "$file"
    echo "✅ Removed: $file"
  else
    echo "⏭️  Not found: $file (already removed?)"
  fi
done

echo ""
echo "✨ Cleanup complete!"
echo ""
echo "Active Docker files are now at: /home/mckusa/Code/matrix-miles/"
echo "  - Dockerfile"
echo "  - docker-compose.yml"
echo "  - railway.json"
echo "  - .dockerignore"
