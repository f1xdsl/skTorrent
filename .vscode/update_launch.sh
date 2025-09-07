#!/bin/bash

APP_NAME="Passive"
CHANGELOG_FILE="Changelog.txt"
LAUNCH_FILE=".vscode/launch.json"

if [[ ! -f "$CHANGELOG_FILE" ]]; then
    echo "Файл $CHANGELOG_FILE не найден"
    exit 1
fi

version_line=$(grep -m1 "$APP_NAME v" "$CHANGELOG_FILE")
regex="$APP_NAME v([0-9]+(\.[0-9]+){1,4}) \(([0-9]{2}\.[0-9]{2}\.[0-9]{4})\)"

if [[ ! $version_line =~ $regex ]]; then
    echo "Не удалось извлечь версию из changelog"
    exit 1
fi

version="${BASH_REMATCH[1]}"
date="${BASH_REMATCH[3]}"
new_suffix="${APP_NAME}_${version}_${date}"

if [[ ! -f "$LAUNCH_FILE" ]]; then
    echo "Файл $LAUNCH_FILE не найден"
    exit 1
fi

tmp_file=$(mktemp)
sed -E "s/${APP_NAME}_[0-9]+(\.[0-9]+){1,4}_[0-9]{2}\.[0-9]{2}\.[0-9]{4}/${new_suffix}/g" "$LAUNCH_FILE" > "$tmp_file"
mv "$tmp_file" "$LAUNCH_FILE"

echo "launch.json обновлён до $new_suffix"

