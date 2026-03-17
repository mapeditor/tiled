"""Command queue for communication with Tiled extension via JSON file."""

import json
import os
import threading
from pathlib import Path
from typing import Any

from pydantic import BaseModel


class Command(BaseModel):
    """A command to be executed by the Tiled extension."""

    type: str
    params: dict[str, Any] = {}


class CommandQueue:
    """Thread-safe command queue that writes to a JSON file for Tiled to poll."""

    def __init__(self, command_file: Path | None = None):
        """Initialize the command queue.

        Args:
            command_file: Path to the commands.json file. Defaults to
                          ./commands.json in the mcp-server directory.
        """
        if command_file is None:
            # Default to commands.json in the extension directory
            # (where the Tiled extension polls for it)
            command_file = Path(__file__).parent.parent.parent / "extension" / "commands.json"
        self.command_file = Path(command_file)
        self._lock = threading.Lock()
        # Initialize with empty array
        self._write_commands([])

    def _write_commands(self, commands: list[dict[str, Any]]) -> None:
        """Write commands to the JSON file."""
        with open(self.command_file, "w") as f:
            json.dump(commands, f, indent=2)

    def _read_commands(self) -> list[dict[str, Any]]:
        """Read commands from the JSON file."""
        if not self.command_file.exists():
            return []
        try:
            with open(self.command_file) as f:
                return json.load(f)
        except (json.JSONDecodeError, FileNotFoundError):
            return []

    def push(self, command: Command) -> None:
        """Add a command to the queue.

        Args:
            command: The command to add.
        """
        with self._lock:
            commands = self._read_commands()
            commands.append(command.model_dump())
            self._write_commands(commands)

    def push_many(self, commands: list[Command]) -> None:
        """Add multiple commands to the queue atomically.

        Args:
            commands: List of commands to add.
        """
        with self._lock:
            existing = self._read_commands()
            existing.extend(cmd.model_dump() for cmd in commands)
            self._write_commands(existing)

    def clear(self) -> None:
        """Clear all commands from the queue."""
        with self._lock:
            self._write_commands([])

    def get_file_path(self) -> str:
        """Get the absolute path to the command file."""
        return str(self.command_file.absolute())


# Global command queue instance
_queue: CommandQueue | None = None


def get_command_queue() -> CommandQueue:
    """Get the global command queue instance."""
    global _queue
    if _queue is None:
        _queue = CommandQueue()
    return _queue


def push_command(command_type: str, **params: Any) -> None:
    """Convenience function to push a command to the global queue.

    Args:
        command_type: The type of command (e.g., "create_map", "set_tile")
        **params: Command parameters
    """
    queue = get_command_queue()
    queue.push(Command(type=command_type, params=params))
