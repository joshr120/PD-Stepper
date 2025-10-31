#!/usr/bin/env awk
#
# This awk script processes a Makefile and extracts targets and their descriptions
# so as to make it easy to document the sub-commands/targets supported
# by the Makefile.
#
# For the purpose of this awk script a "Target" is a space seperated list of
# names followed by a colon and the description is the text following "## "
# on the same line.
#
# Example Makefile usage:
# ----------------------------------
# NOTE: Recipe lines below are written as "#<space><tab>cmd".
#       If you delete the leading "# ", the tab is preserved,
#       so you get a valid Makefile with proper tabs.
#
# .PHONY: help h H compile c clean
#
# help: ## Show this help message
# 	@echo "Manage the building and testing this project"
# 	@echo ""
# 	@echo "Usage:"
# 	@echo "   make <target>
# 	@echo ""
# 	@echo "Targets:"
# 	@awk -f support/target-list.awk $(lastword $(MAKEFILE_LIST))
#
# # No space is required after the colon, but it is recommended for readability.
# clean:## Remove build artifacts
# 	@echo "Remove any and all build artifacts."
#
# # Target maybe a list of names seperated by spaces
# compile comp C: ## Build the project from source
# 	@echo "Translate source files to object files."
#
# # A tarte make using dot notation to provide sub-commands
# # such as lib.list, lib.install, lib.remove.
# # Example: `make lib.install PKG="LibraryName"`.
# lib.%:  ## lib.<cmd>
# 	primary lib $(@:lib.%=%) $(if $(PKG),"$(PKG)")
# ----------------------------------
#
# Rules:
# - Parse ONLY the file passed as argv[1] (typically $(lastword $(MAKEFILE_LIST))).
# - Basic form; list of targets, a colon then anything then "## " followed by description:
#       <targets> ":.*" "##" spaces <desc>
# - Recipe lines (start with a tab) are ignored.
# - Character set is UTF-8, case-sensitive. No multi-line target lists.
# - Ignore lines where desc begins with "@internal".
# - Render "name.%" as "name.<cmd>" in the targets column.
# - Acceptes two parameters:
#    - indent_pad=X   Where X is number of spaces to indent first column, default 4
#    - desc_pad=Y     Where Y is number of spaces between the widest targets column
#                     and the start of descriptor column, default 4
# - Order returned is the order processed.

BEGIN {
  n=0
  maxw=0
  if (desc_pad == 0) desc_pad = 4
  if (indent_pad == 0) indent_pad = 4
}

# --- helpers (avoid DRY) ---
function ltrim(s){ sub(/^[[:space:]]+/, "", s); return s }      # remove leading spaces
function rtrim(s){ sub(/[[:space:]]+$/, "", s); return s }      # remove trailing spaces
function trim(s){ return rtrim(ltrim(s)) }                      # remove both
function is_recipe(line){ return line ~ /^\t/ }                 # true if line starts with a tab
function is_internal(desc){ d=ltrim(desc); return d ~ /^@internal([[:space:]]|$)/ }
function render_target(t){ if (t ~ /\.%$/) sub(/\.%$/, ".<cmd>", t); return t }

# Process each line of the file passed in $0.
# We're going to extract a set of space seperated
# targets followed by a ":" and then a description
# which follows "## ":
{
  # Get the line
  line = $0

  # Skip receipe lines, they start with a tab
  if (is_recipe(line)) next

  # Must be at least one ':'
  col = index(line, ":")
  if (!col) next

  # Targets are before the colon
  targets_raw = trim(substr(line, 1, col-1))
  rest = substr(line, col+1)

  # The description is two hash marks followed by whitespace
  if (match(rest, /##[[:space:]]+/) == 0) next
  description = substr(rest, RSTART + RLENGTH)

  # Skip if there are no targets or if description is internal
  if (targets_raw == "" || is_internal(description)) next

  # Split target list on whitespace, rewrite any ".%" to ".<cmd>", then rejoin
  nt = split(targets_raw, T, /[[:space:]]+/)
  targets = ""
  for (i=1; i<=nt; i++) {
    t = render_target(T[i])
    targets = (targets=="" ? t : targets " " t)
  }

  # Save results, track widest targets column
  targets_array[++n] = targets
  descriptions_array[n]   = description
  if (length(targets) > maxw) maxw = length(targets)
}

END {
  # Print results in two aligned columns
  # the first column is indented by indent_pad spaces
  # the second column is separated by desc_pad spaces
  # from the longest entry in the first column
  for (i=1; i<=n; i++) {
    pad = maxw - length(targets_array[i]) + desc_pad
    printf "%*s%s", indent_pad, "", targets_array[i]
    while (pad-- > 0) printf " "
    printf "%s\n", descriptions_array[i]
  }
}
