#!/usr/bin/env python3
"""
Check OpenCollective sponsors in `_data/sponsors.yml` against the Tiled collective.

Hybrid logic:
- Activity status: GraphQL members(role: BACKER) + Member.isActive
- Slot counts: GraphQL account.orders(oppositeAccount: tiled), counting ACTIVE recurring orders
- Missing-sponsor detection: only banner-level candidates (active + >= $100 total donations)
- URL extraction: use GraphQL `account.website` when suggesting URLs for missing sponsors

Usage:
    python3 scripts/check-sponsors.py

Exit codes:
    0 - all checked sponsors are active and aligned
    1 - issues found (inactive/missing/mismatches/new banner-level sponsors)
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
            "User-Agent": "check-sponsors/3.0",
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
            "User-Agent": "check-sponsors/3.0",
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


def fetch_backer_members_with_orders(slug: str) -> list[dict[str, Any]]:
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
              orders(oppositeAccount: { slug: $slug }, limit: 100) {
                nodes {
                  id
                  createdAt
                  amount {
                    valueInCents
                    currency
                  }
                  frequency
                  status
                }
              }
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


def cents_to_amount_str(value_in_cents: int | None, currency: str | None) -> str:
    if value_in_cents is None:
        return "?"
    return f"{value_in_cents / 100:.2f} {(currency or '').strip()}".strip()


def parse_iso_datetime(value: str | None) -> datetime | None:
    if not value:
        return None
    try:
        return datetime.fromisoformat(value.replace("Z", "+00:00"))
    except ValueError:
        return None


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

    member_rows = fetch_backer_members_with_orders(COLLECTIVE_SLUG)
    rest_members = fetch_rest_members(COLLECTIVE_SLUG)

    # Structures
    rows_by_slug: dict[str, list[dict[str, Any]]] = defaultdict(list)
    active_row_count_by_slug: Counter[str] = Counter()
    active_order_count_by_slug: Counter[str] = Counter()
    max_total_donations_cents_by_slug: dict[str, int] = defaultdict(int)
    most_recent_order_by_slug: dict[str, datetime] = {}
    name_by_slug: dict[str, str] = {}
    rest_last_tx_by_slug: dict[str, str] = {}

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

        # Slot counting from orders: only ACTIVE recurring orders.
        orders = (acct.get("orders") or {}).get("nodes") or []
        active_order_ids = set()
        most_recent = most_recent_order_by_slug.get(slug)
        for o in orders:
            if o.get("status") == "ACTIVE":
                active_order_ids.add(o.get("id"))
            created_at = parse_iso_datetime(o.get("createdAt"))
            if created_at and (most_recent is None or created_at > most_recent):
                most_recent = created_at
        if most_recent is not None:
            most_recent_order_by_slug[slug] = most_recent
        active_order_count_by_slug[slug] = max(
            active_order_count_by_slug[slug], len(active_order_ids)
        )

    yml_slugs = sorted({slug for _, slug in yml_entries})

    print(
        bold(
            f"Checking {len(yml_entries)} OpenCollective sponsor entries "
            f"({len(yml_slugs)} unique slugs):\n"
        )
    )
    print(f"  {'Name':<70} {'Slug':<40} {'Status'}")
    print(f"  {'─' * 70} {'─' * 40} {'─' * 24}")

    inactive: list[str] = []
    not_found: list[str] = []
    active: list[str] = []

    for slug in yml_slugs:
        sponsor_obj = next(s for s, k in yml_entries if k == slug)
        name = sponsor_obj.get("name", "(unnamed)")
        display_name = (name[:67] + "…") if len(name) > 68 else name

        rows = rows_by_slug.get(slug, [])
        if not rows:
            status = red("NOT FOUND IN GRAPHQL MEMBERS")
            not_found.append(slug)
        else:
            is_active = any(r.get("isActive") is True for r in rows)
            if is_active:
                status = green("ACTIVE")
                active.append(slug)
            else:
                status = yellow("INACTIVE")
                inactive.append(slug)

        print(f"  {display_name:<70} {slug:<40} {status}")

    print()
    print(bold("Summary"))
    print(f"  YAML OC entries: {len(yml_entries)} ({len(yml_slugs)} unique slugs)")
    print(f"  {green('Active')}:    {len(active)}")
    print(f"  {yellow('Inactive')}:  {len(inactive)}")
    print(f"  {red('Not found')}: {len(not_found)}")

    has_problems = False

    if inactive:
        has_problems = True
        print()
        print(yellow(bold("Inactive sponsors in sponsors.yml:")))
        for slug in inactive:
            rows = rows_by_slug.get(slug, [])
            acct_name = name_by_slug.get(slug, slug)
            print(f"  • {acct_name} ({slug})")
            for r in rows:
                td = r.get("totalDonations") or {}
                tier = (r.get("tier") or {}).get("name", "Unknown tier")
                total = cents_to_amount_str(td.get("valueInCents"), td.get("currency"))
                print(f"    - isActive={r.get('isActive')} tier={tier} total={total}")
            print()

    if not_found:
        has_problems = True
        print()
        print(red(bold("Sponsors present in sponsors.yml but missing in GraphQL members:")))
        for slug in not_found:
            print(f"  • {slug}")
            print(f"    profile: https://opencollective.com/{slug}")
        print()

    # Missing banner-level active sponsors:
    # active member + donation total above threshold + absent from YAML
    now = datetime.now(timezone.utc)
    recent_cutoff = now - timedelta(days=RECENT_ACTIVITY_DAYS)

    active_banner_candidate_slugs = sorted(
        slug
        for slug, count in active_row_count_by_slug.items()
        if count > 0
        and max_total_donations_cents_by_slug.get(slug, 0) >= BANNER_MIN_TOTAL_CENTS
        and (
            (
                most_recent_order_by_slug.get(slug) is not None
                and most_recent_order_by_slug[slug] >= recent_cutoff
            )
            or active_order_count_by_slug.get(slug, 0) > 0
        )
    )
    missing_from_yml = [
        slug for slug in active_banner_candidate_slugs if yml_count_by_slug.get(slug, 0) == 0
    ]

    if missing_from_yml:
        has_problems = True
        print(yellow(bold("Active banner-level BACKER members missing from sponsors.yml:")))
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
            print(f"    active member rows: {active_row_count_by_slug.get(slug, 0)}")
            print(f"    active recurring orders: {active_order_count_by_slug.get(slug, 0)}")
            print()

    else:
        print()
        print(green("No active banner-level BACKER members are missing from sponsors.yml. ✓"))

    # Count mismatch for existing YAML entries:
    # Compare YAML entry count vs ACTIVE recurring order count.
    # This captures multi-slot sponsors.
    mismatches = []
    for slug in sorted(yml_count_by_slug.keys()):
        yml_count = yml_count_by_slug.get(slug, 0)
        active_order_count = active_order_count_by_slug.get(slug, 0)

        if yml_count != active_order_count:
            mismatches.append((slug, yml_count, active_order_count))

    if mismatches:
        has_problems = True
        print()
        print(
            yellow(
                bold(
                    "Count mismatches (sponsors.yml entries vs ACTIVE recurring orders):"
                )
            )
        )
        for slug, yml_count, active_orders in mismatches:
            acct_name = name_by_slug.get(slug, slug)
            direction = (
                "too many entries in sponsors.yml"
                if yml_count > active_orders
                else "missing entries in sponsors.yml"
            )
            print(f"  • {acct_name} ({slug})")
            print(
                f"    sponsors.yml entries: {yml_count}, "
                f"active recurring orders: {active_orders} → {direction}"
            )

            # order + transaction diagnostics
            rows = rows_by_slug.get(slug, [])
            all_order_nodes = []
            for r in rows:
                acct = r.get("account") or {}
                all_order_nodes.extend((acct.get("orders") or {}).get("nodes") or [])

            unique_orders = {o.get("id"): o for o in all_order_nodes if o.get("id")}

            print("    order diagnostics:")
            rest_last_tx = rest_last_tx_by_slug.get(slug, "unknown")
            print(f"      last collective transaction (REST): {rest_last_tx}")

            if unique_orders:
                for order in unique_orders.values():
                    amt = order.get("amount") or {}
                    amount = cents_to_amount_str(
                        amt.get("valueInCents"), amt.get("currency")
                    )
                    print(
                        f"      - {order.get('id')}: status={order.get('status')}, "
                        f"freq={order.get('frequency')}, amount={amount}, "
                        f"created={str(order.get('createdAt', ''))[:10]}"
                    )
            print()
    else:
        print()
        print(green("Entry counts match ACTIVE recurring order counts. ✓"))

    if has_problems:
        sys.exit(1)

    print()
    print(green("All OpenCollective sponsors are currently active and aligned. ✓"))
    sys.exit(0)


if __name__ == "__main__":
    main()
