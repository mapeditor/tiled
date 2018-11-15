/*
 *  This file is part of WinSparkle (http://winsparkle.org)
 *
 *  Copyright (C) 2009-2016 Vaclav Slavik
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef _winsparkle_h_
#define _winsparkle_h_

#include <stddef.h>

#include "winsparkle-version.h"

#if !defined(BUILDING_WIN_SPARKLE) && defined(_MSC_VER)
#pragma comment(lib, "WinSparkle.lib")
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BUILDING_WIN_SPARKLE
    #define WIN_SPARKLE_API __declspec(dllexport)
#else
    #define WIN_SPARKLE_API __declspec(dllimport)
#endif

/*--------------------------------------------------------------------------*
                       Initialization and shutdown
 *--------------------------------------------------------------------------*/

/**
    @name Initialization functions
 */
//@{

/**
    Starts WinSparkle.

    If WinSparkle is configured to check for updates on startup, proceeds
    to perform the check. You should only call this function when your app
    is initialized and shows its main window.

    @note This call doesn't block and returns almost immediately. If an
          update is available, the respective UI is shown later from a separate
          thread.

    @see win_sparkle_cleanup()
 */
WIN_SPARKLE_API void __cdecl win_sparkle_init();

/**
    Cleans up after WinSparkle.

    Should be called by the app when it's shutting down. Cancels any
    pending Sparkle operations and shuts down its helper threads.
 */
WIN_SPARKLE_API void __cdecl win_sparkle_cleanup();

//@}


/*--------------------------------------------------------------------------*
                             Language settings
*--------------------------------------------------------------------------*/

/**
@name Language settings

These functions set user interface language. They must be called before
win_sparkle_init() to have any effect. If none of them is called, WinSparkle
detects the UI language automatically.
*/
//@{

/**
    Sets UI language from its ISO code.

    This function must be called before win_sparkle_init().

    @param lang ISO 639 language code with an optional ISO 3116 country
                code, e.g. "fr", "pt-PT", "pt-BR" or "pt_BR", as used
                e.g. by ::GetThreadPreferredUILanguages() too.

    @since 0.5

    @see win_sparkle_set_langid()
*/
WIN_SPARKLE_API void __cdecl win_sparkle_set_lang(const char *lang);

/**
    Sets UI language from its Win32 LANGID code.

    This function must be called before win_sparkle_init().

    @param lang Language code (LANGID) as created by the MAKELANGID macro
                or returned by e.g. ::GetThreadUILanguage()

    @since 0.5

    @see win_sparkle_set_lang()
*/
WIN_SPARKLE_API void __cdecl win_sparkle_set_langid(unsigned short lang);

//@}

/*--------------------------------------------------------------------------*
                               Configuration
 *--------------------------------------------------------------------------*/

/**
    @name Configuration functions

    Functions for setting up WinSparkle.

    All functions in this category can only be called @em before the first
    call to win_sparkle_init()!

    Typically, the application would configure WinSparkle on startup and then
    call win_sparkle_init(), all from its main thread.
 */
//@{

/**
    Sets URL for the app's appcast.

    Only http and https schemes are supported.

    If this function isn't called by the app, the URL is obtained from
    Windows resource named "FeedURL" of type "APPCAST".

    @param url  URL of the appcast.
 */
WIN_SPARKLE_API void __cdecl win_sparkle_set_appcast_url(const char *url);

/**
    Sets application metadata.

    Normally, these are taken from VERSIONINFO/StringFileInfo resources,
    but if your application doesn't use them for some reason, using this
    function is an alternative.

    @param company_name  Company name of the vendor.
    @param app_name      Application name. This is both shown to the user
                         and used in HTTP User-Agent header.
    @param app_version   Version of the app, as string (e.g. "1.2" or "1.2rc1").

    @note @a company_name and @a app_name are used to determine the location
          of WinSparkle settings in registry.
          (HKCU\Software\<company_name>\<app_name>\WinSparkle is used.)

    @since 0.3

    @see win_sparkle_set_app_build_version();
 */
WIN_SPARKLE_API void __cdecl win_sparkle_set_app_details(const wchar_t *company_name,
                                                         const wchar_t *app_name,
                                                         const wchar_t *app_version);

/**
    Sets application build version number.

    This is the internal version number that is not normally shown to the user.
    It can be used for finer granularity that official release versions, e.g. for
    interim builds.

    If this function is called, then the provided *build* number is used for comparing
    versions; it is compared to the "version" attribute in the appcast and corresponds
    to OS X Sparkle's CFBundleVersion handling. If used, then the appcast must
    also contain the "shortVersionString" attribute with human-readable display
    version string. The version passed to win_sparkle_set_app_details()
    corresponds to this and is used for display.

    @since 0.4

    @see win_sparkle_set_app_details()
 */
WIN_SPARKLE_API void __cdecl win_sparkle_set_app_build_version(const wchar_t *build);

/**
    Set the registry path where settings will be stored.

    Normally, these are stored in
    "HKCU\Software\<company_name>\<app_name>\WinSparkle"
    but if your application needs to store the data elsewhere for
    some reason, using this function is an alternative.

    Note that @a path is relative to HKCU/HKLM root and the root is not part
    of it. For example:
    @code
    win_sparkle_set_registry_path("Software\\My App\\Updates");
    @endcode

    @param path  Registry path where settings will be stored.

    @since 0.3
 */
WIN_SPARKLE_API void __cdecl win_sparkle_set_registry_path(const char *path);

/**
    Sets whether updates are checked automatically or only through a manual call.

    If disabled, win_sparkle_check_update_with_ui() must be used explicitly.

    @param  state  1 to have updates checked automatically, 0 otherwise

    @since 0.4
 */
WIN_SPARKLE_API void __cdecl win_sparkle_set_automatic_check_for_updates(int state);

/**
    Gets the automatic update checking state

    @return  1 if updates are set to be checked automatically, 0 otherwise

    @note Defaults to 0 when not yet configured (as happens on first start).

    @since 0.4
 */
WIN_SPARKLE_API int __cdecl win_sparkle_get_automatic_check_for_updates();

/**
    Sets the automatic update interval.

    @param  interval The interval in seconds between checks for updates.
                     The minimum update interval is 3600 seconds (1 hour).

    @since 0.4
 */
WIN_SPARKLE_API void __cdecl win_sparkle_set_update_check_interval(int interval);

/**
    Gets the automatic update interval in seconds.

    Default value is one day.

    @since 0.4
 */
WIN_SPARKLE_API int __cdecl win_sparkle_get_update_check_interval();

/**
    Gets the time for the last update check.

    Default value is -1, indicating that the update check has never run.

    @since 0.4
*/
WIN_SPARKLE_API time_t __cdecl win_sparkle_get_last_check_time();

/// Callback type for win_sparkle_error_callback()
typedef void (__cdecl *win_sparkle_error_callback_t)();

/**
    Set callback to be called when the updater encounters an error.

    @since 0.5
*/
WIN_SPARKLE_API void __cdecl win_sparkle_set_error_callback(win_sparkle_error_callback_t callback);

/// Callback type for win_sparkle_can_shutdown_callback()
typedef int (__cdecl *win_sparkle_can_shutdown_callback_t)();

/**
    Set callback for querying the application if it can be closed.

    This callback will be called to ask the host if it's ready to shut down,
    before attempting to launch the installer. The callback returns TRUE if
    the host application can be safely shut down or FALSE if not (e.g. because
    the user has unsaved documents).

    @note There's no guarantee about the thread from which the callback is called,
          except that it certainly *won't* be called from the app's main thread.
          Make sure the callback is thread-safe.

    @since 0.4

    @see win_sparkle_set_shutdown_request_callback()
*/
WIN_SPARKLE_API void __cdecl win_sparkle_set_can_shutdown_callback(win_sparkle_can_shutdown_callback_t callback);


/// Callback type for win_sparkle_shutdown_request_callback()
typedef void (__cdecl *win_sparkle_shutdown_request_callback_t)();

/**
    Set callback for shutting down the application.

    This callback will be called to ask the host to shut down immediately after
    launching the installer. Its implementation should gracefully terminate the
    application.

    It will only be called if the call to the callback set with
    win_sparkle_set_can_shutdown_callback() returns TRUE.

    @note There's no guarantee about the thread from which the callback is called,
          except that it certainly *won't* be called from the app's main thread.
          Make sure the callback is thread-safe.

    @since 0.4

    @see win_sparkle_set_can_shutdown_callback()
*/
WIN_SPARKLE_API void __cdecl win_sparkle_set_shutdown_request_callback(win_sparkle_shutdown_request_callback_t);

/// Callback type for win_sparkle_did_find_update_callback()
typedef void(__cdecl *win_sparkle_did_find_update_callback_t)();

/**
    Set callback to be called when the updater did find an update.

    This is useful in combination with
    win_sparkle_check_update_with_ui_and_install() as it allows you to perform
    some action after WinSparkle checks for updates.

    @since 0.5

    @see win_sparkle_did_not_find_update_callback()
    @see win_sparkle_check_update_with_ui_and_install()
*/
WIN_SPARKLE_API void __cdecl win_sparkle_set_did_find_update_callback(win_sparkle_did_find_update_callback_t callback);

/// Callback type for win_sparkle_did_not_find_update_callback()
typedef void (__cdecl *win_sparkle_did_not_find_update_callback_t)();

/**
    Set callback to be called when the updater did not find an update.

    This is useful in combination with
    win_sparkle_check_update_with_ui_and_install() as it allows you to perform
    some action after WinSparkle checks for updates.

    @since 0.5

    @see win_sparkle_did_find_update_callback()
    @see win_sparkle_check_update_with_ui_and_install()
*/
WIN_SPARKLE_API void __cdecl win_sparkle_set_did_not_find_update_callback(win_sparkle_did_not_find_update_callback_t callback);

/// Callback type for win_sparkle_update_cancelled_callback()
typedef void (__cdecl *win_sparkle_update_cancelled_callback_t)();

/**
    Set callback to be called when the user cancels a download.

    This is useful in combination with
    win_sparkle_check_update_with_ui_and_install() as it allows you to perform
    some action when the installation is interrupted.

    @since 0.5

    @see win_sparkle_check_update_with_ui_and_install()
*/
WIN_SPARKLE_API void __cdecl win_sparkle_set_update_cancelled_callback(win_sparkle_update_cancelled_callback_t callback);

//@}


/*--------------------------------------------------------------------------*
                              Manual usage
 *--------------------------------------------------------------------------*/

/**
    @name Manually using WinSparkle
 */
//@{

/**
    Checks if an update is available, showing progress UI to the user.

    Normally, WinSparkle checks for updates on startup and only shows its UI
    when it finds an update. If the application disables this behavior, it
    can hook this function to "Check for updates..." menu item.

    When called, background thread is started to check for updates. A small
    window is shown to let the user know the progress. If no update is found,
    the user is told so. If there is an update, the usual "update available"
    window is shown.

    This function returns immediately.

    @note Because this function is intended for manual, user-initiated checks
          for updates, it ignores "Skip this version" even if the user checked
          it previously.

    @see win_sparkle_check_update_without_ui()
 */
WIN_SPARKLE_API void __cdecl win_sparkle_check_update_with_ui();

/**
    Checks if an update is available, showing progress UI to the user and
    immediately installing the update if one is available.

    This is useful for the case when users should almost always use the
    newest version of your software. When called, WinSparkle will check for
    updates showing a progress UI to the user. If an update is found the update
    prompt will be skipped and the update will be installed immediately.

    If your application expects to do something after checking for updates you
    may wish to use win_sparkle_set_did_not_find_update_callback() and
    win_sparkle_set_update_cancelled_callback().

    @since 0.5

    @see win_sparkle_set_did_find_update_callback()
    @see win_sparkle_set_update_cancelled_callback()
 */
WIN_SPARKLE_API void __cdecl win_sparkle_check_update_with_ui_and_install();

/**
    Checks if an update is available.

    No progress UI is shown to the user when checking. If an update is
    available, the usual "update available" window is shown; this function
    is *not* completely UI-less.

    Use with caution, it usually makes more sense to use the automatic update
    checks on interval option or manual check with visible UI.

    This function returns immediately.

    @note This function respects "Skip this version" choice by the user.

    @since 0.4

    @see win_sparkle_check_update_with_ui()
*/
WIN_SPARKLE_API void __cdecl win_sparkle_check_update_without_ui();

//@}

#ifdef __cplusplus
}
#endif

#endif // _winsparkle_h_
