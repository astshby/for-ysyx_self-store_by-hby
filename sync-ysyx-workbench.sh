#!/usr/bin/env bash

set -euo pipefail

repo_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
source_dir=${YSYX_SOURCE_DIR:-"$repo_dir/../ysyx-workbench"}
snapshot_dir="$repo_dir/ysyx-workbench"
commit_message=${1:-"snapshot: sync ysyx-workbench $(date '+%Y-%m-%d %H:%M:%S')"}

require_repo() {
  if ! git -C "$1" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    echo "error: not a Git repository: $1" >&2
    exit 1
  fi
}

copy_visible_files() {
  local source_repo=$1
  local destination=$2

  mkdir -p -- "$destination"
  git -C "$source_repo" ls-files -co --exclude-standard -z |
    rsync -a --from0 --files-from=- "$source_repo/" "$destination/"
}

require_repo "$repo_dir"
require_repo "$source_dir"
for nested_repo in am-kernels nvboard fceux-am; do
  require_repo "$source_dir/$nested_repo"
done

snapshot_tmp=$(mktemp -d)
trap 'rm -rf -- "$snapshot_tmp"' EXIT

copy_visible_files "$source_dir" "$snapshot_tmp"
for nested_repo in am-kernels nvboard fceux-am; do
  copy_visible_files "$source_dir/$nested_repo" "$snapshot_tmp/$nested_repo"
done

# AGENTS.md 在课程仓库中被忽略，但允许进入个人备份仓库。
if [[ -f "$source_dir/AGENTS.md" ]]; then
  cp -a -- "$source_dir/AGENTS.md" "$snapshot_tmp/AGENTS.md"
fi

mkdir -p -- "$snapshot_dir"
rsync -a --delete -- "$snapshot_tmp/" "$snapshot_dir/"

git -C "$repo_dir" add -A -- ysyx-workbench sync-ysyx-workbench.sh
if [[ -f "$snapshot_dir/AGENTS.md" ]]; then
  git -C "$repo_dir" add -f -- ysyx-workbench/AGENTS.md
fi

if git -C "$repo_dir" diff --cached --quiet; then
  echo "No source changes to commit."
else
  git -C "$repo_dir" commit -m "$commit_message"
fi

git -C "$repo_dir" push
