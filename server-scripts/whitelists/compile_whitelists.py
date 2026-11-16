#!/usr/bin/env python3
"""Merge repository-owned whitelist additions into the packaged CoreScripts."""

from __future__ import annotations

import argparse
import re
from pathlib import Path


SCOPES_MARKER_START = "-- BEGIN TES3COOP managed client variable whitelist"
SCOPES_MARKER_END = "-- END TES3COOP managed client variable whitelist"
SCRIPTS_MARKER_START = "    -- BEGIN TES3COOP managed synchronized client scripts"
SCRIPTS_MARKER_END = "    -- END TES3COOP managed synchronized client scripts"


def read_lua_strings(block: str) -> list[str]:
    return re.findall(r'"((?:\\.|[^"\\])*)"', block)


def read_scope_lists(path: Path) -> dict[str, list[str]]:
    text = path.read_text(encoding="utf-8")
    result: dict[str, list[str]] = {}
    for scope in ("personal", "worldwide", "kills", "quest"):
        match = re.search(rf"(?m)^{scope}\s*=\s*\{{(.*?)^\}}", text, re.S)
        if not match:
            raise ValueError(f"Missing {scope} table in {path}")
        result[scope] = read_lua_strings(match.group(1))
        if not result[scope]:
            raise ValueError(f"No entries found for {scope} in {path}")
    return result


def read_script_ids(path: Path) -> list[str]:
    text = path.read_text(encoding="utf-8")
    match = re.search(r"config\.synchronizedClientScriptIds\s*=\s*\{(.*?)^\}", text, re.S | re.M)
    if not match:
        raise ValueError(f"Missing synchronizedClientScriptIds table in {path}")
    ids = read_lua_strings(match.group(1))
    if not ids:
        raise ValueError(f"No script IDs found in {path}")
    return ids


def lua_quote(value: str) -> str:
    return '"' + value.replace("\\", "\\\\").replace('"', '\\"') + '"'


def patch_scopes(path: Path, scopes: dict[str, list[str]]) -> None:
    text = path.read_text(encoding="utf-8")
    if SCOPES_MARKER_START in text:
        text = text[: text.index(SCOPES_MARKER_START)] + text[text.index(SCOPES_MARKER_END) + len(SCOPES_MARKER_END) :]

    lines = [
        SCOPES_MARKER_START,
        'if tableHelper.containsCaseInsensitiveString(clientDataFiles, "TR_Mainland.esm") then',
    ]
    for scope, variables in scopes.items():
        lines += [
            f"    clientVariableScopes.globals.{scope} = clientVariableScopes.globals.{scope} or {{}}",
            "    local trVariables = {",
        ]
        lines.extend(f"        {lua_quote(value)}," for value in variables)
        lines += [
            "    }",
            f"    for _, variable in ipairs(trVariables) do",
            f"        if not tableHelper.containsCaseInsensitiveString(clientVariableScopes.globals.{scope}, variable) then",
            f"            table.insert(clientVariableScopes.globals.{scope}, variable)",
            "        end",
            "    end",
        ]
    lines += ["end", SCOPES_MARKER_END, ""]
    marker = "return clientVariableScopes"
    if marker not in text:
        raise ValueError(f"Could not find {marker!r} in {path}")
    text = text.replace(marker, "\n".join(lines) + marker, 1)
    path.write_text(text, encoding="utf-8", newline="")


def patch_script_ids(path: Path, additions: list[str]) -> None:
    text = path.read_text(encoding="utf-8")
    if SCRIPTS_MARKER_START in text:
        start = text.index(SCRIPTS_MARKER_START)
        end = text.index(SCRIPTS_MARKER_END, start) + len(SCRIPTS_MARKER_END)
        text = text[:start] + text[end:]

    match = re.search(r"config\.synchronizedClientScriptIds\s*=\s*\{", text)
    if not match:
        raise ValueError(f"Could not find config.synchronizedClientScriptIds in {path}")
    close = text.find("\n}", match.end())
    if close < 0:
        raise ValueError(f"Could not find closing brace for synchronizedClientScriptIds in {path}")
    block = text[match.end() : close]
    existing = {value.lower() for value in read_lua_strings(block)}
    missing = [value for value in additions if value.lower() not in existing]
    if missing:
        generated = "\n" + SCRIPTS_MARKER_START + "\n" + "\n".join(
            f"    {lua_quote(value)}," for value in missing
        ) + "\n" + SCRIPTS_MARKER_END + "\n"
        text = text[:close] + generated + text[close:]
    path.write_text(text, encoding="utf-8", newline="")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--core-scripts", type=Path, required=True)
    parser.add_argument("--whitelist-dir", type=Path, required=True)
    args = parser.parse_args()

    scopes = read_scope_lists(args.whitelist_dir / "clientVariableScopes.lua")
    scripts = read_script_ids(args.whitelist_dir / "synchronizedClientScriptIds.lua")
    scopes_file = args.core_scripts / "clientVariableScopes.lua"
    config_file = args.core_scripts / "config.lua"
    if not scopes_file.is_file() or not config_file.is_file():
        raise FileNotFoundError("CoreScripts clientVariableScopes.lua or config.lua is missing")
    patch_scopes(scopes_file, scopes)
    patch_script_ids(config_file, scripts)


if __name__ == "__main__":
    main()
