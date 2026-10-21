# ./devenv/scripts/f3-json-check.nix

{ ... }: {
  scripts.f3-json-check = {
    description = "Verify artifacts/F3.json against OOAnalyzer's Prolog results; --provenance prints where it came from";
    exec = /* bash */ ''
      set -uo pipefail

      # T-040. artifacts/F3.json is a FROZEN 2024-04-04 OOAnalyzer export, not a live index:
      #   IDB + OOAnalyzer.py --facts--> F3-facts.pl --ooprolog--> F3-results.pl --ooprolog--> F3.json
      # Stage 2 (ooprolog, Java) is not on this machine and the installed OOAnalyzer.py (v2.0) cannot
      # export facts, so an IDB rename cannot be flowed into this file here. This script pins the
      # schema instead: it re-derives every field we claim to understand and fails on drift, so a
      # future exporter that quietly changes shape is caught rather than trusted.
      #
      # BOUNDARY: proves class keys, class sizes, method sets, member sizes and member usages. It does
      # NOT verify the 4,059 synthesized base/struc/vftptr member entries or the vcalls map, and it says
      # so. It never writes artifacts/F3.json.
      #
      # IT ALSO ENFORCES ITS OWN HEADLINE CLAIM (added 1a8d101, after the coordinator's "what if this
      # goes red" note exposed that it printed a md5 it never checked). The guard asserts, not just
      # describes:
      #   export "version" == "2.2.0"  (the schema pin in IDA/plugins/OOAnalyzer.py:21)
      #   export "filemd5" == md5($OPENVB_ORIG_EXE), when that variable points at a real file
      # so a tampered export can no longer report clean. Verified: version -> "9.9.9" now exits 1 with
      # "DRIFT REASON: PROVENANCE: ...". The md5 shown on the red path is MEASURED at run time, not a
      # baked-in constant, because a stale hash in an error message misleads the person it is for.
      #
      # IF IT GOES RED AND YOU DID NOT CHANGE THE CODE: the likely cause is that a real exporter was
      # restored -- a live OOAnalyzer emits actual base/struc/vftptr values, which this rebuild marks
      # UNVERIFIED, so a correct generator reads as drift. That is the guard WORKING. Finish the
      # synthesized-member rebuild AND re-pin the golden expectations in the same change; do not soften,
      # skip or threshold this check, and never hand-edit artifacts/F3.json (T-040 acceptance rule).
      # Comment-only change; the 0/1/2/4 exit contract below is unchanged.
      if [ "''${1:-}" = "--provenance" ]; then
        json="artifacts/F3.json"
        [ -f "$json" ] || { echo "f3-json-check: missing $json"; exit 4; }
        printf 'file            : %s\n' "$json"
        printf 'mtime           : %s  (relocation date, NOT generation date)\n' "$(date -r "$json" +%Y-%m-%d 2>/dev/null || echo '?')"
        printf 'idb mtime       : %s\n' "$(date -r artifacts/F3.exe.i64 +%Y-%m-%d 2>/dev/null || echo '?')"
        printf 'version in file : %s\n' "$(grep -o '"version":"[^"]*"' "$json" | head -1)"
        printf 'OOAnalyzer schema pin: %s\n' "$(grep -m1 EXPECTED_JSON_VERSION /mnt/secondary/IDA/*/IDA/plugins/OOAnalyzer.py 2>/dev/null | sed 's/.*= //' || echo 'plugin not found')"
        printf '_PyOOAnalyzer__ fingerprint occurrences in the IDB: %s\n' \
          "$(strings artifacts/F3.exe.id0 2>/dev/null | grep -c '_PyOOAnalyzer__' || echo 0)"
        printf 'original pipeline recipe: %s\n' \
          "$(ls /mnt/secondary/*/Desktop/OpenJE/info/ooanalyzer_steps.txt 2>/dev/null | head -1 || echo 'not found')"
        for p in /mnt/secondary/VSCode/openje-re/ooanalyzer /mnt/secondary/Ubuntu/Desktop/VSCode/OpenJE/openje-re/ooanalyzer; do
          [ -f "$p/F3.json" ] && printf 'sibling export  : %s/F3.json md5=%s (ours=%s)\n' \
            "$p" "$(md5sum "$p/F3.json" | cut -c1-32)" "$(md5sum "$json" | cut -c1-32)"
        done
        echo
        echo "VERDICT: F3.json is a frozen 2024-04-04 OOAnalyzer export. Measure against the live IDB"
        echo "(route A); quote this file as route B and label it stale. See"
        echo "discoveries/2025-10-20-t040-f3json-provenance.md"
        exit 0
      fi

      py=tools/f3json-from-results.py
      [ -f "$py" ] || { echo "f3-json-check: missing $py"; exit 4; }
      # NB: `if ! cmd; then rc=$?` captures the negation's status (always 0), not the tool's.
      # Measure the program, not the shell wrapper -- same class of error as reading exit codes
      # through a pipeline.
      python3 "$py" "$@"
      rc=$?
      if [ "$rc" -ne 0 ]; then
        echo
        echo "f3-json-check FAILED (exit $rc): the export no longer matches what OOAnalyzer's results"
        echo "say it should contain. Do NOT hand-edit artifacts/F3.json to make a delta appear --"
        echo "that converts a tooling gap into a false measurement (T-040 acceptance rule)."
        exit $rc
      fi
    '';
  };
}
