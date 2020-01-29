import os.path


def find_sensitive_path(dir, insensitive_path):
    insensitive_path = insensitive_path.strip(os.path.sep)

    parts = insensitive_path.split(os.path.sep)
    next_name = parts[0]
    for name in os.listdir(dir):
        if next_name.lower() == name.lower():
            improved_path = os.path.join(dir, name)
            if len(parts) == 1:
                return improved_path
            else:
                return find_sensitive_path(improved_path, os.path.sep.join(parts[1:]))
    return None

