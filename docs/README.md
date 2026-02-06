# Generating the Tiled Documentation

This guide covers how to generate the Tiled documentation on various platforms using a virtual environment. You'll need Python 3 installed.

## Prerequisites

- Python 3
- pip (Python package installer)

## Setting Up a Virtual Environment

It's recommended to use a virtual environment to isolate the project dependencies. Here's how to set it up:

### Linux (Ubuntu, Fedora, RHEL, CentOS) and macOS

```bash
python3 -m venv .venv
source .venv/bin/activate
```

### Windows

```cmd
python -m venv .venv
.venv\Scripts\activate
```

## Installing Dependencies

After activating the virtual environment, install the required packages using the `requirements.txt` file:

```bash
pip install -r requirements.txt
```

## Generating the Documentation

With the virtual environment activated and dependencies installed, you can generate the documentation by running:

```bash
make html
```

On Windows, use:

```cmd
make.bat html
```

### Using a Different Language

Add `O=-Dlanguage=<language>` to the above command, where `<language>` should be replaced by a language code like `fr`, `de`, etc.

## Updating Translation Files

To update the translation files before tagging a new release:

1. Ensure your virtual environment is activated.
2. Generate the .pot files:
   ```bash
   make gettext
   ```
3. Update the .po files for German and French:
   ```bash
   sphinx-intl update -p _build/gettext -l de -l fr
   ```

Note: Currently, only German and French translations are hosted. Additional languages can be added if there's sufficient interest in maintaining those translations.

## Deactivating the Virtual Environment

When you're done working on the documentation, you can deactivate the virtual environment:

```bash
deactivate
```

## Requirements

The `requirements.txt` file in the documentation directory specifies the required packages and their versions. If you need to view or update these requirements, you can find them in this file.

Remember to activate the virtual environment each time you work on the documentation, and to update your virtual environment if `requirements.txt` changes:

```bash
pip install -r requirements.txt
```

This ensures you're always using the correct versions of the dependencies for building the documentation.
