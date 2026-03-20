import re
import shutil
from pathlib import Path

DSP_DIR = Path("source/dsp")

# Each entry: (filename, list of (old_pattern, new_name) tuples)
# Patterns use word boundaries. Order matters — longer/more specific patterns first.

REFACTORS = {

}

def refactor_file(filename: str, replacements: list[tuple[str, str]]):
    filepath = DSP_DIR / filename
    if not filepath.exists():
        print(f"  SKIP (not found): {filepath}")
        return

    backup = filepath.with_suffix(filepath.suffix + ".bak")
    shutil.copy2(filepath, backup)

    text = filepath.read_text()
    for pattern, replacement in replacements:
        text = re.sub(pattern, replacement, text)

    filepath.write_text(text)
    print(f"  OK: {filepath} (backup: {backup.name})")

def main():
    print("=== DSP Variable Refactor ===\n")
    for filename, replacements in REFACTORS.items():
        print(f"Processing {filename}...")
        refactor_file(filename, replacements)
    print("\nDone. Review changes with `git diff source/dsp/`")
    print("To revert: rename .bak files back to originals.")


if __name__ == "__main__":
    main()