This project is a Backup Tool that recursively copies a directory and its contents while preserving symlinks and file permissions. Instead of duplicating the content of regular files, the tool creates hard links to save storage space.
Features:
1. Recursively copies all files and directories.
2. Preserves symbolic links (recreates the symlink instead of copying the actual file).
3. Creates hard links for regular files instead of duplicating their content.
4. Maintains file permissions and attributes.

Core concepts: Files
