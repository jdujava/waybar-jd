# Repository workflow

- Keep implementation changes and commits in this source repository. Do not
  export patches, source pins or file inventories into the dotfiles repo.
- Dotfiles contain desktop configuration and build helpers; source fixes belong
  on this repository's `desktop` branch.
- Before future implementation changes, fetch the official upstream and inspect
  new commits. Carry the custom series on its current default branch, using the
  symbolic upstream ref (currently `upstream/master`) rather than a pinned base.
- Keep feature commits coherent: fold follow-up fixes into their owning feature
  and keep documentation and maintenance in one final commit. Before regrouping
  history, preserve the original tip under a private ref or in a verified bundle.
  Verify complete tree equality at the same base before rebasing the lean series.
- Rebase on a temporary branch in an isolated worktree. Inspect overlapping
  upstream changes before resolving conflicts; preserve both custom semantics and
  new upstream behavior. Remove only demonstrably redundant custom hunks.
- Notify the user of upstream changes that affect desktop behavior,
  configuration, protocol support or build requirements. Report rebase conflicts
  and preserve the intended custom behavior when resolving them.
- Preserve existing user edits. Use an isolated checkout when needed for a
  rebase. Build and check the candidate before integration; keep checkpoint and
  conflict reports in private backups, not dotfiles. Do not push rewritten
  history unless the user requests it.
