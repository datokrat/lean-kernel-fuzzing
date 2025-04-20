import argparse
import re
import sys
from pathlib import Path
from typing import List, Optional, Set, Dict

# Regex to find import statements at the beginning of a line (ignoring whitespace)
# It captures the module names (one or more, separated by whitespace) in group 1.
# Does NOT perfectly handle block comments (`/- ... -/`) containing imports.
IMPORT_REGEX = re.compile(r"^\s*import\s+((?:[\w\.]+\s*)+)")

# Cache to store direct imports already parsed from a file
DIRECT_IMPORT_CACHE: Dict[Path, Set[str]] = {}

def find_lean_file(module_name: str, source_roots: List[Path]) -> Optional[Path]:
    """
    Tries to find the .lean file corresponding to a module name within the source roots.

    Args:
        module_name: The Lean module name (e.g., "Mathlib.Data.Nat.Basic").
        source_roots: A list of directories to search within.

    Returns:
        The Path object for the found .lean file, or None if not found.
    """
    module_parts = module_name.split('.')
    relative_path = Path(*module_parts).with_suffix('.lean')

    for root in source_roots:
        potential_path = root / relative_path
        if potential_path.is_file():
            return potential_path
    return None

def find_c_file(module_name: str, c_root: str) -> Optional[Path]:
    module_parts = module_name.split('.')
    relative_path = Path(*module_parts).with_suffix('.c')
    return c_root / relative_path

def get_direct_imports(file_path: Path) -> Set[str]:
    """
    Parses a Lean file and extracts its direct imports.

    Args:
        file_path: The Path to the .lean file.

    Returns:
        A set of directly imported module names (strings).
        Returns an empty set if the file cannot be read or has no imports.
    """
    if file_path in DIRECT_IMPORT_CACHE:
        return DIRECT_IMPORT_CACHE[file_path]

    imports: Set[str] = set()
    try:
        with file_path.open('r', encoding='utf-8') as f:
            for line in f:
                # Basic check for single-line comments
                if line.strip().startswith('--'):
                    continue

                match = IMPORT_REGEX.match(line)
                if match:
                    # Split potentially multiple imports on the same line (e.g., import A B C)
                    imported_modules_str = match.group(1)
                    imports.update(mod for mod in imported_modules_str.split() if mod)

    except IOError as e:
        print(f"Warning: Could not read file {file_path}: {e}", file=sys.stderr)
        return set()
    except Exception as e: # Catch potential decoding errors too
        print(f"Warning: Error processing file {file_path}: {e}", file=sys.stderr)
        return set()


    DIRECT_IMPORT_CACHE[file_path] = imports
    return imports

def get_lean_path_module_name(file_path: Path, source_roots: List[Path]) -> Optional[str]:
    """
    Converts a file path within a source root back to a Lean module name.

    Args:
        file_path: The absolute Path to the .lean file.
        source_roots: A list of possible source root directories.

    Returns:
        The corresponding Lean module name, or None if the file is not
        relative to any source root.
    """
    abs_file_path = file_path.resolve()
    for root in source_roots:
        abs_root = root.resolve()
        try:
            relative_path = abs_file_path.relative_to(abs_root)
            # Remove '.lean' suffix and replace path separators with dots
            module_name = '.'.join(relative_path.with_suffix('').parts)
            return module_name
        except ValueError:
            # Not relative to this root
            continue
    return None

def get_transitive_imports(
    start_files: Path,
    source_roots: List[Path]
) -> Set[str]:
    """
    Computes the set of all transitive imports for a starting Lean file.

    Args:
        start_file: The Path object for the initial .lean file.
        source_roots: A list of directories where Lean source files reside
                      (e.g., project root, lake-packages sources).

    Returns:
        A set containing all unique transitive module names imported.
    """
    start_modules = []
    for start_file in start_files:
        start_module = get_lean_path_module_name(start_file, source_roots)
        if not start_module:
            print(
                f"Error: Starting file {start_file} is not located within any of the "
                f"provided source roots: {source_roots}", file=sys.stderr
            )
            return set()
        start_modules.append(start_module)

    all_imports: Set[str] = set()
    processed_modules: Set[str] = set()
    # Use a list as a queue/stack for modules to scan
    modules_to_scan: List[str] = start_modules

    # Exclude the starting module itself from the final list of *imports*
    # but process it to kick off the scan. We also don't process builtins.
    #processed_modules.add(start_module)
    builtins = {"Init", "Lean"} # Modules without source files we track

    while modules_to_scan:
        current_module = modules_to_scan.pop()

        lean_file_path = find_lean_file(current_module, source_roots)

        if lean_file_path is None:
            print(f"Warning: Could not find source file for module: {current_module}", file=sys.stderr)
            # Add to processed so we don't warn again
            processed_modules.add(current_module)
            continue

        if not lean_file_path.is_file():
             print(f"Warning: Expected file does not exist: {lean_file_path}", file=sys.stderr)
             processed_modules.add(current_module)
             continue

        direct_imports = get_direct_imports(lean_file_path)

        for imported_module in direct_imports:
            # Add to the final list of imports
            all_imports.add(current_module)
            # If we haven't processed this module yet, add it to the scan list
            if imported_module not in processed_modules:
                 processed_modules.add(imported_module) # Mark as processed *now*
                 modules_to_scan.append(imported_module)

    return all_imports

def main():
    parser = argparse.ArgumentParser(
        description="Compute the transitive Lean imports for a given file within a project.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument(
        "-f",
        "--start_file",
        type=Path,
        action='append',
        help="The path to the initial .lean file."
    )
    parser.add_argument(
        "-s", "--source-root",
        type=Path,
        action='append',
        dest='source_roots',
        required=True,
        help="Path to a source root directory (e.g., '.', 'lake-packages/mathlib/src'). "
             "Can be specified multiple times. At least one is required."
    )
             
    parser.add_argument(
        "-t", "--target-root",
        type=Path,
        dest='target_root',
        required=True,
        help="directory of C files"
    )
    # Future improvement idea:
    # parser.add_argument(
    #     "--use-lake",
    #     action="store_true",
    #     help="Attempt to automatically determine source roots using 'lake print-paths'."
    # )

    args = parser.parse_args()
    
    valid_start_files = []
    for file in args.start_file:
        if not file.is_file():
            print(f"Error: Start file not found: {file}", file=sys.stderr)
            sys.exit(1)
        else:
            valid_start_files.append(file)

    if not args.target_root.is_dir():
        print(f"Error: Target root is not a directory: {root}", file=sys.stderr)
        sys.exit(1)

    # Ensure source roots exist
    valid_source_roots = []
    for root in args.source_roots:
        if not root.is_dir():
            print(f"Warning: Specified source root is not a directory: {root}", file=sys.stderr)
        else:
            valid_source_roots.append(root.resolve()) # Use absolute paths

    if not valid_source_roots:
        print("Error: No valid source root directories provided or found.", file=sys.stderr)
        sys.exit(1)

    # print(f"Scanning imports starting from: {valid_start_files}")
    # print(f"Using source roots: {valid_source_roots}")
    # print("-" * 20)

    transitive_imports = get_transitive_imports(valid_start_files, valid_source_roots)

    if not transitive_imports:
        # print("No imports found.")
        pass
    else:
        # print("Transitive Imports:")
        # Sort for consistent output
        for module in sorted(list(transitive_imports)):
            print(f"{find_c_file(module, args.target_root)}")

    # print("-" * 20)
    # print(f"Found {len(transitive_imports)} unique transitive imports.")


if __name__ == "__main__":
    main()