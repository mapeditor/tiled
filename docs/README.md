# Generating the Tiled Documentation

This guide covers how to generate the Tiled documentation on various platforms using a virtual environment. You'll need Python 3 installed.

## Prerequisites

- Python 3
- pip (Python package installer)

## Setting Up a Virtual Environment

It's recommended to use a virtual environment to isolate the project dependencies. Here's how to set it up:

### Windows

1. Open Command Prompt and navigate to the project directory.
2. Create a virtual environment named ".venv":
   ```cmd
   python -m venv .venv
   ```
3. Activate the virtual environment:
   ```cmd
   .venv\Scripts\activate
   ```

### Linux (Ubuntu, Fedora, RHEL, CentOS) and macOS

1. Open a terminal and navigate to the project directory.
2. Create a virtual environment named ".venv":
   ```bash
   python3 -m venv .venv
   ```
3. Activate the virtual environment:
   ```bash
   source .venv/bin/activate
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