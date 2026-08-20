#!/usr/bin/env python3
"""
Check OpenCollective sponsors in `_data/sponsors.yml` against the Tiled collective.

Logic:
- Slot counts: distinct orders that actually charged within RECENT_ACTIVITY_DAYS.
  Order.status is unreliable here: OpenCollective marks many long-running orders
  CANCELLED while they keep billing monthly, so payments are the source of truth.
- Activity status: a sponsor is ACTIVE when it has at least one such recent
  payment, LAPSED otherwise (Member.isActive is only used as extra diagnostics)
- Missing-sponsor detection: only banner-level candidates
  (recent payment + >= $100 total donations)
- URL extraction: use GraphQL `account.website` when suggesting URLs for missing sponsors

Usage:
    python3 _source/check-sponsors.py

Exit codes:
    0 - all checked sponsors are active and aligned
    1 - issues found (lapsed/missing/mismatches/new banner-level sponsors)
    2 - script/runtime/API error
"""

from __future__ import annotations

import json
import os
from datetime import datetime, timedelta, timezone
import sys
import urllib.error
import urllib.request
from collections import Counter, defaultdict
from typing import Any


import yaml

COLLECTIVE_SLUG = "tiled"
GRAPHQL_URL = "https://api.opencollective.com/graphql/v2"
SPONSORS_FILE = os.path.join(
    os.path.dirname(os.path.abspath(__file__)), "..", "_data", "sponsors.yml"
)

# "Big Sponsor" threshold currently used by Tiled banner sponsors.
BANNER_MIN_TOTAL_CENTS = 10_000  # $100.00

# How far back a contribution still counts as "currently sponsoring". Banner
# sponsorships are billed monthly, so this is one cycle plus roughly two weeks
# of slack for retries and billing-date drift.
RECENT_ACTIVITY_DAYS = 45

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


def extract_oc_slug(sponsor_url: str | None) -> str | None:
    if not sponsor_url:
        return None
    prefix = "https://opencollective.com/"
    if not sponsor_url.startswith(prefix):
        return None
    slug = sponsor_url[len(prefix):].strip("/")
    return slug or None


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


def fetch_rest_members(slug: str) -> list[dict[str, Any]]:
    url = f"https://opencollective.com/{slug}/members/all.json"
    req = urllib.request.Request(
        url,
        headers={
            "Accept": "application/json",
            "User-Agent": "check-sponsors/4.0",
        },
    )
    try:
        with urllib.request.urlopen(req, timeout=45) as resp:
            payload = json.loads(resp.read().decode("utf-8"))
    except urllib.error.URLError as exc:
        print(red(f"ERROR: REST members request failed: {exc}"), file=sys.stderr)
        sys.exit(2)

    if not isinstance(payload, list):
        print(red("ERROR: unexpected REST members response format"), file=sys.stderr)
        sys.exit(2)

    return payload


def graphql_request(query: str, variables: dict[str, Any]) -> dict[str, Any]:
    req = urllib.request.Request(
        GRAPHQL_URL,
        data=json.dumps({"query": query, "variables": variables}).encode("utf-8"),
        headers={
            "Content-Type": "application/json",
            "Accept": "application/json",
            "User-Agent": "check-sponsors/4.0",
        },
    )
    try:
        with urllib.request.urlopen(req, timeout=45) as resp:
            payload = json.loads(resp.read().decode("utf-8"))
    except urllib.error.URLError as exc:
        print(red(f"ERROR: GraphQL request failed: {exc}"), file=sys.stderr)
        sys.exit(2)

    if "errors" in payload:
        print(red(f"ERROR: GraphQL errors: {payload['errors']}"), file=sys.stderr)
        sys.exit(2)

    return payload["data"]


def fetch_backer_members(slug: str) -> list[dict[str, Any]]:
    query = """
    query($slug: String!, $offset: Int!, $limit: Int!) {
      account(slug: $slug) {
        members(role: BACKER, offset: $offset, limit: $limit) {
          totalCount
          nodes {
            isActive
            account {
              slug
              name
              website
            }
            totalDonations {
              valueInCents
              currency
            }
          }
        }
      }
    }
    """

    all_nodes: list[dict[str, Any]] = []
    offset = 0
    limit = 100
    total_count = None

    print("Fetching BACKER members via GraphQL (paged) …")
    while True:
        data = graphql_request(query, {"slug": slug, "offset": offset, "limit": limit})
        members = data["account"]["members"]
        nodes = members["nodes"]
        if total_count is None:
            total_count = members["totalCount"]

        all_nodes.extend(nodes)
        offset += len(nodes)

        if not nodes or offset >= total_count:
            break

    print(f"  → received {len(all_nodes)} member rows (totalCount={total_count})\n")
    return all_nodes


def fetch_recent_contribution_slots(
    slug: str, days: int
) -> dict[str, dict[str, str]]:
    """Map each contributor slug to {order id: last charge date} within `days`.

    Each distinct order that charged in the window is one banner slot, which is
    what sponsors.yml entries correspond to.
    """
    query = """
    query($slug: String!, $dateFrom: DateTime!, $offset: Int!, $limit: Int!) {
      transactions(account: {slug: $slug}, type: CREDIT, kind: CONTRIBUTION,
                   dateFrom: $dateFrom, offset: $offset, limit: $limit) {
        totalCount
        nodes {
          createdAt
          fromAccount {
            slug
          }
          order {
            id
          }
        }
      }
    }
    """

    date_from = (
        datetime.now(timezone.utc) - timedelta(days=days)
    ).strftime("%Y-%m-%dT%H:%M:%SZ")

    slots: dict[str, dict[str, str]] = defaultdict(dict)
    offset = 0
    limit = 100
    total_count = None

    print(f"Fetching contributions of the last {days} days (paged) …")
    while True:
        data = graphql_request(
            query,
            {
                "slug": slug,
                "dateFrom": date_from,
                "offset": offset,
                "limit": limit,
            },
        )
        transactions = data["transactions"]
        nodes = transactions["nodes"]
        if total_count is None:
            total_count = transactions["totalCount"]

        for node in nodes:
            from_slug = ((node.get("fromAccount") or {}).get("slug") or "").lower()
            order_id = (node.get("order") or {}).get("id")
            created_at = node.get("createdAt")
            if not from_slug or not order_id or not created_at:
                continue
            previous = slots[from_slug].get(order_id)
            if previous is None or created_at > previous:
                slots[from_slug][order_id] = created_at

        offset += len(nodes)

        if not nodes or offset >= total_count:
            break

    print(
        f"  → {total_count} contributions from {len(slots)} contributors "
        f"since {date_from[:10]}\n"
    )
    return slots


def cents_to_amount_str(value_in_cents: int | None, currency: str | None) -> str:
    if value_in_cents is None:
        return "?"
    return f"{value_in_cents / 100:.2f} {(currency or '').strip()}".strip()


def first_nonempty(values: list[str | None]) -> str | None:
    for v in values:
        if isinstance(v, str) and v.strip():
            return v.strip()
    return None


def main() -> None:
    sponsors = load_sponsors()

    # Current YAML OpenCollective entries
    yml_entries: list[tuple[dict[str, Any], str]] = []
    yml_count_by_slug: Counter[str] = Counter()

    for s in sponsors:
        slug = extract_oc_slug(s.get("sponsor"))
        if slug:
            key = slug.lower()
            yml_entries.append((s, key))
            yml_count_by_slug[key] += 1

    if not yml_entries:
        print("No OpenCollective sponsor entries found in sponsors.yml.")
        sys.exit(0)

    member_rows = fetch_backer_members(COLLECTIVE_SLUG)
    recent_slots = fetch_recent_contribution_slots(
        COLLECTIVE_SLUG, RECENT_ACTIVITY_DAYS
    )
    rest_members = fetch_rest_members(COLLECTIVE_SLUG)

    # Structures
    rows_by_slug: dict[str, list[dict[str, Any]]] = defaultdict(list)
    active_row_count_by_slug: Counter[str] = Counter()
    max_total_donations_cents_by_slug: dict[str, int] = defaultdict(int)
    name_by_slug: dict[str, str] = {}
    rest_last_tx_by_slug: dict[str, str] = {}

    # One slot per order that charged recently.
    slot_count_by_slug: Counter[str] = Counter(
        {slug: len(orders) for slug, orders in recent_slots.items()}
    )
    last_paid_by_slug: dict[str, str] = {
        slug: max(orders.values()) for slug, orders in recent_slots.items()
    }

    for m in rest_members:
        profile = m.get("profile")
        if isinstance(profile, str):
            rest_slug = extract_oc_slug(profile)
            if rest_slug:
                key = rest_slug.lower()
                last_tx = m.get("lastTransactionAt")
                if isinstance(last_tx, str) and last_tx.strip():
                    prev = rest_last_tx_by_slug.get(key)
                    if prev is None or last_tx > prev:
                        rest_last_tx_by_slug[key] = last_tx

    for row in member_rows:
        acct = row.get("account") or {}
        slug = (acct.get("slug") or "").strip().lower()
        if not slug:
            continue

        rows_by_slug[slug].append(row)
        if row.get("isActive") is True:
            active_row_count_by_slug[slug] += 1

        if acct.get("name"):
            name_by_slug[slug] = acct["name"]

        td = row.get("totalDonations") or {}
        cents = td.get("valueInCents")
        if isinstance(cents, int) and cents > max_total_donations_cents_by_slug[slug]:
            max_total_donations_cents_by_slug[slug] = cents

    yml_slugs = sorted({slug for _, slug in yml_entries})

    print(
        bold(
            f"Checking {len(yml_entries)} OpenCollective sponsor entries "
            f"({len(yml_slugs)} unique slugs):\n"
        )
    )
    print(f"  {'Name':<60} {'Slug':<34} {'Last paid':<12} {'Status'}")
    print(f"  {'─' * 60} {'─' * 34} {'─' * 12} {'─' * 22}")

    lapsed: list[str] = []
    not_found: list[str] = []
    active: list[str] = []

    for slug in yml_slugs:
        sponsor_obj = next(s for s, k in yml_entries if k == slug)
        name = sponsor_obj.get("name", "(unnamed)")
        display_name = (name[:57] + "…") if len(name) > 58 else name

        last_paid = last_paid_by_slug.get(slug) or rest_last_tx_by_slug.get(slug)
        last_paid_str = last_paid[:10] if last_paid else "never"

        if slot_count_by_slug.get(slug, 0) > 0:
            status = green("ACTIVE")
            active.append(slug)
        elif rows_by_slug.get(slug):
            status = yellow("LAPSED")
            lapsed.append(slug)
        else:
            status = red("NOT FOUND")
            not_found.append(slug)

        print(f"  {display_name:<60} {slug:<34} {last_paid_str:<12} {status}")

    print()
    print(bold("Summary"))
    print(f"  YAML OC entries: {len(yml_entries)} ({len(yml_slugs)} unique slugs)")
    print(f"  {green('Active')}:    {len(active)}")
    print(f"  {yellow('Lapsed')}:    {len(lapsed)}")
    print(f"  {red('Not found')}: {len(not_found)}")

    has_problems = False

    if lapsed:
        has_problems = True
        print()
        print(
            yellow(
                bold(
                    f"Sponsors with no contribution in the last "
                    f"{RECENT_ACTIVITY_DAYS} days:"
                )
            )
        )
        for slug in lapsed:
            rows = rows_by_slug.get(slug, [])
            acct_name = name_by_slug.get(slug, slug)
            last_tx = rest_last_tx_by_slug.get(slug, "unknown")
            print(f"  • {acct_name} ({slug})")
            print(f"    sponsors.yml entries: {yml_count_by_slug.get(slug, 0)}")
            print(f"    last collective transaction: {last_tx}")
            for r in rows:
                td = r.get("totalDonations") or {}
                total = cents_to_amount_str(td.get("valueInCents"), td.get("currency"))
                print(f"    - isActive={r.get('isActive')} total={total}")
            print()

    if not_found:
        has_problems = True
        print()
        print(red(bold("Sponsors present in sponsors.yml but missing in GraphQL members:")))
        for slug in not_found:
            print(f"  • {slug}")
            print(f"    profile: https://opencollective.com/{slug}")
        print()

    # Missing banner-level sponsors:
    # recent payment + donation total above threshold + absent from YAML
    banner_candidate_slugs = sorted(
        slug
        for slug in slot_count_by_slug
        if max_total_donations_cents_by_slug.get(slug, 0) >= BANNER_MIN_TOTAL_CENTS
    )
    missing_from_yml = [
        slug for slug in banner_candidate_slugs if yml_count_by_slug.get(slug, 0) == 0
    ]

    if missing_from_yml:
        has_problems = True
        print(yellow(bold("Banner-level sponsors missing from sponsors.yml:")))
        for slug in missing_from_yml:
            rows = rows_by_slug.get(slug, [])
            acct_name = name_by_slug.get(slug, slug)

            profile_website = first_nonempty(
                [(r.get("account") or {}).get("website") for r in rows]
            )

            print(f"  • {acct_name} ({slug})")
            print(f"    profile: https://opencollective.com/{slug}")
            print(
                f"    suggested url: {profile_website or '(not found in account.website)'}"
            )
            print(f"    slots paid recently: {slot_count_by_slug.get(slug, 0)}")
            print(f"    last paid: {last_paid_by_slug.get(slug, 'unknown')[:10]}")
            print()

    else:
        print()
        print(green("No banner-level sponsors are missing from sponsors.yml. ✓"))

    # Count mismatch for existing YAML entries:
    # compare YAML entry count against the number of recently paid slots.
    # This captures multi-slot sponsors that added or dropped a slot.
    mismatches = []
    for slug in sorted(yml_count_by_slug.keys()):
        yml_count = yml_count_by_slug.get(slug, 0)
        slot_count = slot_count_by_slug.get(slug, 0)

        # Fully lapsed sponsors are already reported above.
        if slot_count == 0:
            continue

        if yml_count != slot_count:
            mismatches.append((slug, yml_count, slot_count))

    if mismatches:
        has_problems = True
        print()
        print(
            yellow(
                bold("Count mismatches (sponsors.yml entries vs paid slots):")
            )
        )
        for slug, yml_count, slot_count in mismatches:
            acct_name = name_by_slug.get(slug, slug)
            direction = (
                "too many entries in sponsors.yml"
                if yml_count > slot_count
                else "missing entries in sponsors.yml"
            )
            print(f"  • {acct_name} ({slug})")
            print(
                f"    sponsors.yml entries: {yml_count}, "
                f"paid slots: {slot_count} → {direction}"
            )
            for order_id, charged_at in sorted(
                recent_slots.get(slug, {}).items(), key=lambda kv: kv[1], reverse=True
            ):
                print(f"      - {order_id}: last charged {charged_at[:10]}")
            print()
    else:
        print()
        print(green("Entry counts match paid slot counts. ✓"))

    if has_problems:
        sys.exit(1)

    print()
    print(green("All OpenCollective sponsors are currently active and aligned. ✓"))
    sys.exit(0)


if __name__ == "__main__":
    main()
