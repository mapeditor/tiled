#!/usr/bin/env python3
"""
Check Patreon sponsors in `_data/sponsors.yml` against the Tiled campaign.

Mirrors the logic of check-sponsors.py (OpenCollective), but using Patreon
API v2. Requires a Creator Access Token in the PATREON_ACCESS_TOKEN env var
(create one at https://www.patreon.com/portal/registration/register-clients).

Checks:
- Activity status: member.patron_status == "active_patron"
- Payment health: member.last_charge_status (warns when != "Paid")
- Slot counts: number of active patrons matching each YAML entry
- Missing-sponsor detection: active patrons with currently_entitled_amount_cents
  >= BANNER_MIN_AMOUNT_CENTS that are absent from sponsors.yml

Usage:
    PATREON_ACCESS_TOKEN=... python3 _source/check-patreon-sponsors.py

Exit codes:
    0 - all checked sponsors are active and aligned
    1 - issues found
    2 - script/runtime/API error
"""

from __future__ import annotations

import json
import os
import re
import sys
import urllib.error
import urllib.parse
import urllib.request
from collections import Counter, defaultdict
from typing import Any

import yaml

API_BASE = "https://www.patreon.com/api/oauth2/v2"
SPONSORS_FILE = os.path.join(
    os.path.dirname(os.path.abspath(__file__)), "..", "_data", "sponsors.yml"
)

# Banner-level threshold. Patreon returns the campaign's currency, which for
# Tiled is the campaign's pledge currency (typically USD). Keep this aligned
# with check-sponsors.py.
BANNER_MIN_AMOUNT_CENTS = 10_000  # $100.00 / mo equivalent

USE_COLOR = hasattr(sys.stdout, "isatty") and sys.stdout.isatty()


def _c(code: str, text: str) -> str:
    return f"\033[{code}m{text}\033[0m" if USE_COLOR else text


def green(text: str) -> str:
    return _c("32", text)


def yellow(text: str) -> str:
    return _c("33", text)


def red(text: str) -> str:
    return _c("31", text)


def bold(text: str) -> str:
    return _c("1", text)


# Patron URL forms seen in sponsors.yml:
#   https://www.patreon.com/user/?u=473662
#   https://www.patreon.com/user?u=2508874
#   https://www.patreon.com/user/creators?u=43447498
#   https://www.patreon.com/anti666
#   https://www.patreon.com/OnlineCasinosSpelen
PATREON_HOST_RE = re.compile(r"^https?://(www\.)?patreon\.com/", re.IGNORECASE)


def extract_patreon_identity(sponsor_url: str | None) -> tuple[str, str] | None:
    """Return ('id', '<numeric>') or ('vanity', '<slug-lower>'), else None."""
    if not sponsor_url or not PATREON_HOST_RE.match(sponsor_url):
        return None

    parsed = urllib.parse.urlparse(sponsor_url)
    qs = urllib.parse.parse_qs(parsed.query)
    uid = qs.get("u", [None])[0]
    if uid and uid.isdigit():
        return ("id", uid)

    # Path-based vanity: take the first non-empty segment, unless it's "user"
    # (which always carries ?u= and was handled above).
    parts = [p for p in parsed.path.split("/") if p]
    if parts and parts[0].lower() != "user":
        return ("vanity", parts[0].lower())

    return None


def load_sponsors() -> list[dict[str, Any]]:
    if not os.path.isfile(SPONSORS_FILE):
        print(red(f"ERROR: sponsors file not found: {SPONSORS_FILE}"), file=sys.stderr)
        sys.exit(2)

    with open(SPONSORS_FILE, "r", encoding="utf-8") as fh:
        data = yaml.safe_load(fh)

    if not isinstance(data, list):
        print(red("ERROR: sponsors.yml did not parse as a list"), file=sys.stderr)
        sys.exit(2)

    return data


def api_get(path: str, token: str, params: dict[str, str] | None = None) -> dict[str, Any]:
    url = f"{API_BASE}{path}"
    if params:
        url += "?" + urllib.parse.urlencode(params, safe=",[]")
    req = urllib.request.Request(
        url,
        headers={
            "Authorization": f"Bearer {token}",
            "Accept": "application/json",
            "User-Agent": "check-patreon-sponsors/1.0",
        },
    )
    try:
        with urllib.request.urlopen(req, timeout=45) as resp:
            return json.loads(resp.read().decode("utf-8"))
    except urllib.error.HTTPError as exc:
        body = exc.read().decode("utf-8", errors="replace")
        print(red(f"ERROR: Patreon API {exc.code} on {path}: {body}"), file=sys.stderr)
        sys.exit(2)
    except urllib.error.URLError as exc:
        print(red(f"ERROR: Patreon API request failed: {exc}"), file=sys.stderr)
        sys.exit(2)


def fetch_campaign_id(token: str) -> str:
    data = api_get("/campaigns", token)
    campaigns = data.get("data") or []
    if not campaigns:
        print(red("ERROR: token has no campaigns"), file=sys.stderr)
        sys.exit(2)
    if len(campaigns) > 1:
        print(
            yellow(
                f"WARNING: token has {len(campaigns)} campaigns; using the first ({campaigns[0].get('id')})"
            )
        )
    return campaigns[0]["id"]


def fetch_all_members(token: str, campaign_id: str) -> tuple[list[dict[str, Any]], dict[str, dict[str, Any]]]:
    """Return (members, users_by_id)."""
    members: list[dict[str, Any]] = []
    users_by_id: dict[str, dict[str, Any]] = {}

    params = {
        "include": "user",
        "fields[member]": ",".join(
            [
                "full_name",
                "patron_status",
                "last_charge_status",
                "last_charge_date",
                "currently_entitled_amount_cents",
                "lifetime_support_cents",
                "pledge_relationship_start",
            ]
        ),
        "fields[user]": "vanity,url,full_name",
        "page[count]": "100",
    }

    cursor: str | None = None
    print("Fetching Patreon members (paged) …")
    while True:
        if cursor:
            params["page[cursor]"] = cursor
        data = api_get(f"/campaigns/{campaign_id}/members", token, params)
        members.extend(data.get("data") or [])
        for inc in data.get("included") or []:
            if inc.get("type") == "user":
                users_by_id[inc["id"]] = inc

        next_link = (data.get("links") or {}).get("next")
        if not next_link:
            break
        # Patreon returns full URL; parse out page[cursor]
        nq = urllib.parse.parse_qs(urllib.parse.urlparse(next_link).query)
        cursor = nq.get("page[cursor]", [None])[0]
        if not cursor:
            break

    print(f"  → received {len(members)} members\n")
    return members, users_by_id


class _NoRedirect(urllib.request.HTTPRedirectHandler):
    def redirect_request(self, *args, **kwargs):  # type: ignore[override]
        return None


_no_redirect_opener = urllib.request.build_opener(_NoRedirect)


def probe_vanity(vanity: str) -> str | None:
    """Resolve a Patreon vanity slug to a numeric user ID. Patreon's v2 API
    often returns an empty `user.vanity` even when patreon.com/<vanity>
    works, so we bridge by walking the redirect chain from the public
    profile URL — Patreon redirects vanities to /profile?u=<id> before
    the final page, which is much more reliable than scraping the HTML.
    """
    location = f"/{urllib.parse.quote(vanity)}"
    seen: set[str] = set()
    for _ in range(8):  # bounded redirect chain
        if location in seen:
            break
        seen.add(location)

        m = re.search(r"[?&]u=(\d+)", location)
        if m:
            return m.group(1)

        url = urllib.parse.urljoin("https://www.patreon.com/", location)
        req = urllib.request.Request(
            url,
            headers={
                "User-Agent": "Mozilla/5.0 (check-patreon-sponsors/1.0)",
                "Accept": "text/html",
            },
        )
        try:
            resp = _no_redirect_opener.open(req, timeout=20)
        except urllib.error.HTTPError as exc:
            # Redirects come through here when the no-redirect handler
            # short-circuits them; pick up Location and continue.
            new_loc = exc.headers.get("Location") if exc.headers else None
            if exc.code in (301, 302, 303, 307, 308) and new_loc:
                location = new_loc
                continue
            return None
        except urllib.error.URLError:
            return None

        new_loc = resp.headers.get("Location")
        if new_loc and resp.status in (301, 302, 303, 307, 308):
            location = new_loc
            continue

        # Terminal response — fall back to scanning the body.
        try:
            html = resp.read().decode("utf-8", errors="ignore")
        except Exception:
            return None
        for pat in (
            r'"type"\s*:\s*"user"\s*,\s*"id"\s*:\s*"(\d+)"',
            r'patreon\.com/(?:user|profile)\?u=(\d+)',
            r'"creator_id"\s*:\s*"?(\d+)"?',
        ):
            m = re.search(pat, html)
            if m:
                return m.group(1)
        return None
    return None


def cents(value: int | None) -> str:
    if value is None:
        return "?"
    return f"${value / 100:.2f}"


def main() -> None:
    token = os.environ.get("PATREON_ACCESS_TOKEN")
    if not token:
        print(
            red("ERROR: PATREON_ACCESS_TOKEN env var is required."),
            file=sys.stderr,
        )
        print(
            "Create a Creator Access Token at "
            "https://www.patreon.com/portal/registration/register-clients",
            file=sys.stderr,
        )
        sys.exit(2)

    sponsors = load_sponsors()

    # YAML Patreon entries, keyed by ('id'|'vanity', value).
    yml_entries: list[tuple[dict[str, Any], tuple[str, str]]] = []
    yml_count_by_key: Counter[tuple[str, str]] = Counter()
    for s in sponsors:
        ident = extract_patreon_identity(s.get("sponsor"))
        if ident:
            yml_entries.append((s, ident))
            yml_count_by_key[ident] += 1

    if not yml_entries:
        print("No Patreon sponsor entries found in sponsors.yml.")
        sys.exit(0)

    campaign_id = fetch_campaign_id(token)
    members, users_by_id = fetch_all_members(token, campaign_id)

    # The API's `user.vanity` is unreliable: many users with working
    # patreon.com/<vanity> URLs come back with vanity=null. To bridge YAML
    # entries that link by vanity, resolve each unique vanity to a numeric
    # user ID via the public profile page, then rewrite the key.
    vanity_yml_keys = {k[1] for _, k in yml_entries if k[0] == "vanity"}
    api_known_user_ids = {m_id for m_id in users_by_id.keys()}
    # Also collect user IDs that members carry directly (in case `users_by_id`
    # missed any, which shouldn't happen but is cheap to guard against).
    for m in members:
        ud = ((m.get("relationships") or {}).get("user") or {}).get("data") or {}
        if ud.get("id"):
            api_known_user_ids.add(ud["id"])

    vanity_to_id: dict[str, str] = {}
    for vanity in sorted(vanity_yml_keys):
        # If a member already exposed this vanity, no need to probe.
        already_keyed = any(
            (
                ((users_by_id.get(
                    ((m.get("relationships") or {}).get("user") or {}).get("data", {}).get("id") or ""
                ) or {}).get("attributes") or {}).get("vanity") or ""
            ).lower() == vanity
            for m in members
        )
        if already_keyed:
            continue
        resolved = probe_vanity(vanity)
        if resolved:
            vanity_to_id[vanity] = resolved
            note = "matches active member" if resolved in api_known_user_ids else "not in campaign members"
            print(yellow(f"  vanity '{vanity}' → user id {resolved} ({note})"))
        else:
            print(red(f"  vanity '{vanity}' could not be resolved via public probe"))
    if vanity_to_id:
        print()

    # Rewrite YAML entry keys for resolved vanities so they collide with the
    # ('id', N) buckets the member loop will build.
    rewritten_yml_entries: list[tuple[dict[str, Any], tuple[str, str]]] = []
    rewritten_count: Counter[tuple[str, str]] = Counter()
    for s, k in yml_entries:
        if k[0] == "vanity" and k[1] in vanity_to_id:
            k = ("id", vanity_to_id[k[1]])
        rewritten_yml_entries.append((s, k))
        rewritten_count[k] += 1
    yml_entries = rewritten_yml_entries
    yml_count_by_key = rewritten_count

    # Build per-key state.
    rows_by_key: dict[tuple[str, str], list[dict[str, Any]]] = defaultdict(list)
    active_count_by_key: Counter[tuple[str, str]] = Counter()
    amount_by_key: dict[tuple[str, str], int] = defaultdict(int)
    name_by_key: dict[tuple[str, str], str] = {}

    # Track ALL active banner-level members for missing-sponsor detection.
    member_keys: list[tuple[dict[str, Any], list[tuple[str, str]]]] = []

    for m in members:
        attrs = m.get("attributes") or {}
        user_ref = ((m.get("relationships") or {}).get("user") or {}).get("data") or {}
        user_id = user_ref.get("id")
        user = users_by_id.get(user_id) if user_id else None
        vanity = ((user or {}).get("attributes") or {}).get("vanity")

        keys: list[tuple[str, str]] = []
        if user_id:
            keys.append(("id", user_id))
        if vanity:
            keys.append(("vanity", vanity.lower()))

        member_keys.append((m, keys))

        is_active = attrs.get("patron_status") == "active_patron"
        amount = attrs.get("currently_entitled_amount_cents") or 0
        name = attrs.get("full_name") or (user or {}).get("attributes", {}).get("full_name") or "(unknown)"

        for k in keys:
            rows_by_key[k].append(m)
            if is_active:
                active_count_by_key[k] += 1
            if amount > amount_by_key[k]:
                amount_by_key[k] = amount
            name_by_key.setdefault(k, name)

    yml_keys = sorted({k for _, k in yml_entries})

    print(
        bold(
            f"Checking {len(yml_entries)} Patreon sponsor entries "
            f"({len(yml_keys)} unique patrons):\n"
        )
    )
    print(f"  {'Name':<60} {'Key':<32} {'Status'}")
    print(f"  {'─' * 60} {'─' * 32} {'─' * 30}")

    inactive: list[tuple[str, str]] = []
    not_found: list[tuple[str, str]] = []
    bad_charge: list[tuple[str, str]] = []
    active: list[tuple[str, str]] = []

    for key in yml_keys:
        sponsor_obj = next(s for s, k in yml_entries if k == key)
        name = sponsor_obj.get("name", "(unnamed)")
        display = (name[:57] + "…") if len(name) > 58 else name
        key_str = f"{key[0]}={key[1]}"

        rows = rows_by_key.get(key, [])
        if not rows:
            status = red("NOT FOUND")
            not_found.append(key)
        else:
            row = rows[0]
            attrs = row.get("attributes") or {}
            if attrs.get("patron_status") == "active_patron":
                if attrs.get("last_charge_status") in (None, "Paid"):
                    status = green("ACTIVE")
                    active.append(key)
                else:
                    status = yellow(f"ACTIVE / charge={attrs.get('last_charge_status')}")
                    bad_charge.append(key)
            else:
                status = yellow(attrs.get("patron_status") or "INACTIVE")
                inactive.append(key)

        print(f"  {display:<60} {key_str:<32} {status}")

    print()
    print(bold("Summary"))
    print(f"  YAML Patreon entries: {len(yml_entries)} ({len(yml_keys)} unique patrons)")
    print(f"  {green('Active')}:           {len(active)}")
    print(f"  {yellow('Inactive')}:         {len(inactive)}")
    print(f"  {yellow('Active w/ charge issue')}: {len(bad_charge)}")
    print(f"  {red('Not found')}:        {len(not_found)}")

    has_problems = False

    if inactive:
        has_problems = True
        print()
        print(yellow(bold("Inactive Patreon sponsors in sponsors.yml:")))
        for key in inactive:
            rows = rows_by_key.get(key, [])
            for r in rows:
                a = r.get("attributes") or {}
                print(
                    f"  • {name_by_key.get(key, key[1])} ({key[0]}={key[1]})"
                    f"\n    patron_status={a.get('patron_status')}"
                    f" last_charge_status={a.get('last_charge_status')}"
                    f" last_charge_date={a.get('last_charge_date')}"
                    f" entitled={cents(a.get('currently_entitled_amount_cents'))}"
                    f" lifetime={cents(a.get('lifetime_support_cents'))}"
                )
        print()

    if bad_charge:
        has_problems = True
        print()
        print(yellow(bold("Patreon sponsors with non-Paid last charge:")))
        for key in bad_charge:
            rows = rows_by_key.get(key, [])
            for r in rows:
                a = r.get("attributes") or {}
                print(
                    f"  • {name_by_key.get(key, key[1])} ({key[0]}={key[1]})"
                    f"\n    last_charge_status={a.get('last_charge_status')}"
                    f" last_charge_date={a.get('last_charge_date')}"
                    f" entitled={cents(a.get('currently_entitled_amount_cents'))}"
                )
        print()

    if not_found:
        has_problems = True
        print()
        print(red(bold("Sponsors present in sponsors.yml but missing on Patreon:")))
        for key in not_found:
            print(f"  • {key[0]}={key[1]}")
        print()

    # Missing banner-level active patrons (not in YAML).
    yml_key_set = set(yml_keys)
    missing: list[tuple[dict[str, Any], list[tuple[str, str]]]] = []
    for m, keys in member_keys:
        a = m.get("attributes") or {}
        if a.get("patron_status") != "active_patron":
            continue
        if (a.get("currently_entitled_amount_cents") or 0) < BANNER_MIN_AMOUNT_CENTS:
            continue
        if any(k in yml_key_set for k in keys):
            continue
        missing.append((m, keys))

    if missing:
        has_problems = True
        print(yellow(bold("Active banner-level Patreon members missing from sponsors.yml:")))
        for m, keys in missing:
            a = m.get("attributes") or {}
            user_ref = ((m.get("relationships") or {}).get("user") or {}).get("data") or {}
            user = users_by_id.get(user_ref.get("id") or "")
            uattrs = (user or {}).get("attributes") or {}
            name = a.get("full_name") or uattrs.get("full_name") or "(unknown)"
            print(f"  • {name}")
            for k in keys:
                print(f"    key: {k[0]}={k[1]}")
            if uattrs.get("url"):
                print(f"    profile: {uattrs.get('url')}")
            print(
                f"    entitled: {cents(a.get('currently_entitled_amount_cents'))}/mo"
                f"  lifetime: {cents(a.get('lifetime_support_cents'))}"
                f"  last_charge: {a.get('last_charge_status')}"
                f" on {a.get('last_charge_date')}"
            )
            print()
    else:
        print()
        print(green("No active banner-level Patreon members are missing from sponsors.yml. ✓"))

    # Count mismatches: YAML entries vs active patron rows per key.
    mismatches = []
    for key in sorted(yml_count_by_key.keys()):
        yml_count = yml_count_by_key[key]
        active_count = active_count_by_key.get(key, 0)
        if yml_count != active_count:
            mismatches.append((key, yml_count, active_count))

    if mismatches:
        has_problems = True
        print()
        print(yellow(bold("Count mismatches (sponsors.yml entries vs active patrons):")))
        for key, yml_count, ac in mismatches:
            direction = (
                "too many entries in sponsors.yml"
                if yml_count > ac
                else "missing entries in sponsors.yml"
            )
            print(
                f"  • {name_by_key.get(key, key[1])} ({key[0]}={key[1]})"
                f"\n    sponsors.yml: {yml_count}, active patrons: {ac} → {direction}"
            )
    else:
        print()
        print(green("Entry counts match active patron counts. ✓"))

    if has_problems:
        sys.exit(1)
    print()
    print(green("All Patreon sponsors are currently active and aligned. ✓"))
    sys.exit(0)


if __name__ == "__main__":
    main()
