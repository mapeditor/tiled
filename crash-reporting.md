---
layout: default
---

# Crash Reporting

In an effort to make Tiled more stable, basic crash reporting support is being
added. This feature is disabled by default, but since the provided information
is *incredibly useful* in determining both the stability of the software as
well as for fixing any stability issues, Tiled builds with this feature will
explicitly request it to be enabled.

Please consider enabling this feature when available. If you have any doubts
about what information is being submitted and how it is used, please read the
details below. Crash reporting can be disabled at any time in the Preferences.

## What is Collected?

First of all, no personal data, such as names, IP addresses, MAC addresses, or
project and path names are collected.

When nothing goes wrong, the only thing that's being collected is an entirely
anonymous session count. The purpose of this is mainly to determine the
stability of a release (ie. what percentage of sessions are crash-free?).

In case of a crash, the following information is sent as part of the crash
report:

* The name of the operating system (Windows, macOS or Linux).

* The version of Tiled, Sentry Native SDK and the operating system.

* The processor architecture (for example, x86\_64).

* A "minidump" of the memory used by Tiled, based on which a *stack trace* is
  computed. The stack trace shows the list of function calls that lead up to
  the crash and is often the most useful bit of information when trying to fix
  the issue.

## How is the Information Used?

The collected information is being stored by [Sentry][sentry] and
is only accessible by Tiled's maintainer, [Thorbjørn Lindeijer][github-bjorn].
It is used for the sole purpose of making Tiled more stable.

## How Else Can I Help?

When a crash happened, please additionally check if there is already a issue
reported about it [on GitHub][github-bugs]. Please open a new issue when you
can't find an existing report covering your problem and describe what you did
to trigger the crash.

[sentry]: https://sentry.io/
[github-bjorn]: https://github.com/bjorn
[github-bugs]: https://github.com/mapeditor/tiled/issues?q=is%3Aopen+is%3Aissue+label%3Abug
